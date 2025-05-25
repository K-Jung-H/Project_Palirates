cbuffer Frame_Info : register(b0)
{
    float gfCurrentTime; 
    float gfElapsedTime; 
};

Texture2D gtxtAlbedoTexture : register(t0);

SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);

cbuffer UIConstants : register(b1) 
{
    float4 g_tintColor;
    float4 g_hoverGlowColor;
    float g_isHovered;
    float3 padding;
};


struct VS_UI_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_UI_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_UI_OUTPUT VS_UI(VS_UI_INPUT input)
{
    VS_UI_OUTPUT output;
    output.position = float4(input.position.xy, 0.0f, 1.0f); 
    output.uv = input.uv;
    return output;
}

float4 PS_UI(VS_UI_OUTPUT input) : SV_TARGET
{
    // scan
    //float2 uv = input.uv;
    //float4 texColor = gtxtAlbedoTexture.Sample(gssClamp, uv);

    //float scanY = fmod(gfCurrentTime * 0.5f, 1.0f);

    //float thickness = 0.02f;
    //float scanGlow = smoothstep(thickness, 0.0f, abs(uv.y - scanY));

    //float4 scanColor = g_borderColor * scanGlow;

    //float4 baseColor = texColor * g_tintColor;

    //return g_isHovered ? baseColor + scanColor : baseColor;
    
    float2 uv = input.uv;

    float4 texColor = gtxtAlbedoTexture.Sample(gssClamp, uv);
    float4 result = texColor;

    if (g_isHovered)
    {
        float pulse = 0.5f + 0.5f * sin(gfCurrentTime * 6.0f);
        float glowStrength = 0.3f;
        float4 glow = g_hoverGlowColor * pulse * glowStrength;

        result.rgb += glow.rgb;
        result *= g_tintColor;
    }

    return result;
}