cbuffer Frame_Info : register(b0)
{
    float3 boat_pos;
    float ElapsedTime;

    float3 boat_dir;
    float wave_seed;
};

Texture2D<float> HeightMap_Read : register(t0);
RWTexture2D<float> HeightMap_Write : register(u0);
RWTexture2D<float4> NormalMap_Write : register(u1);

[numthreads(8, 8, 1)]
void CS_Global_Wave_Height(uint3 DTid : SV_DispatchThreadID)
{
    // Simple sine wave based on x-position and time
    float2 uv = DTid.xy;
    float wave = sin(uv.x * 0.1 + ElapsedTime) * 0.5 + 0.5; // Normalize to [0,1]
    HeightMap_Write[DTid.xy] = wave;
   
}

[numthreads(8, 8, 1)]
void CS_Boat_Wave_Height(uint3 DTid : SV_DispatchThreadID)
{
    float2 pos = DTid.xy;
    float2 toBoat = pos - boat_pos.xy;
    float dist = length(toBoat);

    // Create a ripple effect around the boat
    float wave = exp(-dist * 0.1) * sin(dist * 0.5 - ElapsedTime * 5.0);
    HeightMap_Write[DTid.xy] += wave;
}

[numthreads(8, 8, 1)]
void CS_Wave_Normal(uint3 DTid : SV_DispatchThreadID)
{
    // Compute gradient from HeightMap_Write (center differences)
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

    float3 dx = float3(2, hR - hL, 0);
    float3 dz = float3(0, hD - hU, 2);
    float3 normal = normalize(cross(dz, dx));

    NormalMap_Write[DTid.xy] = float4(normal * 0.5 + 0.5, 1.0); 
}
