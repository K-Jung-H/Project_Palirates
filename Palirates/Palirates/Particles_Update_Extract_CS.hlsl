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
    
    float3 Main_Direction;
    float pad3;
}

RWStructuredBuffer<Particle_Info> ParticleBuffer_Update : register(u0);
AppendStructuredBuffer<Render_Instance> RenderInstanceBuffer : register(u1);
RWStructuredBuffer<uint> debug_buffer : register(u2);



float3 RandomSpreadDirection(uint id, float3 baseDir, float spreadAmount)
{
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

[numthreads(1, 1, 1)]
void Update_Extract_CS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;

    if (index >= Max_Particle_N)
        return;

    Particle_Info particle = ParticleBuffer_Update[index];

    if (particle.Active == 1)
    {
        particle.Lifetime += ElapsedTime;
        
        bool out_of_bounds =
        particle.Position.x < EmitRegionMin.x || particle.Position.x > EmitRegionMax.x ||
        particle.Position.y < EmitRegionMin.y || particle.Position.y > EmitRegionMax.y ||
        particle.Position.z < EmitRegionMin.z || particle.Position.z > EmitRegionMax.z;

        if (particle.Lifetime >= particle.MaxLifetime || out_of_bounds)
        {
            particle.Active = 0;
            // 디버그 용: 비활성화 입자 개수 기록
            InterlockedAdd(debug_buffer[2], 1);
        }
        else
        {
            particle.Velocity += particle.Acceleration * ElapsedTime;
            particle.Position += particle.Velocity * ElapsedTime;
            particle.Velocity += RandomSpreadDirection(index, particle.Velocity, 1.0f);
            
            Render_Instance instance;
            instance.Position = particle.Position;
            instance.Velocity = particle.Velocity;
            instance.Color = float4(particle.Color, 1.0f);

            
            // 디버그 용: 활성화 입자 개수 기록
            InterlockedAdd(debug_buffer[3], 1);
            RenderInstanceBuffer.Append(instance);
        }

        ParticleBuffer_Update[index] = particle;
    }
}
