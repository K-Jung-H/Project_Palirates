#include "Shaders.hlsl"
#include "Light.hlsl"

Texture2D<float4> T_Albedo_Color : register(t0);
Texture2D<float4> T_View_Normal : register(t1);
Texture2D<float2> T_Depth_and_CameraDistance : register(t2);
Texture2D<float4> T_Material_Light_Info : register(t3);
Texture2D<float4> T_Emissive_Color: register(t4);


cbuffer cb_Post_Camera : register(b0)
{
    matrix gmtx_Inv_View;
    matrix gmtx_Inv_View_Projection; 
    float4 camera_pos;
};


//==================================================================


float4 VSPostProcessing(uint nVertexID : SV_VertexID) : SV_POSITION
{
    if (nVertexID == 0)
        return (float4(-1.0f, +1.0f, 0.0f, 1.0f));
    if (nVertexID == 1)
        return (float4(+1.0f, +1.0f, 0.0f, 1.0f));
    if (nVertexID == 2)
        return (float4(+1.0f, -1.0f, 0.0f, 1.0f));

    if (nVertexID == 3)
        return (float4(-1.0f, +1.0f, 0.0f, 1.0f));
    if (nVertexID == 4)
        return (float4(+1.0f, -1.0f, 0.0f, 1.0f));
    if (nVertexID == 5)
        return (float4(-1.0f, -1.0f, 0.0f, 1.0f));

    return (float4(0, 0, 0, 0));
}

float4 PSPostProcessing(float4 position : SV_POSITION) : SV_Target
{
    return (float4(0.0f, 0.0f, 0.0f, 1.0f));
}


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
    float4 vNormal = T_View_Normal.Sample(gssWrap, input.uv);
    float2 Depth_and_Distance = T_Depth_and_CameraDistance.Sample(gssWrap, input.uv);
    float vDepth = Depth_and_Distance.r;
    float Camera_Distance = Depth_and_Distance.g;
    float4 material_light_info = T_Material_Light_Info.Sample(gssWrap, input.uv);
    float4 colorEmissive = T_Emissive_Color.Sample(gssWrap, input.uv);
    
    Material material;
    material.gAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    material.gEmissiveColor = colorEmissive;
    material.gRoughness = material_light_info.r;
    material.gMetallic = material_light_info.g;
    material.gSpecular = material_light_info.b;

    //================================================================
    
    float2 screenPos = input.uv * 2.0f - 1.0f;
    float4 clipSpacePos = float4(screenPos.x, screenPos.y, vDepth, 1.0f);
    float4 viewSpacePos = mul(clipSpacePos, gmtx_Inv_View_Projection);
    viewSpacePos /= viewSpacePos.w;
    
    float4 worldPos = mul(viewSpacePos, gmtx_Inv_View);
    float3 worldNormal = normalize(mul(vNormal.xyz, (float3x3) gmtx_Inv_View));
    
    
    float4 finalColor = Lighting(worldPos.xyz, worldNormal, camera_pos.xyz, material);

    finalColor += material.gEmissiveColor;
    return finalColor;
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
    
    
    //float4 cColor  = lerp(colorTexture, colorIllumination, 0.5f); // 기본 색상 혼합
    //cColor.rgb = lerp(cColor.rgb, fogColor, fogFactor); // 안개 효과 적용
    
    //================================================================

    float4 cColor = finalColor;
    cColor.rgb = lerp(cColor.rgb, fogColor, fogFactor); // 안개 효과 적용
    return cColor;

    
    }
