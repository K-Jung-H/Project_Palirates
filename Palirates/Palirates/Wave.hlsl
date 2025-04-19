cbuffer Frame_Info : register(b0)
{
    float3 boat_pos;
    float ElapsedTime;

    float3 boat_dir;
    float TotalTime;
};

Texture2D<float> HeightMap_Read : register(t0);
RWTexture2D<float> HeightMap_Write : register(u0);
RWTexture2D<float4> NormalMap_Write : register(u1);

RWStructuredBuffer <float4>Write_Pos_Normal : register(u2);


[numthreads(8, 8, 1)]
void CS_Global_Wave_Height(uint3 DTid : SV_DispatchThreadID)
{
    // === 기본 파라미터 ===
    float waveMin = 0.0;
    float waveMax = 1.0;
    float baseSpacing = 0.01;
    float baseSharpness = 0.9;
    float baseSpeed = 0.5;
    float dampingFactor = 0.02;

    float2 uv = DTid.xy;
    float prev = HeightMap_Read[uv];

    // === 단층 계산 ===
    float bandSize = 32.0;
    float band = floor(uv.y / bandSize);

    // === 각 단층마다 진행 방향 변경 (약간 회전) ===
    float baseAngle = 0.0;
    float angleOffsetPerBand = 5.1; // 라디안 단위 (ex: 0.1 ~ 5.7도)
    float angle = baseAngle + band * angleOffsetPerBand;
    float2 dir = float2(cos(angle), sin(angle)); // 진행 방향

    // === 각 층별 속도/형태 변화 ===
    float bandOffset = band * 0.7;
    float spacing = baseSpacing + band * 0.003;
    float sharpness = saturate(baseSharpness - band * 0.02);

    // === 파형 생성 ===
    float x = frac(dot(uv, dir) * spacing + TotalTime * baseSpeed + bandOffset);
    float tri = abs(x * 2.0 - 1.0);
    float shaped = pow(tri, sharpness);
    float wave01 = 1.0 - shaped;

    float wave = lerp(waveMin, waveMax, wave01);
    float result = lerp(prev, wave, dampingFactor);

    HeightMap_Write[uv] = result;
}



[numthreads(8, 8, 1)]
void CS_Boat_Wave_Height(uint3 DTid : SV_DispatchThreadID)
{
    // ===== 파라미터 정의 (나중에 CBV로 분리 가능) =====
    float maxDist = 150.0; // 웨이크 길이
    float maxAngle = radians(30.0); // 좌우로 퍼지는 각도 (켈빈 패턴)
    float depthStrength = 1.0; // 전체 깊이 세기
    float decay = 5.0; // 좌우 감쇠율 (클수록 중심 뾰족)

    float2 coord = DTid.xy;
    float2 dir = normalize(boat_dir.xy);
    float2 toPix = coord - boat_pos.xy;

    float forwardDist = dot(toPix, dir);
    if (forwardDist < 0 || forwardDist > maxDist)
    {
        HeightMap_Write[coord] = HeightMap_Read[coord];
        return;
    }

    float2 lateral = toPix - forwardDist * dir;
    float sideDist = length(lateral);
    float angle = atan2(sideDist, forwardDist);

    if (angle > maxAngle)
    {
        HeightMap_Write[coord] = HeightMap_Read[coord];
        return;
    }

    float angleRatio = angle / maxAngle; // 중심축으로부터 좌우 거리 (비율)
    float sideWeight = pow(1.0 - angleRatio, decay); // 중심일수록 값 큼

    float forwardWeight = 1.0 - (forwardDist / maxDist); // 보트 머리 기준 거리

    float depth = sideWeight * forwardWeight;

    float base = HeightMap_Read[coord];
    float result = saturate(base - depth * depthStrength);

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
    
    int2 boatCoord = int2(round(boat_pos.xy)); 
    
    float base = HeightMap_Read[DTid.xy];
    HeightMap_Write[DTid.xy] = base;
    
    if (all(coord == boatCoord))
    {
        Write_Pos_Normal[0] = float4(normal, base);
    }
}
