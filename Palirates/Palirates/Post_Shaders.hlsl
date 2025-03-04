#include "Shaders.hlsl"

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

Texture2D<float4> Post_Texture : register(t0);
Texture2D<float4> Post_Illumination : register(t1);
Texture2D<float4> Post_Normal : register(t2);

Texture2D<float> Post_Depth : register(t3);
Texture2D<float> Post_Z_Depth : register(t4);

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
    float4 colorTexture = Post_Texture.Sample(gssWrap, input.uv);
    float4 colorIllumination = Post_Illumination.Sample(gssWrap, input.uv);
    float4 colorNormal = Post_Normal.Sample(gssWrap, input.uv);
    
    // 플레이어와 거리
    float pixelDistance = Post_Depth.Load(uint3((uint) input.position.x, (uint) input.position.y, 0)).r;

    // 뷰 공간 깊이 샘플링
    float4 Depth = Post_Z_Depth.Load(uint3((uint) input.position.x, (uint) input.position.y, 0));


    // 안개 강도 계산 (카메라와의 거리를 기반)
    float fogStart = 10.0f; // 안개 시작 거리
    float fogEnd = 200.0f; // 안개 끝 거리
    
    // 선형 안개
    float fogFactor = saturate((pixelDistance - fogStart) / (fogEnd - fogStart)); // 선형 안개


    // Density 가 작으면, 은은하게, 크면, 급격한 안개 형성
    //float fogDensity = 0.02f;
    //float fogFactor = 1.0 - exp(-pixelDistance * fogDensity);
    
    float3 fogColor = float3(0.5f, 0.5f, 0.5f); // 안개 색상 (회색)
    float4 cColor = lerp(colorTexture, colorIllumination, 0.5f); // 기본 색상 혼합
    cColor.rgb = lerp(cColor.rgb, fogColor, fogFactor); // 안개 효과 적용


 //   return float4(pixelDistance.xxx, 1.0f);
    
    
    return cColor;
}
//==================================================================