#include "Shaders.hlsl"
#include "Light.hlsl"

Texture2D<float4> Post_Albedo_Texture : register(t0);
Texture2D<int> Post_Material_ID : register(t1);
Texture2D<float4> Post_View_Normal : register(t2);
Texture2D<float> Post_Depth : register(t3);
Texture2D<float> Post_Camera_Distance : register(t4);

cbuffer cb_Post_Camera : register(b0)
{
    float4 post_camera_pos;
    matrix gmtxInvProjection;
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

// HSV → RGB 변환 함수
float3 HSVtoRGB(float h, float s, float v)
{
    float3 rgb;
    
    float i = floor(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    if (i == 0)
        rgb = float3(v, t, p);
    else if (i == 1)
        rgb = float3(q, v, p);
    else if (i == 2)
        rgb = float3(p, v, t);
    else if (i == 3)
        rgb = float3(p, q, v);
    else if (i == 4)
        rgb = float3(t, p, v);
    else
        rgb = float3(v, p, q);

    return rgb;
}

// Material ID를 색상으로 변환하는 함수
float4 GetMaterialColor(int materialID)
{
    if (materialID == -1)
    {
        return float4(1, 0, 0, 1); 
    }

    // ID 범위 제한 (0 ~ 20)
    materialID = clamp(materialID, 0, 20);

    // ID를 0 ~ 1 사이의 Hue 값으로 변환
    float hue = (float) materialID / 20.0f; // 0 ~ 1 사이의 값
    float3 color = HSVtoRGB(hue, 1.0f, 1.0f); // 채도, 명도는 최대

    return float4(color, 1.0f);
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
    float4 colorTexture = Post_Albedo_Texture.Sample(gssWrap, input.uv);
    float3 vNormal = Post_View_Normal.Sample(gssWrap, input.uv).xyz;
    float cDepth = Post_Depth.Load(uint3((uint) input.position.x, (uint) input.position.y, 0));
    float pixelDistance = Post_Camera_Distance.Load(uint3((uint) input.position.x, (uint) input.position.y, 0)).r;
    
    uint width, height;
    Post_Material_ID.GetDimensions(width, height);
    int pixel_Material_ID = Post_Material_ID.Load(int3(input.uv * float2(width, height), 0));

//    if (pixel_Material_ID == -1)
        return colorTexture;
       
        float4 screenSpacePosition = float4(input.position.xy * 0.5f + 0.5f, input.position.z, 1.0f);
    float4 vPosition = mul(screenSpacePosition, gmtxInvProjection); 

    
 
    float4 colorIllumination = Lighting(vPosition.xyz, vNormal, post_camera_pos.xyz, pixel_Material_ID);

    //================================================================    
    
    // 안개 강도 계산 (카메라와의 거리를 기반)
    float3 fogColor = float3(0.5f, 0.5f, 0.5f); // 안개 색상 (회색)
    float fogStart = 10.0f; // 안개 시작 거리
    float fogEnd = 200.0f; // 안개 끝 거리
    
    // 선형 안개
    float fogFactor = saturate((pixelDistance - fogStart) / (fogEnd - fogStart)); // 선형 안개

    // Density 가 작으면, 은은하게, 크면, 급격한 안개 형성
    //float fogDensity = 0.02f;
    //float fogFactor = 1.0 - exp(-pixelDistance * fogDensity);
    
    
    float4 cColor = lerp(colorTexture, colorIllumination, 0.5f); // 기본 색상 혼합
    //cColor.rgb = lerp(cColor.rgb, fogColor, fogFactor); // 안개 효과 적용
    //================================================================
    
    if (colorTexture.x == 0.0f && 
        colorTexture.y == 0.0f &&
        colorTexture.z == 0.0f)
        return float4(1.0f, 0.0f, 0.0f, 1.0f);
    
    return colorTexture;
//    return colorIllumination;


}
