#include "Shaders.hlsl"
#include "Light.hlsl"

Texture2D<float4> T_Albedo_Color : register(t0);
Texture2D<float4> T_World_Position: register(t1);
Texture2D<float4> T_World_Normal_and_Camera_Distance : register(t2);
Texture2D<float4> T_Material_Light_Info : register(t3);


cbuffer cb_Post_Camera : register(b0)
{
    float3 camera_pos;
};

//==================================================================


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
    // 기본 텍스처 샘플링
    float4 colorTexture = T_Albedo_Color.Sample(gssWrap, input.uv);
    float4 world_position = T_World_Position.Sample(gssWrap, input.uv);
    float4 wNormal_CD = T_World_Normal_and_Camera_Distance.Sample(gssWrap, input.uv);
    float4 material_light_info = T_Material_Light_Info.Sample(gssWrap, input.uv);
    
    float3 wNormal = wNormal_CD.xyz;
    float Camera_Distance = wNormal_CD.w;    

    Material material;
    material.gAlbedoColor = colorTexture;
    material.gRoughness = material_light_info.r;
    material.gMetallic = material_light_info.g;
    material.gSpecular_intensity = material_light_info.b;
    material.gEmissive_intensity = material_light_info.a;

    //================================================================

    float4 Light_Color = Lighting(world_position.xyz, wNormal, camera_pos, material);
    
    //================================================================    
    
    // 안개 강도 계산 (카메라와의 거리를 기반)
    float3 fogColor = float3(0.5f, 0.5f, 0.5f); // 안개 색상 (회색)
    float fogStart = 10.0f; // 안개 시작 거리
    float fogEnd = 200.0f; // 안개 끝 거리
    
    // 선형 안개
    float fogFactor = saturate((Camera_Distance - fogStart) / (fogEnd - fogStart)); // 선형 안개

    // Density 가 작으면, 은은하게, 크면, 급격한 안개 형성
    //float fogDensity = 0.02f;
    //float fogFactor = 1.0 - exp(-Camera_Distance * fogDensity);
    
    
    float4 cColor = float4(lerp(Light_Color.rgb, fogColor, fogFactor), 1.0f); // 안개 효과 적용
    
    //================================================================

    return cColor;
    
    }

//=====================================================================

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
    float4 colorTexture = Screen_Texture.Sample(gssWrap, input.uv);
    return colorTexture;
}