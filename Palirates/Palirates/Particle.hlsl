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
    float2 sideTime : TEXCOORD1;
};

struct VS_TRAIL_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float2 uv : TEXCOORD0;
    float time : TEXCOORD1;
};


VS_TRAIL_OUTPUT Trail_VS(VS_TRAIL_INPUT input)
{
    VS_TRAIL_OUTPUT output;

    float4 worldPos = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.positionW = worldPos.xyz;

    float4 viewPos = mul(worldPos, gmtxView);
    output.position = mul(viewPos, gmtxProjection);

    output.uv = input.uv;
    output.time = input.sideTime.y; // 세그먼트 생성 시간 전달

    return output;
}



float4 Trail_PS(VS_TRAIL_OUTPUT input) : SV_Target
{
    float trailAge = gfCurrentTime - input.time; // 현재 시각 - 생성 시각
    float lifespan = 5.0f; // 생존 시간 (초)
    float fade = saturate(1.0f - trailAge / lifespan); // 오래될수록 투명해짐

    float3 baseColor = float3(1.0f, 0.2f, 0.1f); // 붉은 계열 트레일
    float4 trailColor = float4(baseColor, fade); // 알파 적용

    return trailColor;


}