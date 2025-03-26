Texture2D gtxtInput : register(t0);
RWTexture2D<float4> gtxtRWOutput : register(u0);

#define _WITH_BY_LUMINANCE
#define _WITH_GROUPSHARED_MEMORY
#define _WITH_SOBEL_EDGE
#define _WITH_TOON_EDGE

#define CX_THREADS 32
#define CY_THREADS 32

static float3 gf3ToLuminance = float3(0.3f, 0.59f, 0.11f);

#ifdef _WITH_GROUPSHARED_MEMORY
groupshared float4 gf4GroupSharedCache[CX_THREADS + 2][CY_THREADS + 2];
#endif

static float gfLaplacians[9] = { -1, -1, -1, -1, 8, -1, -1, -1, -1 };
static int2 gnOffsets[9] =
{
    { -1, -1 },
    { 0, -1 },
    { 1, -1 },
    { -1, 0 },
    { 0, 0 },
    { 1, 0 },
    { -1, 1 },
    { 0, 1 },
    { 1, 1 }
};

void SobelEdge(uint3 tid, uint3 gid)
{
    float h = dot(gf3ToLuminance,
        -gf4GroupSharedCache[tid.x][tid.y + 1].rgb +
        2.0 * gf4GroupSharedCache[tid.x + 1][tid.y + 1].rgb +
        -gf4GroupSharedCache[tid.x + 2][tid.y + 1].rgb);

    float v = dot(gf3ToLuminance,
        -gf4GroupSharedCache[tid.x + 1][tid.y].rgb +
        2.0 * gf4GroupSharedCache[tid.x + 1][tid.y + 1].rgb +
        -gf4GroupSharedCache[tid.x + 1][tid.y + 2].rgb);

    float edge = sqrt(h * h + v * v) * 1.3f;
    edge = saturate(edge);

    gtxtRWOutput[gid.xy] = float4(edge.xxx, 1.0f);
}

void SobelEdge_Toon(uint3 tid, uint3 gid)
{
    float h = dot(gf3ToLuminance,
        -gf4GroupSharedCache[tid.x][tid.y + 1].rgb +
        2.0 * gf4GroupSharedCache[tid.x + 1][tid.y + 1].rgb +
        -gf4GroupSharedCache[tid.x + 2][tid.y + 1].rgb);

    float v = dot(gf3ToLuminance,
        -gf4GroupSharedCache[tid.x + 1][tid.y].rgb +
        2.0 * gf4GroupSharedCache[tid.x + 1][tid.y + 1].rgb +
        -gf4GroupSharedCache[tid.x + 1][tid.y + 2].rgb);

    float edge = sqrt(h * h + v * v) * 1.3f;
    edge = saturate(edge);

    float3 original = gtxtInput[gid.xy].rgb;
    float3 finalColor = original * (1.0 - edge);

    gtxtRWOutput[gid.xy] = float4(finalColor, 1.0f);
}

void LaplacianEdge(uint3 tid, uint3 gid)
{
    float sum = 0.0f;
    for (int i = 0; i < 9; ++i)
    {
        float3 sample = gf4GroupSharedCache[tid.x + 1 + gnOffsets[i].x][tid.y + 1 + gnOffsets[i].y].rgb;
        sum += gfLaplacians[i] * dot(gf3ToLuminance, sample);
    }

    float edge = abs(sum);
    edge = saturate(edge);

    gtxtRWOutput[gid.xy] = float4(edge.xxx, 1.0f);
}

void LaplacianEdge_Toon(uint3 tid, uint3 gid)
{
    float sum = 0.0f;
    for (int i = 0; i < 9; ++i)
    {
        float3 sample = gf4GroupSharedCache[tid.x + 1 + gnOffsets[i].x][tid.y + 1 + gnOffsets[i].y].rgb;
        sum += gfLaplacians[i] * dot(gf3ToLuminance, sample);
    }

    float edge = abs(sum);
    edge = saturate(edge);

    float3 original = gtxtInput[gid.xy].rgb;
    float3 finalColor = original * (1.0 - edge);

    gtxtRWOutput[gid.xy] = float4(finalColor, 1.0f);
}

[numthreads(CX_THREADS, CY_THREADS, 1)]
void CS_EdgeDetection(uint3 tid : SV_GroupThreadID, uint3 gid : SV_DispatchThreadID)
{
    uint2 texSize;
    gtxtInput.GetDimensions(texSize.x, texSize.y);

    bool isValid = (gid.x < texSize.x && gid.y < texSize.y);

#ifdef _WITH_GROUPSHARED_MEMORY
    if (isValid)
        gf4GroupSharedCache[tid.x + 1][tid.y + 1] = gtxtInput[gid.xy];

    bool left = tid.x == 0;
    bool right = tid.x == CX_THREADS - 1;
    bool top = tid.y == 0;
    bool bottom = tid.y == CY_THREADS - 1;

    if (isValid && left)
        gf4GroupSharedCache[0][tid.y + 1] = gtxtInput[gid.xy + uint2(-1, 0)];
    if (isValid && right)
        gf4GroupSharedCache[CX_THREADS + 1][tid.y + 1] = gtxtInput[gid.xy + uint2(1, 0)];
    if (isValid && top)
        gf4GroupSharedCache[tid.x + 1][0] = gtxtInput[gid.xy + uint2(0, -1)];
    if (isValid && bottom)
        gf4GroupSharedCache[tid.x + 1][CY_THREADS + 1] = gtxtInput[gid.xy + uint2(0, 1)];

    if (isValid && left && top)
        gf4GroupSharedCache[0][0] = gtxtInput[gid.xy + uint2(-1, -1)];
    if (isValid && right && top)
        gf4GroupSharedCache[CX_THREADS + 1][0] = gtxtInput[gid.xy + uint2(1, -1)];
    if (isValid && left && bottom)
        gf4GroupSharedCache[0][CY_THREADS + 1] = gtxtInput[gid.xy + uint2(-1, 1)];
    if (isValid && right && bottom)
        gf4GroupSharedCache[CX_THREADS + 1][CY_THREADS + 1] = gtxtInput[gid.xy + uint2(1, 1)];

    GroupMemoryBarrierWithGroupSync();

    if (isValid)
    {
#ifdef _WITH_TOON_EDGE
#ifdef _WITH_SOBEL_EDGE
        SobelEdge_Toon(tid, gid);
#else
        LaplacianEdge_Toon(tid, gid);
#endif
#else
#ifdef _WITH_SOBEL_EDGE
        SobelEdge(tid, gid);
#else
        LaplacianEdge(tid, gid);
#endif
#endif
    }
#endif
}
