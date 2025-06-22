Texture2D gtxtInput : register(t0);


RWTexture2D<float4> gtxtRWOutput : register(u0);
Texture2D<float4> gtxtVelocity_Mask_Obj_Id : register(t1);

float3 GetObjectColorById(float objId) // 나중에는 색상 배열을  UAV로 전달받아서 ID 기반 인덱싱하기
{
    if (objId == 0.0f)
        return float3(0.0, 0.0, 0.0); // Black
    else if (objId == 1.0f)
        return float3(1.0, 0.0, 0.0); // Red
    else if (objId == 2.0f)
        return float3(0.0, 1.0, 0.0); // Green
    else if (objId == 3.0f)
        return float3(0.0, 1.0, 1.0); // Cyan
    else
        return float3(1.0, 1.0, 1.0); // White fallback
}

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
    float objId = gtxtVelocity_Mask_Obj_Id[gid.xy].w;

    if (objId >= 10.0f)
    {
        // 그냥 원본 복사 (윤곽선 생략 대상)
        gtxtRWOutput[gid.xy] = gtxtInput[gid.xy];
        return;
    }

    float3 edgeColor = GetObjectColorById(objId);
    float3 original = gtxtInput[gid.xy].rgb;

    // Sobel 계산
    float h = dot(gf3ToLuminance, -gf4GroupSharedCache[tid.x][tid.y + 1].rgb 
        + 2.0 * gf4GroupSharedCache[tid.x + 1][tid.y + 1].rgb 
        + -gf4GroupSharedCache[tid.x + 2][tid.y + 1].rgb);

    float v = dot(gf3ToLuminance, -gf4GroupSharedCache[tid.x + 1][tid.y].rgb 
        + 2.0 * gf4GroupSharedCache[tid.x + 1][tid.y + 1].rgb 
        + -gf4GroupSharedCache[tid.x + 1][tid.y + 2].rgb);

    // 윤곽선 객체일수록 edge 강하게
    float edgeScale = (objId != 0.0f) ? 10.0f : 1.0f;

    float edge = sqrt(h * h + v * v) * 1.3f * edgeScale;
    edge = saturate(edge);

    float3 finalColor = lerp(original, edgeColor, edge);
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


//========================================================================================


static const float BlurScale = 0.5f; // 감도 조절
static const float MaxBlurLength = 0.05f; // 최대 블러 길이 (NDC)
static const float VelocityThreshold = 1e-4f; // 블러 생략 기준

[numthreads(CX_THREADS, CY_THREADS, 1)]
void CS_MotionBlur(uint3 tid : SV_GroupThreadID, uint3 gid : SV_DispatchThreadID)
{
    uint2 texSize;
    gtxtInput.GetDimensions(texSize.x, texSize.y);

    if (gid.x >= texSize.x || gid.y >= texSize.y)
        return;

    float4 baseColor = gtxtInput[gid.xy];
    float4 velocityMaskObjId = gtxtVelocity_Mask_Obj_Id[gid.xy];
    float2 velocity = velocityMaskObjId.xy;
    float mask = velocityMaskObjId.z;

    // If mask is 0, skip blur
    if (mask == 0.0f)
    {
        gtxtRWOutput[gid.xy] = baseColor;
        return;
    }

    // Apply blur sensitivity
    velocity *= BlurScale;

    // Invert Y axis of velocity
    velocity.y *= -1.0f;
    
    // Limit the length
    float len = length(velocity);
    if (len < VelocityThreshold)
    {
        gtxtRWOutput[gid.xy] = baseColor; // Skip blur if velocity is too low
        return;
    }
    if (len > MaxBlurLength)
    {
        velocity = normalize(velocity) * MaxBlurLength;
    }

    // Blur sampling
    const int samples = 5;
    float3 accum = baseColor.rgb;

    for (int i = 1; i <= samples; ++i)
    {
        float2 offset = -velocity * (float(i) / samples);
        float2 sampleUV = (float2) gid.xy + offset * texSize;

        int2 sampleCoord = int2(sampleUV);
        sampleCoord = clamp(sampleCoord, int2(0, 0), int2(texSize - 1));

        float3 sampleColor = gtxtInput[sampleCoord].rgb;
        accum += sampleColor;
    }

    float3 finalColor = accum / (samples + 1);
    gtxtRWOutput[gid.xy] = float4(finalColor, 1.0f);
}

//========================================================================================

SamplerState samplerLinearClamp : register(s0);

#define MAX_OBJECT_NUM 2

cbuffer Zoom_Blur_Info : register(b0)
{
    float4 obj_screen_pos[MAX_OBJECT_NUM]; // if float2(-1.0f,-1.0f)  -> None
    float4 blur_params;
};


[numthreads(CX_THREADS, CY_THREADS, 1)]
void CS_ZoomBlur(uint3 tid : SV_GroupThreadID, uint3 gid : SV_DispatchThreadID)
{
    int2 coord = int2(gid.xy);
    uint2 texSize;
    gtxtInput.GetDimensions(texSize.x, texSize.y);

    if (coord.x < 0 || coord.y < 0 || uint(coord.x) >= texSize.x || uint(coord.y) >= texSize.y)
        return;

    if (obj_screen_pos[0].x == 0.5f && obj_screen_pos[0].y == 0.5f)
    {
        gtxtRWOutput[coord] = float4(1.0f, 0.0f, 0.0f, 1.0f);
        return;
    }
    
    //int samples_as_int = (int) round(blur_params.y); // round()를 사용하여 반올림 처리
    //if (blur_params.x == 1.0f && samples_as_int == 32)
    
        float2 uv = (coord + 0.5) / texSize;


    // 1. 가장 가까운 활성 center를 찾아 그 방향으로 줌 블러 적용
        float2 nearest_center = float2(-1, -1);
        float min_dist = 1e9;
    //[unroll]
    //    for (int j = 0; j < MAX_OBJECT_NUM; ++j)
    //    {
    //        if (obj_screen_pos[j].x < 0.0f)
    //            continue; // 비활성 center skip

    //        float d = length(uv - obj_screen_pos[j]);
    //        if (d < min_dist)
    //        {
    //            min_dist = d;
    //            nearest_center = obj_screen_pos[j];
    //        }
    //    }

        if (nearest_center.x < 0.0f)
        {
        // 유효 중심이 없다면 원본 복사 (또는 디버깅)
            gtxtRWOutput[coord] = gtxtInput[coord];
        // gtxtRWOutput[coord] = float4(1.0f, 0.0f ,0.0f ,1.0f); // 빨강 디버깅
            return;
        }

    // 2. 줌 블러 (최근접 중심점 방향)
    int N = max(blur_params.y, 1);
    float s = blur_params.x;
        float4 accum = float4(0, 0, 0, 0);
        for (int k = 0; k < N; ++k)
        {
            float t = (float) k / (N - 1);
            float2 sample_uv = lerp(uv, nearest_center, t * s);
            accum += gtxtInput.SampleLevel(samplerLinearClamp, sample_uv, 0);
        }
        gtxtRWOutput[coord] = accum / N;
    }