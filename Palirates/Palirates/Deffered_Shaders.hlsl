#include "Light.hlsl"

Texture2D<float4> T_Albedo_Color : register(t0);
Texture2D<float4> T_World_Position: register(t1);
Texture2D<float4> T_World_Normal_and_Camera_Distance : register(t2);
Texture2D<float4> T_Velocity : register(t3);
// t4 = Light_Material_Info
Texture2D<float4> T_Fog_Noise : register(t5);
Texture2D<float> T_Fixed_ShadowMap : register(t6);

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

    float4x4 LightViewProjTex;

    float3 LightDirectionWS;
    float shadow_bias;

    float3 LightPositionWS;
    float LightCamera_Info_padding2;

    float2 shadow_map_size;
    float2 inv_shadow_map_size;
};



SamplerState gssWrap : register(s0);
SamplerComparisonState gssShadowSampler : register(s1);

//==================================================================
float4 GetDebugColorFromID(uint id)
{
    // Hashing을 이용해 간단한 무작위 색상 생성
    uint hash = id * 1664525u + 1013904223u;

    float r = ((hash >> 16) & 0xFF) / 255.0f;
    float g = ((hash >> 8) & 0xFF) / 255.0f;
    float b = (hash & 0xFF) / 255.0f;

    if (id == 0)
        return float4(1.0f, 1.0f, 1.0f, 1.0f);
    return float4(r, g, b, 1.0f);
}

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



#define LIGHT_CAMERA_TYPE_DIRECTIONAL 0


float4 PS_Textured_ScreenRect(VS_TEXTURED_SCREEN_RECT_OUTPUT input) : SV_Target
{
    float4 colorTexture = T_Albedo_Color.Sample(gssWrap, input.uv);
    float4 world_position = T_World_Position.Sample(gssWrap, input.uv);
    float4 wNormal_CD = T_World_Normal_and_Camera_Distance.Sample(gssWrap, input.uv);

    float3 wNormal = wNormal_CD.xyz;
    float Camera_Distance = wNormal_CD.w;

    uint materialID = (uint) (colorTexture.a * 255.0f + 0.5f);

    //float depth = T_Fixed_ShadowMap.Sample(gssWrap, input.uv);
    //return float4(depth.xxx, 1.0f);

    // ==================== Shadow Mapping ====================
    float shadowFactor = 1.0f;

    if (shadow_pass == 1 && light_type == LIGHT_CAMERA_TYPE_DIRECTIONAL)
    {
        float4 shadowCoord = mul(float4(world_position.xyz, 1.0f), LightViewProjTex);
        shadowCoord /= shadowCoord.w;

        bool inShadowMap =
            shadowCoord.x >= 0.0f && shadowCoord.x <= 1.0f &&
            shadowCoord.y >= 0.0f && shadowCoord.y <= 1.0f &&
            shadowCoord.z >= 0.0f && shadowCoord.z <= 1.0f;

        if (inShadowMap)
        {
            shadowFactor = T_Fixed_ShadowMap.SampleCmpLevelZero(gssShadowSampler, shadowCoord.xy, shadowCoord.z - shadow_bias);
        }
    }
    
    // ==================== Lighting ====================

    // 조명 계산
    float3 Light_Color = Lighting(world_position.xyz, wNormal, camera_pos, colorTexture.rgb, materialID, shadowFactor).rgb;
    Light_Color = lerp(float3(0.0f, 0.0f, 0.0f), Light_Color, shadowFactor);
    // ==================== FOG 처리 ====================
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

    // ==================== 비어 있는 픽셀 처리 ====================
    bool isEmptyPixel = all(wNormal == 0.0f) || Camera_Distance == 0.0f;
    if (isEmptyPixel)
    {
        return Fog_Trigger == 1 ? float4(finalFogColor, 1.0f) : float4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    return Fog_Trigger == 1 ? float4(foggedColor, 1.0f) : float4(Light_Color, 1.0f);
}



//-------------------------------------------------------------------------------------------------------------


Texture2D<float4> Screen_Texture: register(t0);

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