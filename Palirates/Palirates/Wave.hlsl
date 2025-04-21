//cbuffer Frame_Info : register(b0)
//{
//    float3 boat_pos;
//    float ElapsedTime;

//    float3 boat_dir;
//    float TotalTime;
//};

cbuffer WaveParams : register(b0)
{
     float g_WaveSpeed; // 파도 진행 속도
    float g_HeightDamping; // 파형의 감쇠율 (lerp)
    float g_WaveMin; // 파형 최소값
    float g_WaveMax; // 파형 최대값
    float g_BaseSpacing; // 주기 기본값 (layer 기반 spacing에 사용)
    float g_BaseSharpness; // 파형 날카로움 (tri → peak)
    float g_BandSize; // 단층 구분 높이 (y축 기준)
    float g_AngleOffsetPerBand; // 각 층별 진행 방향 차이 (단위: rad)

    float g_WakeMaxDist; // 보트 wake 영향 거리
    float g_WakeMaxAngle; // 퍼짐 각도 (rad)
    float g_WakeDepthStrength; // 깊이 강도 (파임 정도)
    float g_WakeDecay; // 중심부 감쇠율 (높을수록 중심 뾰족)

    float2 g_BoatPos; // 보트 위치
    float2 g_BoatDir; // 보트 진행 방향 (정규화)

    float g_TotalTime; // 전역 시간 (파형 움직임)
    float _padding; // 16바이트 정렬용
};


Texture2D<float> HeightMap_Read : register(t0);
RWTexture2D<float> HeightMap_Write : register(u0);
RWTexture2D<float4> NormalMap_Write : register(u1);

RWStructuredBuffer <float4>Write_Pos_Normal : register(u2);


[numthreads(8, 8, 1)]
void CS_Global_Wave_Height(uint3 DTid : SV_DispatchThreadID)
{
    float2 uv = DTid.xy;
    float prev = HeightMap_Read[uv];

    // === Band blending (per y layer) ===
    float bandF = uv.y / g_BandSize;
    float band = floor(bandF);
    float t = frac(bandF);
    float s = t * t * (3.0 - 2.0 * t); // smoothstep

    // === Interpolated angle ===
    float angle0 = band * g_AngleOffsetPerBand;
    float angle1 = (band + 1.0) * g_AngleOffsetPerBand;
    float angle = lerp(angle0, angle1, s);
    float2 dir = float2(cos(angle), sin(angle));

    // === Interpolated attributes ===
    float spacing0 = g_BaseSpacing + band * 0.003;
    float spacing1 = g_BaseSpacing + (band + 1.0) * 0.003;
    float spacing = lerp(spacing0, spacing1, s);

    float sharpness0 = saturate(g_BaseSharpness - band * 0.02);
    float sharpness1 = saturate(g_BaseSharpness - (band + 1.0) * 0.02);
    float sharpness = lerp(sharpness0, sharpness1, s);

    float bandOffset = lerp(band * 0.7, (band + 1.0) * 0.7, s);

    // === Wave generation ===
    float x = frac(dot(uv, dir) * spacing + g_TotalTime * g_WaveSpeed + bandOffset);
    float tri = abs(x * 2.0 - 1.0);
    float shaped = pow(tri, sharpness);
    float wave01 = 1.0 - shaped;

    float wave = lerp(g_WaveMin, g_WaveMax, wave01);
    float result = lerp(prev, wave, g_HeightDamping);

    HeightMap_Write[uv] = result;
}



[numthreads(8, 8, 1)]
void CS_Boat_Wave_Height(uint3 DTid : SV_DispatchThreadID)
{
    float2 coord = DTid.xy;
    float2 dir = normalize(g_BoatDir);
    float2 toPix = coord - g_BoatPos;

    float forwardDist = dot(toPix, dir);
    if (forwardDist < 0.0 || forwardDist > g_WakeMaxDist)
    {
        HeightMap_Write[coord] = HeightMap_Read[coord];
        return;
    }

    float2 lateral = toPix - forwardDist * dir;
    float sideDist = length(lateral);
    float angle = atan2(sideDist, forwardDist);

    if (angle > g_WakeMaxAngle)
    {
        HeightMap_Write[coord] = HeightMap_Read[coord];
        return;
    }

    float angleRatio = angle / g_WakeMaxAngle;
    float sideWeight = pow(1.0 - angleRatio, g_WakeDecay);
    float forwardWeight = 1.0 - (forwardDist / g_WakeMaxDist);

    float depth = sideWeight * forwardWeight;

    float base = HeightMap_Read[coord];
    float result = saturate(base - depth * g_WakeDepthStrength);

    HeightMap_Write[coord] = result;
}


[numthreads(8, 8, 1)]
void CS_Wave_Normal(uint3 DTid : SV_DispatchThreadID)
{
    // Compute gradient from HeightMap_Write (center differences)
    float HeightScale = 50.0f;
    int2 texSize;
    HeightMap_Write.GetDimensions(texSize.x, texSize.y);

    int2 coord = DTid.xy;
    int2 left = int2(max(coord.x - 1, 0), coord.y);
    int2 right = int2(min(coord.x + 1, texSize.x - 1), coord.y);
    int2 up = int2(coord.x, max(coord.y - 1, 0));
    int2 down = int2(coord.x, min(coord.y + 1, texSize.y - 1));

    float hL = HeightMap_Write[left];
    float hR = HeightMap_Write[right];
    float hU = HeightMap_Write[up];
    float hD = HeightMap_Write[down];

    float3 dx = float3(2, (hR - hL) * HeightScale, 0);
    float3 dz = float3(0, (hD - hU) * HeightScale, 2);
    float3 normal = normalize(cross(dz, dx));

    NormalMap_Write[DTid.xy] = float4(normal * 0.5 + 0.5, 1.0); 
    
    int2 boatCoord = int2(round(g_BoatPos.xy)); 
    
    float base = HeightMap_Read[DTid.xy];
    HeightMap_Write[DTid.xy] = base;
    
    if (all(coord == boatCoord))
    {
        Write_Pos_Normal[0] = float4(normal, base);
    }
}
