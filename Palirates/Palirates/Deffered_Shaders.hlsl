#include "Light.hlsl"

#define NUM_CASCADES 4
#define LIGHT_CAMERA_TYPE_DIRECTIONAL 0


Texture2D<float4> T_Albedo_Color : register(t0);
Texture2D<float4> T_World_Position : register(t1);
Texture2D<float4> T_World_Normal_and_Camera_Distance : register(t2);
Texture2D<float4> T_Velocity : register(t3);
Texture2D<float4> T_ViewSpace_Z : register(t4);

// t4 = Light_Material_Info
Texture2D<float4> T_Fog_Noise : register(t6);
Texture2D<float> gShadowMaps[NUM_CASCADES] : register(t7); // t7 ~ t11

cbuffer cb_Fog_Info : register(b0)
{
    float3 fogColor;
    int Fog_Trigger;
    
    float fogStart;
    float fogEnd;
    float fogDensity;
    float noiseScale;
    
    float noiseStrength;
    float time;
    float2 padding0;
}

cbuffer cb_Post_Camera : register(b1)
{
    float3 camera_pos;
};




cbuffer LightCamera_Info : register(b3)
{
    uint shadow_pass;
    uint light_type;
    uint LightCamera_Info_padding0;
    uint LightCamera_Info_padding1;

    float4x4 LightViewProjTex[NUM_CASCADES];

    float3 LightDirectionWS;
    float shadow_bias;

    float2 shadow_map_size;
    float2 inv_shadow_map_size;

    float CascadeSplits[NUM_CASCADES];
}



SamplerState gssWrap : register(s0);
SamplerComparisonState gssShadowSampler : register(s1);

//==================================================================

// PCF 필터링 함수 (3x3 커널)
float SampleShadowPCF(Texture2D<float> shadowMap, SamplerComparisonState shadow_sampler, float2 uv, float depth, int cascadeIdx)
{
    float shadowSum = 0.0f;
    int kernelSize = 0;
    int count = 0;
    [unroll]
    for (int dx = -kernelSize; dx <= kernelSize; ++dx)
    {
        [unroll]
        for (int dy = -kernelSize; dy <= kernelSize; ++dy)
        {
            float2 offset = float2(dx, dy) * inv_shadow_map_size[cascadeIdx];
            shadowSum += shadowMap.SampleCmpLevelZero(shadow_sampler, uv + offset, depth);
            count++;
        }
    }
    return shadowSum / count;
}

float CalcCSMShadowFactor(float3 worldPos, float viewZ)
{
    int cascadeIdx = 0;
    [unroll]
    for (int i = 0; i < NUM_CASCADES; ++i)
    {
        if (viewZ < CascadeSplits[i])
        {
            cascadeIdx = i;
            break;
        }
    }

    float shadowFactor = 1.0f;

    if (shadow_pass == 1 && light_type == LIGHT_CAMERA_TYPE_DIRECTIONAL)
    {
        const float transitionRange = 0.05f;
        float splitCurr = CascadeSplits[cascadeIdx];
        float blendWeight = 0.0f;
        if (cascadeIdx > 0)
            blendWeight = saturate((viewZ - (splitCurr - transitionRange)) / transitionRange);

        float4 shadowCoord0 = mul(float4(worldPos, 1.0f), LightViewProjTex[cascadeIdx]);
        shadowCoord0 /= shadowCoord0.w;

        float shadow0 = 1.0f;
        if (shadowCoord0.x >= 0.0f && shadowCoord0.x <= 1.0f && shadowCoord0.y >= 0.0f && shadowCoord0.y <= 1.0f && shadowCoord0.z >= 0.0f && shadowCoord0.z <= 1.0f)
        {
            shadow0 = SampleShadowPCF(gShadowMaps[cascadeIdx], gssShadowSampler, shadowCoord0.xy, shadowCoord0.z - shadow_bias, cascadeIdx);
        }

        float shadow1 = shadow0;
        if (cascadeIdx > 0)
        {
            float4 shadowCoord1 = mul(float4(worldPos, 1.0f), LightViewProjTex[cascadeIdx - 1]);
            shadowCoord1 /= shadowCoord1.w;
            if (shadowCoord1.x >= 0.0f && shadowCoord1.x <= 1.0f && shadowCoord1.y >= 0.0f && shadowCoord1.y <= 1.0f && shadowCoord1.z >= 0.0f && shadowCoord1.z <= 1.0f)
            {
                shadow1 = SampleShadowPCF(gShadowMaps[cascadeIdx - 1], gssShadowSampler, shadowCoord1.xy, shadowCoord1.z - shadow_bias, cascadeIdx - 1);
            }
        }
        shadowFactor = lerp(shadow0, shadow1, blendWeight);
    }
    return shadowFactor;
}


float4 Debug_ShadowMap(float2 uv)
{
    float4 color = float4(1, 1, 1, 1);

    float2 quad_uv = uv * 2.0f;
    int cascade_idx = 0;
    float2 shadow_uv = 0;

    if (quad_uv.x < 1.0f && quad_uv.y < 1.0f)
    {
        cascade_idx = 0;
        shadow_uv = quad_uv;
    }
    else if (quad_uv.x >= 1.0f && quad_uv.y < 1.0f)
    {
        cascade_idx = 1;
        shadow_uv = float2(quad_uv.x - 1.0f, quad_uv.y);
    }
    else if (quad_uv.x < 1.0f && quad_uv.y >= 1.0f)
    {
        cascade_idx = 2;
        shadow_uv = float2(quad_uv.x, quad_uv.y - 1.0f);
    }
    else
    {
        cascade_idx = 3;
        shadow_uv = float2(quad_uv.x - 1.0f, quad_uv.y - 1.0f);
    }
    float depth = gShadowMaps[cascade_idx].SampleLevel(gssWrap, shadow_uv, 0);
    color = float4(depth.xxx, 1);

    return color;
}

int FindCascadeIdx(float viewZ, float CascadeSplits[NUM_CASCADES])
{
    int cascadeIdx = 0;
    [unroll]
    for (int i = 0; i < NUM_CASCADES; ++i)
    {
        if (viewZ < CascadeSplits[i])
        {
            cascadeIdx = i;
            break;
        }
    }
    return cascadeIdx;
}
//================================================================




//================================================================


struct VS_TEXTURED_SCREEN_RECT_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_TEXTURED_SCREEN_RECT_OUTPUT VS_Textured_ScreenRect(uint nVertexID : SV_VertexID)
{
    VS_TEXTURED_SCREEN_RECT_OUTPUT output = (VS_TEXTURED_SCREEN_RECT_OUTPUT) 0;

    if (nVertexID == 0)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 0.0f);
    }
    else if (nVertexID == 1)
    {
        output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 0.0f);
    }
    else if (nVertexID == 2)
    {
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 1.0f);
    }
    else if (nVertexID == 3)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 0.0f);
    }
    else if (nVertexID == 4)
    {
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 1.0f);
    }
    else if (nVertexID == 5)
    {
        output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 1.0f);
    }

    return (output);
}


float4 PS_Textured_ScreenRect(VS_TEXTURED_SCREEN_RECT_OUTPUT input) : SV_Target
{
    float4 colorTexture = T_Albedo_Color.Sample(gssWrap, input.uv);
    float4 world_position = T_World_Position.Sample(gssWrap, input.uv);
    float4 wNormal_CD = T_World_Normal_and_Camera_Distance.Sample(gssWrap, input.uv);
    float viewspace_Z = T_ViewSpace_Z.Sample(gssWrap, input.uv).x;

    float3 wNormal = wNormal_CD.xyz;
    float Camera_Distance = wNormal_CD.w;
    uint materialID = (uint) (colorTexture.a * 255.0f + 0.5f);



    //    return Debug_ShadowMap(input.uv);
    
    float shadowFactor = CalcCSMShadowFactor(world_position.xyz, viewspace_Z);

    

    bool isEmptyPixel = all(wNormal == 0.0f) || Camera_Distance == 0.0f;
    if (isEmptyPixel && Fog_Trigger == 0)
    {
        discard;
    }
    
    // ======= Lighting calculation =======
    float3 Light_Color = Lighting(world_position.xyz, wNormal, camera_pos, colorTexture.rgb, materialID, shadowFactor).rgb;

    // FOG calculationW
    float2 baseUV = world_position.xz - camera_pos.xz;
    float fogFactor = saturate((Camera_Distance - fogStart) / (fogEnd - fogStart));
    fogFactor = pow(fogFactor, fogDensity);
    fogFactor = lerp(0.1f, 1.0f, fogFactor);

    float2 noiseUV1 = baseUV * 0.002f + float2(time * 0.05f, time * 0.05f);
    float2 noiseUV2 = baseUV * 0.0033f + float2(time * 0.013f, time * 0.015f);
    float2 distortion = sin(baseUV.yx * 13.0 + time * 0.2f) * 0.001f;
    noiseUV1 += distortion;
    noiseUV2 += distortion;

    float n1 = T_Fog_Noise.Sample(gssWrap, noiseUV1).r;
    float n2 = T_Fog_Noise.Sample(gssWrap, noiseUV2).r;
    float noiseVal = (n1 + n2) * 0.5f;
    float smoothNoise = smoothstep(0.3f, 0.7f, noiseVal);

    float tintStrength = 0.03f;
    float3 baseFogColor = fogColor * 1.2f;
    float3 modFogColor = baseFogColor * (1.0 - tintStrength + smoothNoise * tintStrength * 2.0);
    float3 finalFogColor = modFogColor;

    float3 foggedColor = lerp(Light_Color, finalFogColor, fogFactor);

    if (isEmptyPixel)
    {
        return Fog_Trigger == 1 ? float4(finalFogColor, 1.0f) : float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    return Fog_Trigger == 1 ? float4(foggedColor, 1.0f) : float4(Light_Color, 1.0f);
}


//-------------------------------------------------------------------------------------------------------------


Texture2D<float4> Screen_Texture : register(t0);

VS_TEXTURED_SCREEN_RECT_OUTPUT VS_FullScreen(uint nVertexID : SV_VertexID)
{
    VS_TEXTURED_SCREEN_RECT_OUTPUT output = (VS_TEXTURED_SCREEN_RECT_OUTPUT) 0;

    if (nVertexID == 0)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 0.0f);
    }
    else if (nVertexID == 1)
    {
        output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 0.0f);
    }
    else if (nVertexID == 2)
    {
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 1.0f);
    }
    else if (nVertexID == 3)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 0.0f);
    }
    else if (nVertexID == 4)
    {
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 1.0f);
    }
    else if (nVertexID == 5)
    {
        output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 1.0f);
    }

    return (output);
}

float4 PS_FullScreen(VS_TEXTURED_SCREEN_RECT_OUTPUT input) : SV_Target
{
    float3 colorTexture = Screen_Texture.Sample(gssWrap, input.uv).xyz;

    return float4(colorTexture, 1.0f);
    

}