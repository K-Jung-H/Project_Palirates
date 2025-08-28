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

    if (p.Type != PARTICLE_TYPE_ORBIT)
    {
        p.Type = PARTICLE_TYPE_ORBIT;
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

    Update_Orbit(p, index);
    
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
    

    p.Type = p.Type = PARTICLE_TYPE_DIFFUSE;
    p.Acceleration = float3(0.0f, -9.8f, 0.0f);

    Emit_RadialDiffuse(p, index); // 원형으로 파장 생성
    
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