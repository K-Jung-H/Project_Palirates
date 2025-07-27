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
    float g_hp;
    int ui_type;
    float start_time;
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

#define UI_EFFECT_CUT_HP     (1 << 0) // 1
#define UI_EFFECT_FADE_OUT   (1 << 1) // 2
#define UI_EFFECT_SLIDE_DOWN   (1 << 2) // 4
#define UI_EFFECT_FADE_IN    (1 << 3) // 8
#define UI_EFFECT_TRANSLUCENT   (1 << 4) // 16

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
    
    int type = (int) (ui_type + 0.5f);

    float elapsed = gfCurrentTime - start_time;
    
    float2 uv = input.uv;
    float4 result = gtxtAlbedoTexture.Sample(gssClamp, uv);

// Hover 
    if (g_isHovered)
    {
        float pulse = 0.5f + 0.5f * sin(gfCurrentTime * 6.0f);
        float glowStrength = 0.3f;
        float4 glow = g_hoverGlowColor * pulse * glowStrength;

        result.rgb += glow.rgb;
        result *= g_tintColor;
    }

// HP CUT
    if ((type & UI_EFFECT_CUT_HP) != 0)
    {
        float alpha = saturate(elapsed / 1.0f);
        result.a *= alpha;
        if (uv.x > g_hp)
            discard;
    }

// Fade-out
    if ((type & UI_EFFECT_FADE_OUT) != 0)
    {
        float fadeStart = g_hp - 1.0f; 
        float t = saturate((elapsed - fadeStart) / 1.0f); 
        float alpha = 1.0f - t; 

        result.a *= saturate(alpha);
    }

// Slide up
    if ((type & UI_EFFECT_SLIDE_DOWN) != 0)
    {
        float slideSpeed = 1.0f / g_hp;
        float offsetY = saturate(elapsed * slideSpeed);
        float2 movingUV = float2(uv.x, uv.y + (1.0 - offsetY));

        if (movingUV.y > 1.0f)
            discard;

        result = gtxtAlbedoTexture.Sample(gssClamp, movingUV);
        result *= g_tintColor;
    }

// Fade-in
    if ((type & UI_EFFECT_FADE_IN) != 0)
    {
        float fadeInDuration = g_hp;
        float alpha = saturate(elapsed / fadeInDuration);
        result.a *= alpha;
    }
    
// TRANSLUCENT
    if ((type & UI_EFFECT_TRANSLUCENT) != 0)
    {
        result.a *= 0.3f;
    }
    
    return result;
}