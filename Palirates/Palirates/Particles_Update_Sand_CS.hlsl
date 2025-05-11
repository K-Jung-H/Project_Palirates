#include "Particles_Update_Extract_CS.hlsl"

//=============================================================

void Update_Sand(inout Particle_Info p, uint index)
{    
    p.Velocity += p.Acceleration * ElapsedTime;
    p.Velocity += RandomSpreadDirection(index, Main_Direction, 2.0f);
    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += 4.5f * ElapsedTime;
}

//=============================================================

void Update_Focus_Sand(inout Particle_Info p, uint index)
{
    float3 startToEnd = focus_point - p.Position;
    float remainTime = max(0.01f, p.MaxLifetime - p.Lifetime);

    float3 newPos = lerp(p.Position, focus_point, ElapsedTime / remainTime);
    p.Velocity = (newPos - p.Position) / ElapsedTime;
    p.Position = newPos;

    p.Rotate_Value += 2.0f * ElapsedTime;

    if (length(focus_point - p.Position) < 1.0f || remainTime <= ElapsedTime)
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
    float3 up = normalize(Main_Direction); // 일반적으로 Y축 방향
    float t = saturate(p.Lifetime / p.MaxLifetime); // 시간 비율
    float seed = frac(sin(index * 91.91f) * 10000.0f);

    // 중심 기준을 focus_point로 변경
    float3 toFocus = focus_point - p.Position;
    float3 tangent = normalize(cross(up, toFocus));

    // 상승 속도 계산 (스파이럴 상승)
    float verticalBase = 1.0f + 3.0f * easeOutExpo(t);
    float verticalNoise = lerp(0.5f, 2.5f, frac(sin(index * 23.23f) * 4567.89f));
    float verticalSpeed = verticalBase * verticalNoise;
    if (t < 0.5f)
        verticalSpeed *= 0.1f;

    // 회전 속도 (스파이럴 회전)
    float rotationBase = 60.0f;
    float rotationNoise = lerp(0.8f, 2.5f, frac(cos(index * 57.57f) * 6789.01f));
    float rotationSpeed = rotationBase * rotationNoise;
    if (t > 0.5f)
        rotationSpeed *= 2.0f;

    // 나선형 가속도 적용
    float3 spiralAccel = tangent * rotationSpeed + up * verticalSpeed;
    p.Velocity += spiralAccel * ElapsedTime;

    // 흔들림 (진동)
    float radialOsc = sin(p.Lifetime * 15.0f + seed * 3.14f) * 2.0f;
    float verticalOsc = sin(p.Lifetime * 12.0f + seed * 6.28f) * 1.5f;

    float3 radial = toFocus - dot(toFocus, up) * up;
    float3 radialDir = (length(radial) > 0.001f) ? normalize(radial) : float3(1, 0, 0);
    float3 shake = radialDir * radialOsc + up * verticalOsc;
    p.Velocity += shake * ElapsedTime;

    // 위치 이동, 회전, 생명 처리
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

    if (DelayActive(p))
    {
        ParticleBuffer_Update[index] = p;
        return;
    }

    if (p.Type != PARTICLE_TYPE_SAND)
    {
        p.Type = PARTICLE_TYPE_SAND;
        p.Active = 0;
        ParticleBuffer_Update[index] = p;
        return;
    }

    if (p.Active == 0)
        return;

    p.Lifetime += ElapsedTime;

    if (p.Lifetime >= p.MaxLifetime)
    {
        p.Active = 0;
        ParticleBuffer_Update[index] = p;
        return;
    }

    p.EmitFaceIndex = 5;
    p.Acceleration = float3(0.0f, -9.8f, 0.0f);
    Update_Sand(p, index);
    

    float3 worldPos = mul(float4(p.Position, 1.0f), gWorldMatrix).xyz;
    if (CheckCollisionWithGridOBBs(worldPos))
    {
        p.Color = float3(1.0f, 0.5f, 0.0f);
        p.Lifetime = p.MaxLifetime;
        p.Active = 0;
        ParticleBuffer_Update[index] = p;
        return;
    }
    
    if (worldPos.y <= 10.0f)
    {
        p.Velocity = float3(0.0f, 0.0f, 0.0f);
        p.Lifetime = 0.5f;
        p.MaxLifetime = 10.0f;
    }

    ParticleBuffer_Update[index] = p;
    Extract_Instance(p);
}

//=============================================================

[numthreads(64, 1, 1)]
void Sand_Gathering_CS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    if (index >= Max_Particle_N)
        return;

    Particle_Info p = ParticleBuffer_Update[index];

    if (DelayActive(p))
    {
        ParticleBuffer_Update[index] = p;
        return;
    }

    if (p.Active == 0)
        return;

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
        ParticleBuffer_Update[index] = p;
        return;
    }

    p.Type = PARTICLE_TYPE_SAND;
    p.EmitFaceIndex = 5;
    p.Acceleration = float3(0.0f, -9.8f, 0.0f);

    Update_Focus_Sand(p, index);
    
    float3 worldPos = mul(float4(p.Position, 1.0f), gWorldMatrix).xyz;
    if (CheckCollisionWithGridOBBs(worldPos))
    {
        p.Color = float3(1.0f, 0.5f, 0.0f);
        p.Lifetime = p.MaxLifetime;
        p.Active = 0;
        ParticleBuffer_Update[index] = p;
        return;
    }
    
    Extract_Instance(p);

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

    if (DelayActive(p))
    {
        ParticleBuffer_Update[index] = p;
        return;
    }

    if (p.Active == 0)
        return;

    p.Lifetime += ElapsedTime;

    if (p.Type == PARTICLE_TYPE_SAND)
    {
        float lifeRatio = p.Lifetime / max(p.MaxLifetime, 0.001f);
        float distToFocus = length(focus_point - p.Position);

        if (lifeRatio < 0.6f) 
        {
            p.Type = PARTICLE_TYPE_SAND_STORM;
            p.Active = 0;
            ParticleBuffer_Update[index] = p;
            return;
        }
        else
        {
            Update_Focus_Sand(p, index);
            Extract_Instance(p);

            if (p.Active == 0) 
            {
                p.Type = PARTICLE_TYPE_SAND_STORM;
                p.Active = 0;
                ParticleBuffer_Update[index] = p;
                return;
            }
        }
    }
    else if (p.Type == PARTICLE_TYPE_SAND_STORM)
    {
        if (p.Lifetime >= p.MaxLifetime)
        {
            p.Active = 0;
        }
        else
        {
            Update_Sand_Storm(p, index);
            
            float3 worldPos = mul(float4(p.Position, 1.0f), gWorldMatrix).xyz;
            if (CheckCollisionWithGridOBBs(worldPos))
            {
                p.Color = float3(1.0f, 0.5f, 0.0f);
                p.Lifetime = p.MaxLifetime;
                p.Active = 0;
                ParticleBuffer_Update[index] = p;
                return;
            }
            
            Extract_Instance(p);
        }
    }

    ParticleBuffer_Update[index] = p;
}