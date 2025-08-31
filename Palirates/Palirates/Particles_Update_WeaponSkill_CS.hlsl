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

    if (p.Type != PARTICLE_TYPE_DIFFUSE_Burst)
    {
        p.Type = PARTICLE_TYPE_DIFFUSE_Burst;
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

    
    Update_RadialDiffuse(p, index, true); 
    
    Check_Collisions(p);
    Extract_Instance(p);

    Particle_Info_Buffer[index] = p;
}

//=============================================================

[numthreads(64, 1, 1)]
void Twin_Sword_Skill_State_1_CS(uint3 DTid : SV_DispatchThreadID)
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

    if (p.Type != PARTICLE_TYPE_DIFFUSE_Continuous)
    {
        p.Type = PARTICLE_TYPE_DIFFUSE_Continuous;
        p.Active = 0;
        Particle_Info_Buffer[index] = p;
        return;
    }
    
    if (p.Active == 0)
        return;
    
    bool out_of_bounds = IsOutOfBounds(p.Position, EmitRegionMin, EmitRegionMax);

    
    p.Lifetime += ElapsedTime;



    if (p.Lifetime >= p.MaxLifetime || out_of_bounds)
    {
        p.Active = 0;
        Particle_Info_Buffer[index] = p;
        return;
    }
    
    Update_RadialDiffuse(p, index, false);
    
    Check_Collisions(p);
    
    
    Particle_Info p_render = p;
    p_render.Color = RadialDiffuse_ColorGradient(p);
    Extract_Instance(p_render);
    
    
    Particle_Info_Buffer[index] = p;
}
