#include "Shaders.hlsl"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_INSTANCE_PARTICLE_DRAW_INPUT
{
    float3 position : POSITION;

    float3 instancePos : INSTANCE_POSITION;
    float4 velocity_and_Rotate : INSTANCE_VELOCITY; // xyz = 회전축, w = 회전각
    float4 color : INSTANCE_COLOR;
};

struct VS_INSTANCE_PARTICLE_DRAW_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float4 color : COLOR;
    float2 velocity : VELOCITY;
};

float3x3 AxisAngleToMatrix(float3 axis, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    float t = 1.0f - c;

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    return float3x3(
        t * x * x + c, t * x * y - s * z, t * x * z + s * y,
        t * x * y + s * z, t * y * y + c, t * y * z - s * x,
        t * x * z - s * y, t * y * z + s * x, t * z * z + c
    );
}

VS_INSTANCE_PARTICLE_DRAW_OUTPUT VSParticleDraw(VS_INSTANCE_PARTICLE_DRAW_INPUT input)
{
    VS_INSTANCE_PARTICLE_DRAW_OUTPUT output = (VS_INSTANCE_PARTICLE_DRAW_OUTPUT) 0;

    // 1. 회전 행렬 생성
    float3 axis = normalize(input.velocity_and_Rotate.xyz);
    float angle = input.velocity_and_Rotate.w;
    float3x3 rotation = AxisAngleToMatrix(axis, angle);

    // 2. 로컬 정점 회전 적용
    float3 rotatedPos = mul(rotation, input.position);

    // 3. 인스턴스 위치 적용
    float3 worldPos = rotatedPos + input.instancePos;

    // 4. 월드 변환
    float4 worldPos4 = mul(float4(worldPos, 1.0f), gmtxGameObject);
    output.positionW = worldPos4.xyz;

    // 5. 뷰-투영
    output.position = mul(mul(worldPos4, gmtxView), gmtxProjection);

    // 6. 색상 전달
    output.color = input.color;

    // 7. 속도 계산 (월드 이동 방향 → 화면 픽셀 방향)
    float4 velocityViewProj = mul(mul(float4(input.velocity_and_Rotate.xyz, 0.0f), gmtxView), gmtxProjection);
    float2 velocityNDC = velocityViewProj.xy / output.position.w;
    output.velocity = velocityNDC * 0.5f * float2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

    return output;
}

float4 PS_Transparent_ParticleDraw(VS_INSTANCE_PARTICLE_DRAW_OUTPUT input) : SV_Target
{
    float4 particle_color = input.color;

    return float4(particle_color);
}

// Pixel Shader
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PS_Deffered_ParticleDraw(VS_INSTANCE_PARTICLE_DRAW_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    output.Albedo_Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    output.world_Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.world_Normal_and_Camera_Distance = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Material_Light_Info = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Velocity_Mask_Obj_Id = float4(0.0f, 0.0f, 0.0f, 0.0f);
    


    output.Albedo_Color = input.color;
    
    output.world_Position = float4(input.positionW, 1.0f);
    output.world_Normal_and_Camera_Distance.xyz = float3(0.0f, 1.0f, 0.0f);
    output.world_Normal_and_Camera_Distance.w = distance(input.positionW, gvCameraPosition);

    output.Material_Light_Info = float4(material_info.gRoughness, 0.0f, material_info.gSpecular_intensity, material_info.gEmissive_intensity);
    output.Velocity_Mask_Obj_Id = float4(input.velocity, 0.0f, 1.0f);

    return output;
}


//==================================================================

struct VS_TRAIL_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 sideTime : TEXCOORD1; // x=side, y=time, z=centerY, w=offsetY
    float ratio : TEXCOORD2; // fading factor (0.0 to 1.0)
};

struct VS_TRAIL_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITIONW;
    float2 uv : TEXCOORD0;
    float time : TEXCOORD1;
    float ratio : TEXCOORD2; // 추가!
};


VS_TRAIL_OUTPUT Trail_VS(VS_TRAIL_INPUT input)
{
    VS_TRAIL_OUTPUT output;

    float ratio = saturate(input.ratio); // 위치 기반 비율
    float pulse = 0.1f * sin(gfCurrentTime * 10.0f + input.position.x * 5.0f); 

    float centerY = input.sideTime.z;
    float offsetY = input.sideTime.w;
    float3 pos = input.position;
    
    pos.y = centerY + offsetY * ratio * (1.0f + pulse); // ratio로 크기 조절 + pulse 추가

    float4 worldPos = mul(float4(pos, 1.0f), gmtxGameObject);
    output.positionW = worldPos.xyz;

    float4 viewPos = mul(worldPos, gmtxView);
    output.position = mul(viewPos, gmtxProjection);

    output.uv = input.uv;
    output.time = input.sideTime.y;
    output.ratio = ratio; // ratio 전달
    return output;
}

float4 Trail_PS(VS_TRAIL_OUTPUT input) : SV_Target
{
    float ratio = saturate(input.ratio);

    // Smooth fade and glow using curve
    float fade = pow(ratio, 1.5); // soft transparency falloff
    float glow = 0.3f + 0.7f * pow(ratio, 0.8); // enhanced brightness near head

    float3 baseColor = float3(1.0f, 0.2f, 0.1f); // reddish trail
    float3 finalColor = baseColor * glow;

    return float4(finalColor, fade);
}