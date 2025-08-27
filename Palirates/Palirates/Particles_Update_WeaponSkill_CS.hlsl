#include "Particles_Update_Extract_CS.hlsl"


[numthreads(64, 1, 1)]
void Spear_Skill_State_1_CS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= Max_Particle_N)
        return;

    Particle_Info p = Particle_Info_Buffer[index];

    // Check Reset Flag
    if (Reset_Flag != 0)
    {
        p.Active = 0;
        Particle_Info_Buffer[index] = p;
        return;
    }

    // Check Delay    
    bool isDelayed = DelayActive(p);
    if (isDelayed)
    {
        Particle_Info_Buffer[index] = p;
        return;
    }

    if (p.Type != PARTICLE_TYPE_SAND)
    {
        p.Type = PARTICLE_TYPE_SAND;
        p.Active = 0;
        Particle_Info_Buffer[index] = p;
        return;
    }

    if (p.Active == 0)
        return;

    
    p.Lifetime += ElapsedTime;

    if (p.Lifetime >= p.MaxLifetime)
    {
        p.Active = 0;
        Particle_Info_Buffer[index] = p;
        return;
    }

    p.EmitFaceIndex = 5;
    p.Acceleration = float3(0.0f, -9.8f, 0.0f);
    Update_Orbit(p, index);
    

    float3 worldPos = mul(float4(p.Position, 1.0f), gWorldMatrix).xyz;
    if (CheckCollisionWithGridOBBs(worldPos))
    {
        p.Color = float3(1.0f, 0.5f, 0.0f);
        p.Lifetime = p.MaxLifetime;
        p.Active = 0;
        Particle_Info_Buffer[index] = p;
        return;
    }
    else if (worldPos.y <= 10.0f)
    {
        p.Velocity = float3(0.0f, 0.0f, 0.0f);
        p.Lifetime = 0.5f;
        p.MaxLifetime = 10.0f;
    }

    Particle_Info_Buffer[index] = p;
    Extract_Instance(p);
}

//=============================================================

[numthreads(64, 1, 1)]
void Spear_Skill_State_2_CS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= Max_Particle_N)
        return;

    Particle_Info p = Particle_Info_Buffer[index];

    // Check Reset Flag
    if (Reset_Flag != 0)
    {
        p.Active = 0;
        Particle_Info_Buffer[index] = p;
        return;
    }

    // Check Delay    
    bool isDelayed = DelayActive(p);
    if (isDelayed)
    {
        Particle_Info_Buffer[index] = p;
        return;
    }
    
    p.Lifetime += ElapsedTime;

    float distToFocus = length(focus_point - p.Position);
    bool reached = (distToFocus < 1.0f || p.Lifetime >= p.MaxLifetime);

    if (reached)
    {
        p.Type = PARTICLE_TYPE_SAND_STORM;
        p.Lifetime = -0.1f;
        p.Active = 1;
        p.EmitFaceIndex = 2;
        p.Acceleration = float3(0.0f, 0.0f, 0.0f);
        Particle_Info_Buffer[index] = p;
        return;
    }

    p.Type = PARTICLE_TYPE_SAND;
    p.EmitFaceIndex = 5;
    p.Acceleration = float3(0.0f, -9.8f, 0.0f);

    Update_Spread(p, index); // 원형으로 파장 생성
    
    float3 worldPos = mul(float4(p.Position, 1.0f), gWorldMatrix).xyz;
    if (CheckCollisionWithGridOBBs(worldPos))
    {
        p.Color = float3(1.0f, 0.5f, 0.0f);
        p.Lifetime = p.MaxLifetime;
        p.Active = 0;
        Particle_Info_Buffer[index] = p;
        return;
    }
    
    Extract_Instance(p);

    Particle_Info_Buffer[index] = p;
}

//=============================================================

[numthreads(64, 1, 1)]
void Spear_Skill_State_3_CS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= Max_Particle_N)
        return;

    Particle_Info p = Particle_Info_Buffer[index];

    // Check Reset Flag
    if (Reset_Flag != 0)
    {
        p.Active = 0;
        Particle_Info_Buffer[index] = p;
        return;
    }

    // Check Delay    
    bool isDelayed = DelayActive(p);
    if (isDelayed)
    {
        Particle_Info_Buffer[index] = p;
        return;
    }

    if (p.Active == 0)
        return;
    
    
    p.Lifetime += ElapsedTime;


    if (p.Type == PARTICLE_TYPE_SAND_STORM)
    {
        if (p.Lifetime >= p.MaxLifetime)
        {
            p.Active = 0;
        }
        else
        {
            Update_Blow_up(p, index); // 위로 상승
            
            float3 worldPos = mul(float4(p.Position, 1.0f), gWorldMatrix).xyz;
            if (CheckCollisionWithGridOBBs(worldPos))
            {
                p.Color = float3(1.0f, 0.5f, 0.0f);
                p.Lifetime = p.MaxLifetime;
                p.Active = 0;
                Particle_Info_Buffer[index] = p;
                return;
            }
            
            Extract_Instance(p);
        }
    }

    Particle_Info_Buffer[index] = p;
}