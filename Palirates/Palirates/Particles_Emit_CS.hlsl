

struct Particle_Info
{
    float3 Position;
    float Lifetime;

    float3 Velocity;
    float MaxLifetime;

    float3 Acceleration;
    float Rotate_Value;

    float3 Color;
    float pad1;

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
    float3 EmitRegionMin; 
    float ElapsedTime; 

    float3 EmitRegionMax; 
    uint Max_Particle_N;

    float3 Main_Direction; 
    float Init_Velocity_Value;
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

void Emit_Snow(inout Particle_Info p, uint index)
{
    p.Position = RandomEmitPosition(index, EmitRegionMin, EmitRegionMax);
    float3 dir = RandomSpreadDirection(index, Main_Direction, 0.5f);
    p.Velocity = normalize(dir) * Init_Velocity_Value;

}

void Emit_Spark(inout Particle_Info p, uint index)
{
    float3 center = (EmitRegionMin + EmitRegionMax) * 0.5f;
    p.Position = center;
    float3 dir = RandomSpreadDirection(index, Main_Direction, 2.0f);
    p.Velocity = normalize(dir) * Init_Velocity_Value;
    p.Color = float3(1.0f, 0.0f, 0.0f);

}

void Emit_Water_Splash(inout Particle_Info p, uint index)
{
    float3 center = (EmitRegionMin + EmitRegionMax) * 0.5f;
    p.Position = center;

    float3 baseDir = normalize(Main_Direction);
    float3 right = normalize(cross(float3(0, 1, 0), baseDir));    
    float side = (index % 2 == 0) ? -1.0f : 1.0f;
    float3 liftedBaseDir = normalize(baseDir + float3(0, 0.5f, 0));
    float3 spreadDir = normalize(liftedBaseDir + right * side * 0.5f);
    float3 finalDir = RandomSpreadDirection(index, spreadDir, 0.5f);
    p.Velocity = normalize(finalDir) * Init_Velocity_Value;
    p.Color = float3(0.6f, 0.8f, 1.0f);
}



#define THREAD_COUNT 64
[numthreads(THREAD_COUNT, 1, 1)]
void EmitCS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;

    if (index >= Max_Particle_N)
        return;

    InterlockedAdd(debug_buffer[0], 1); // 호출 카운트

    Particle_Info p = ParticleBuffer_Emit[index];
    if (p.Active == 1)
        return;

    InterlockedAdd(debug_buffer[1], 1); // emit 카운트

    p.Active = 1;
    p.Lifetime = 0.0f;

    if (p.Type == 0)
        Emit_Snow(p, index);
    else if (p.Type == 1)
        Emit_Spark(p, index);
    else if (p.Type == 2)
    {
        float seed = frac(sin(index * 91.91f) * 12345.6789f);
        float startDelay = seed * 0.1f;

        if (ElapsedTime >= startDelay)
        {
            p.Active = 1;
            p.Lifetime = 0.0f;
            Emit_Water_Splash(p, index);
        }
        else
        {
            return; 
        }
    }

        ParticleBuffer_Emit[index] = p;
}