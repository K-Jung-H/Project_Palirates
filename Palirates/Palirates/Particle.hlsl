#include "Shaders.hlsl"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_INSTANCE_PARTICLE_DRAW_INPUT
{
    float3 position : LOCALPOS; 
    float4 Position_and_Scale : INSTANCE_POS_SCALE;
    float4 velocity_and_Rotate : INSTANCE_VELOCITY; // xyz = 회전축, w = 회전각
    float4 color : INSTANCE_COLOR;
    
    uint instanceID : SV_InstanceID;
};

struct VS_INSTANCE_PARTICLE_DRAW_OUTPUT
{
    float4 position : SV_POSITION; 
    float3 positionW : POSITIONW; 
    float4 color : COLOR;
    float2 velocity : VELOCITY;
};

float3 PseudoRandomAxis(uint seed)
{
    float x = frac(sin(seed * 12.9898f) * 43758.5453f);
    float y = frac(sin(seed * 78.233f) * 12345.6789f);
    float z = frac(sin(seed * 45.164f) * 98765.4321f);
    return normalize(float3(x, y, z)); // 정규화된 임의 축 반환
}

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

    // 1. 회전 행렬 생성 (Axis-Angle → Matrix)
    float3 axis = float3(0.0f, 0.0f, 0.0f);
    if (all(input.velocity_and_Rotate.xyz == float3(0.0f, 0.0f, 0.0f)))
    {
        axis = PseudoRandomAxis(input.instanceID);
    }
    else
    {
        axis = normalize(input.velocity_and_Rotate.xyz);
    }

    float angle = input.velocity_and_Rotate.w;
    float3x3 rotation = AxisAngleToMatrix(axis, angle); // 회전 행렬
    
    
    // 2. 크기 적용 및 회전
    float scale = input.Position_and_Scale.w; // 입자 크기
    float3 localPos = input.position * scale; // 스케일 적용
    float3 rotatedPos = mul(rotation, localPos); // 회전 적용

    // 3. 로컬 위치 + 입자 오프셋 (Position_and_Scale.xyz는 로컬 위치)
    float3 particleLocalPos = rotatedPos + input.Position_and_Scale.xyz;

    // 4. 파티클 오브젝트의 월드 변환 적용
    float4 worldPos4 = mul(float4(particleLocalPos, 1.0f), gmtxGameObject);
    output.positionW = worldPos4.xyz;

    // 5. View → Projection까지 최종 위치
    output.position = mul(mul(worldPos4, gmtxView), gmtxProjection);

    // 6. 컬러 전달
    output.color = input.color;

    // 7. 속도 (뷰 공간 기준으로 투영) → 모션 블러, 충돌 시각화 등에 활용 가능
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


//=============================================================================================


struct VS_INSTANCE_BILLBOARD_PARTICLE_DRAW_INPUT
{
    float4 Position_and_Scale : INSTANCE_POS_SCALE;
    float4 Velocity_and_Rotate : INSTANCE_VELOCITY;
    float4 Color : INSTANCE_COLOR;
};

struct GS_BILLBOARD_INPUT
{
    float3 center : CENTER;
    float scale : SCALE;
    float4 color : COLOR;
    float3 velocity : VELOCITY;
};

struct VS_BILLBOARD_OUTPUT
{
    GS_BILLBOARD_INPUT data;
};

struct PS__BILLBOARD_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0; 
};


VS_BILLBOARD_OUTPUT VS_BILLBOARD_PARTICLE_DRAW(VS_INSTANCE_BILLBOARD_PARTICLE_DRAW_INPUT input)
{
    VS_BILLBOARD_OUTPUT output;
    output.data.center = input.Position_and_Scale.xyz;
    output.data.scale = input.Position_and_Scale.w;
    output.data.color = input.Color;
    output.data.velocity = input.Velocity_and_Rotate.xyz;
    return output;
}


static float3 gf3BillboardOffsets[4] = { float3(-10.0f, +10.0f, 0.0f), float3(+10.0f, +10.0f, 0.0f), float3(-10.0f, -10.0f, 0.0f), float3(+10.0f, -10.0f, 0.0f) };
static float2 gf2QuadUVs[4] = { float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) };

[maxvertexcount(4)]
void GS_BILLBOARD_PARTICLE_DRAW(point VS_BILLBOARD_OUTPUT input[1], inout TriangleStream<PS__BILLBOARD_INPUT> triStream)
{
    float3 center = input[0].data.center;
    float scale = input[0].data.scale;
    float4 color = input[0].data.color;

    float3 right = normalize(float3(gmtxInverseView._11, gmtxInverseView._12, gmtxInverseView._13));
    float3 up = normalize(float3(gmtxInverseView._21, gmtxInverseView._22, gmtxInverseView._23));

    for (int i = 0; i < 4; ++i)
    {
        float3 offset = (gf3BillboardOffsets[i].x * right + gf3BillboardOffsets[i].y * up) * scale;
        float3 posWorld = mul(float4(center + offset, 1.0f), gmtxGameObject).xyz;
        float4 clipPos = mul(mul(float4(posWorld, 1.0f), gmtxView), gmtxProjection);

        PS__BILLBOARD_INPUT outp;
        outp.position = clipPos;
        outp.color = color;
        outp.uv = gf2QuadUVs[i];
        triStream.Append(outp);
    }
}

float4 PS_BILLBOARD_PARTICLE_DRAW(PS__BILLBOARD_INPUT input) : SV_Target
{
    float4 base_texture = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
    return base_texture * input.color;
}

//==================================================================

struct VS_TRAIL_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float4 sideTime : TEXCOORD1; // x=side, y=time, z=centerY, w=offsetY
    float ratio : TEXCOORD2; // fading factor (0.0 to 1.0)
};

struct VS_TRAIL_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITIONW;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float time : TEXCOORD1;
    float ratio : TEXCOORD2; 
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

    output.color = input.color;
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

    float3 baseColor = input.color.xyz;
    float3 finalColor = baseColor * glow;

    return float4(finalColor, fade);
}

