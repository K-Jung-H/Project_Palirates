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
    float4 Velocity_and_Rotate;
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

void Update_Snow(inout Particle_Info p, uint index)
{
    float spinSpeed = 2.5f;
    p.Velocity += p.Acceleration * ElapsedTime;
    p.Velocity += RandomSpreadDirection(index, Main_Direction, 2.0f);
    p.Position += p.Velocity * ElapsedTime;

    p.Rotate_Value += spinSpeed * ElapsedTime;
}

void Update_Spark(inout Particle_Info p, uint index)
{
    float spinSpeed = 8.0f;
    p.Velocity += p.Acceleration * ElapsedTime;
    p.Velocity += RandomSpreadDirection(index, Main_Direction, 1.0f);
    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += spinSpeed * ElapsedTime;
}

void Extract_Instance(in Particle_Info p)
{
    Render_Instance inst;
    inst.Position = p.Position;
    inst.Velocity_and_Rotate = float4(p.Velocity, p.Rotate_Value);

    float normalizedLife = saturate(p.Lifetime / p.MaxLifetime); // 0.0 ~ 1.0
    float alpha = 1.0f - normalizedLife; 

    inst.Color = float4(p.Color, alpha); 

    InterlockedAdd(debug_buffer[3], 1); 
    RenderInstanceBuffer.Append(inst);
}

[numthreads(1, 1, 1)]
void Update_Spread_CS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= Max_Particle_N)
        return;

    Particle_Info p = ParticleBuffer_Update[index];
    if (p.Active == 0)
        return;

    p.Lifetime += ElapsedTime;

    bool out_of_bounds =
        p.Position.x < EmitRegionMin.x || p.Position.x > EmitRegionMax.x ||
        p.Position.y < EmitRegionMin.y || p.Position.y > EmitRegionMax.y ||
        p.Position.z < EmitRegionMin.z || p.Position.z > EmitRegionMax.z;

    if (p.Lifetime >= p.MaxLifetime || out_of_bounds)
    {
        p.Active = 0;
        InterlockedAdd(debug_buffer[2], 1); // 비활성화
    }
    else
    {
        // 타입별 동작 처리
        if (p.Type == 0)
            Update_Snow(p, index);
        else if (p.Type == 1)
            Update_Spark(p, index);

        // 인스턴스 추출
        Extract_Instance(p);
    }

    ParticleBuffer_Update[index] = p;
}
