

cbuffer cbFrameInfo : register(b0)
{
    float4 frameInfo; // dummy data
};

cbuffer cbObjectTransform : register(b1) // Object's world matrix
{
    matrix g_mWorld;
};

cbuffer cbCamera : register(b2) // Shadow camera's ViewProjection matrix
{
    matrix g_mViewProjection;
};

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput VS_Shadow(VSInput vin)
{
    VSOutput vout;

    // Transform to world space
    float4 worldPos = mul(float4(vin.position, 1.0f), g_mWorld);

    // Transform to light's shadow view-projection space
    vout.position = mul(worldPos, g_mViewProjection);

    return vout;
}