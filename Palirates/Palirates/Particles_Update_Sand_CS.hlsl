#include "Particles_Update_Extract_CS.hlsl"

//=============================================================

void Update_Sand(inout Particle_Info p, uint index)
{
    p.Type = 3;
    p.Velocity += p.Acceleration * ElapsedTime;
    p.Velocity += RandomSpreadDirection(index, Main_Direction, 2.0f);
    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += 4.5f * ElapsedTime;
}

//=============================================================

void Update_Focus_Sand(inout Particle_Info p, uint index)
{
    p.Type = 3;
    float3 realMin = min(EmitRegionMin, EmitRegionMax);
    float3 realMax = max(EmitRegionMin, EmitRegionMax);
    float3 center = (realMin + realMax) * 0.5f;

    float3 toCenter = normalize(center - p.Position);
    float speed = length(p.Velocity);
    speed += abs(p.Acceleration.y) * ElapsedTime;

    p.Velocity = toCenter * speed;
    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += 2.0f * ElapsedTime;

    float dist = length(center - p.Position);
    if (dist < 5.0f)
    {
        p.Lifetime = p.MaxLifetime;
        p.Active = 0;
    }
}

//=============================================================

float easeOutExpo(float t)
{
    return (t >= 1.0f) ? 1.0f : 1.0f - pow(2.0f, -10.0f * t);
}

void Update_Sand_Storm(inout Particle_Info p, uint index)
{
    float3 center = GetEmitFaceCenter(p.EmitFaceIndex, EmitRegionMin, EmitRegionMax);
    float3 up = normalize(Main_Direction);
    float t = saturate(p.Lifetime / p.MaxLifetime);
    float seed = frac(sin(index * 91.91f) * 10000.0f);

    float3 toCenter = center - p.Position;
    float3 tangent = normalize(cross(up, toCenter));

    float verticalBase = 1.0f + 3.0f * easeOutExpo(t);
    float verticalNoise = lerp(0.5f, 2.5f, frac(sin(index * 23.23f) * 4567.89f));
    float verticalSpeed = verticalBase * verticalNoise;
    if (t < 0.5f)
        verticalSpeed *= 0.1f;

    float rotationBase = 60.0f;
    float rotationNoise = lerp(0.8f, 2.5f, frac(cos(index * 57.57f) * 6789.01f));
    float rotationSpeed = rotationBase * rotationNoise;
    if (t > 0.5f)
        rotationSpeed *= 2.0f;

    float3 spiralAccel = tangent * rotationSpeed + up * verticalSpeed;
    p.Velocity += spiralAccel * ElapsedTime;

    float radialOsc = sin(p.Lifetime * 15.0f + seed * 3.14f) * 2.0f;
    float verticalOsc = sin(p.Lifetime * 12.0f + seed * 6.28f) * 1.5f;

    float3 radial = toCenter - dot(toCenter, up) * up;
    float3 radialDir = (length(radial) > 0.001f) ? normalize(radial) : float3(1, 0, 0);
    float3 shake = radialDir * radialOsc + up * verticalOsc;
    p.Velocity += shake * ElapsedTime;

    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += (4.0f + seed * 3.0f) * ElapsedTime;
    p.Lifetime += ElapsedTime;

    if (p.Lifetime >= p.MaxLifetime)
        p.Active = 0;
}

//=============================================================


[numthreads(64, 1, 1)]
void Sand_Spread_CS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= Max_Particle_N)
        return;

    Particle_Info p = ParticleBuffer_Update[index];
    if (p.Active == 0)
        return;

    p.Lifetime += ElapsedTime;
    if (p.Lifetime >= p.MaxLifetime)
    {
        p.Active = 0;
    }
    else
    {
        p.Type = PARTICLE_TYPE_SAND;
        p.EmitFaceIndex = 5;
        p.Acceleration = float3(0.0f, -9.8f, 0.0f);
        
        Update_Sand(p, index);
        Extract_Instance(p);
    }

    ParticleBuffer_Update[index] = p;
}

//=============================================================

[numthreads(64, 1, 1)]
void Sand_Gathering_CS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= Max_Particle_N)
        return;

    Particle_Info p = ParticleBuffer_Update[index];
    if (p.Active == 0)
        return;

    p.Lifetime += ElapsedTime;
    if (p.Lifetime >= p.MaxLifetime)
    {
        p.Active = 0;
    }
    else
    {
        p.Type = PARTICLE_TYPE_SAND;
        p.EmitFaceIndex = 5;
        p.Acceleration = float3(0.0f, -9.8f, 0.0f);
        
        Update_Focus_Sand(p, index);
        Extract_Instance(p);
    }

    ParticleBuffer_Update[index] = p;
}

//=============================================================

[numthreads(64, 1, 1)]
void Sand_Storm_CS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= Max_Particle_N)
        return;

    Particle_Info p = ParticleBuffer_Update[index];
    if (p.Active == 0)
        return;

    if (p.Lifetime < 0.0f)
    {
        p.Lifetime += ElapsedTime;
        ParticleBuffer_Update[index] = p;
        return;
    }
    
    
    if (p.Type == PARTICLE_TYPE_SAND)
    {
            p.Type = PARTICLE_TYPE_SAND_STORM;
            p.EmitFaceIndex = 2;
            p.Acceleration = float3(0.0f, 0.0f, 0.0f);
        ParticleBuffer_Update[index] = p;
        return;
    }
    
        p.Lifetime += ElapsedTime;
    if (p.Lifetime >= p.MaxLifetime)
    {
        p.Active = 0;
    }
    else
    {
        Update_Sand_Storm(p, index);
        Extract_Instance(p);
    }

    ParticleBuffer_Update[index] = p;
}
