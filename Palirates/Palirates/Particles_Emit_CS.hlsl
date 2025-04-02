
cbuffer CB_Particle_Update_Info : register(b0)
{
    uint FreeList_Size;    
    uint Max_Particle;
    float ElapsedTime;
}

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

RWStructuredBuffer<Particle_Info> ParticleBuffer_Emit : register(u0);
ConsumeStructuredBuffer<uint> FreeList_Emit : register(u1);

[numthreads(1, 1, 1)]
void EmitCS(uint3 DTid : SV_DispatchThreadID)
{
    uint tid = DTid.x;
    if (tid >= FreeList_Size)
        return;
    
    uint index = FreeList_Emit.Consume();

    Particle_Info p = (Particle_Info) 0;
    p.Position = float3(0.0f, 0.0f, 0.0f);
    p.Velocity = float3(0.0f, 3.0f, 0.0f);
    p.Acceleration = float3(0.0f, -9.8f, 0.0f);
    p.Lifetime = 0.0f;
    p.MaxLifetime = 1.0f;
    p.Color = float3(1.0f, 1.0f, 0.0f);
    p.Size = float2(10.0f, 10.0f);
    p.Type = 0;
    p.Active = 1;

    ParticleBuffer_Emit[index] = p;

}