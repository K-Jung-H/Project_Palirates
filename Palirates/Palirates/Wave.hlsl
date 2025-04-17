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

[numthreads(8, 8, 1)]
void CS_Global_Wave_Height(uint3 DTid : SV_DispatchThreadID)
{

    float2 uv = DTid.xy;
    float prev = HeightMap_Read[uv];
    
    // 단층 구간 (64 픽셀마다)
    float band = floor(uv.y / 64.0);
    float phaseOffset = band * 1.0; 
    float amplitude = 0.3 + band * 0.05;
    float wave = sin(uv.x * 0.05 + TotalTime * 2.0 + phaseOffset) * amplitude + 0.5;
    float result = lerp(prev, wave, 0.02);
    HeightMap_Write[uv] = result;
}

[numthreads(8, 8, 1)]
void CS_Boat_Wave_Height(uint3 DTid : SV_DispatchThreadID)
{
    float2 pos = DTid.xy;
    float2 toBoat = pos - boat_pos.xy;
    float2 dir = normalize(boat_dir.xy);

    float forwardDist = dot(toBoat, dir); // dir 방향 거리
    float2 lateral = toBoat - forwardDist * dir; // 수직 벡터
    float sideDist = length(lateral); // 중심축 기준 좌우 거리

    float base = HeightMap_Read[DTid.xy];

    float maxForward = 100.0f;
    float maxSideAtEnd = 500.0f; // 가장 끝에서의 최대 폭

    if (forwardDist < 0 || forwardDist > maxForward)
    {
        HeightMap_Write[DTid.xy] = base;
        return;
    }

    float maxSide = (forwardDist / maxForward) * maxSideAtEnd;
    if (sideDist > maxSide)
    {
        HeightMap_Write[DTid.xy] = base;
        return;
    }

    float centerRatio = sideDist / maxSide;
    float centerLineWeight = pow(1.0 - centerRatio, 3.0); //  얕아지는 각도

    float forwardWeight = 1.0 - (forwardDist / maxForward);
    float depth = forwardWeight * centerLineWeight;

    float result = saturate(base - depth * 0.3);
    HeightMap_Write[DTid.xy] = result;

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
    
    
    float base = HeightMap_Read[DTid.xy];
    HeightMap_Write[DTid.xy] = base;
}
