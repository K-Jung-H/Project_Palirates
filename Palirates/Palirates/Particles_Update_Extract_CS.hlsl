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
    uint FreeList_Size;
    uint Max_Particle;
    float ElapsedTime;
}


RWStructuredBuffer<Particle_Info> ParticleBuffer_Update : register(u0);
AppendStructuredBuffer<uint> FreeList_Update : register(u1);
AppendStructuredBuffer<Render_Instance> RenderInstanceBuffer : register(u2);

[numthreads(64, 1, 1)]
void Update_Extract_CS(uint3 DTid : SV_DispatchThreadID)
{
        
    uint index = DTid.x;
    if (index >= Max_Particle)
        return; // 안전성

    Particle_Info particle = ParticleBuffer_Update[index];

    if (particle.Active == 0)
        return;
    

    // 생명 시간 증가
    particle.Lifetime += ElapsedTime;

    if (particle.Lifetime >= particle.MaxLifetime)
    {
        // 죽은 입자 처리
        particle.Active = 0;
        FreeList_Update.Append(index);
    }
    else
    {
        // 물리 시뮬레이션
        particle.Velocity += particle.Acceleration * ElapsedTime;
        particle.Position += particle.Velocity * ElapsedTime;

        // 렌더링용 인스턴스 생성
        Render_Instance instance;
        instance.Position = particle.Position;
        instance.Velocity = particle.Velocity;
        instance.Color = float4(particle.Color, 1.0f); // 알파는 기본 1

        // 렌더링 리스트에 추가
        RenderInstanceBuffer.Append(instance);
    }

    // 입자 상태 업데이트
    ParticleBuffer_Update[index] = particle;
}