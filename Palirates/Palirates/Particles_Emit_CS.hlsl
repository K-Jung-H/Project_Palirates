

struct Particle_Info
{
    float3 Position;
    float Lifetime;

    float3 Velocity;
    float MaxLifetime;

    float3 Acceleration;
    float pad1;

    float3 Color;
    float pad2;

    float2 Size;
    uint Type;
    uint Active;
};

struct Render_Instance
{
    float3 Position;
    float3 Velocity;
    float4 Color;
};

cbuffer CB_Particle_Update_Info : register(b0)
{
    uint Max_Particle_N;
    float ElapsedTime;
    float2 pad0;

    float3 EmitRegionMin;
    float pad1;

    float3 EmitRegionMax;
    float pad2;
}

RWStructuredBuffer<Particle_Info> ParticleBuffer_Emit : register(u0);
AppendStructuredBuffer<Render_Instance> RenderInstanceBuffer : register(u1);

RWStructuredBuffer<uint> debug_buffer : register(u2);


float3 RandomEmitPosition(uint id, float3 min, float3 max)
{
    float seed = id * 13.13 + 0.1f;

    float rx = frac(sin(seed) * 43758.5453);
    float rz = frac(sin(seed * 7.77) * 12345.6789);

    float x = lerp(min.x, max.x, rx);
    float z = lerp(min.z, max.z, rz);
    float y = max.y;

    return float3(x, y, z);
}

float3 RandomSpreadDirection(uint id, float3 baseDir, float spreadAmount)
{
    // 랜덤 offset 생성 (XZ 평면 기준, 범위: -0.5 ~ +0.5)
    float seedX = frac(sin(id * 17.17) * 12345.6789);
    float seedY = frac(sin(id * 31.31) * 98765.4321);
    float seedZ = frac(sin(id * 73.73) * 45678.1234);

    float3 offset = float3(
        (seedX - 0.5f) * spreadAmount,
        (seedY - 0.5f) * spreadAmount,
        (seedZ - 0.5f) * spreadAmount
    );


    float3 dir = normalize(baseDir + offset);
    return dir;
}


#define THREAD_COUNT 64
[numthreads(THREAD_COUNT, 1, 1)]
void EmitCS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;

    if (index >= Max_Particle_N)
        return;

    // 디버깅: Emit 호출 수 기록
    InterlockedAdd(debug_buffer[0], 1);

    // 현재 파티클 상태 확인
    Particle_Info p = ParticleBuffer_Emit[index];

    if (p.Active == 1)
        return;

    // 디버깅: Emit 된 인자 개수 저장
    InterlockedAdd(debug_buffer[1], 1);

    // ========================
    // 파티클 초기화
    // ========================
    p.Active = 1;

    // 무작위 위치 (Y는 항상 상단)
    p.Position = RandomEmitPosition(index, EmitRegionMin, EmitRegionMax);
    p.Velocity = RandomSpreadDirection(index, float3(0.0f, -1.0f, 0.0f), 1.0f);
    p.Acceleration = float3(0.0f, -0.0f, 0.0f);

    
    // p.Velocity = float3(0.0f, 0.0f, 0.0f);
    p.Acceleration = float3(0.0f, -9.8f, 0.0f);
    p.Lifetime = 0.0f;
    p.MaxLifetime = 10.0f;
    p.Size = float2(10.0f, 10.0f);
    p.Type = 0;

    switch (p.Type)
    {
        case 0:
            p.Color = float3(1.0f, 1.0f, 0.0f);
            break; // Spark
        case 1:
            p.Color = float3(0.8f, 0.9f, 1.0f);
            break; // Snow
        case 2:
            p.Color = float3(0.7f, 0.6f, 0.4f);
            break; // Sand
        case 3:
            p.Color = float3(0.3f, 0.6f, 1.0f);
            break; // Splash
    }

    ParticleBuffer_Emit[index] = p;
}
