#include "Particles_Emit_CS.hlsl"

//===============================================================

float3 RotateVectorByQuaternion(float3 v, float4 q)
{
    return v + 2.0f * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

static bool CheckOBBCollision(float3 p_point, OBB_INFO obb)
{
    if (obb.Active == 0)
        return false;

    float3 delta = p_point - obb.Center;
    float4 invRot = float4(-obb.Rotation.xyz, obb.Rotation.w);
    float3 localPos = RotateVectorByQuaternion(delta, invRot);

    return all(abs(localPos) <= obb.Extents);
}

static bool CheckCollisionWithGridOBBs(float3 pos)
{
    int3 cell = int3(floor((pos - worldMin) / cellSize));
    if (any(cell < 0) || any(cell >= gridDim))
        return false;

    uint flatIndex = cell.x + cell.y * gridDim.x + cell.z * gridDim.x * gridDim.y;
    CellInfo info = g_CellInfos[flatIndex];

    [loop]
    for (uint i = 0; i < info.count; ++i)
    {
        uint obbIdx = g_OBBIndices[info.startIndex + i];
        if (obbIdx >= obb_num)
            continue;

        OBB_INFO obb = OBB_List[obbIdx];
        if (CheckOBBCollision(pos, obb))
        {
            return true;
        }
    }

    return false;
}

float3 ComputeClosestOBBNormal(float3 pos, OBB_INFO obb)
{
    float3 delta = pos - obb.Center;
    float4 invRot = float4(-obb.Rotation.xyz, obb.Rotation.w);
    float3 local = RotateVectorByQuaternion(delta, invRot); // world → local

    float3 normalLocal = float3(0, 0, 0);
    float minDist = FLT_MAX;

    for (int axis = 0; axis < 3; ++axis)
    {
        float dist = abs(abs(local[axis]) - obb.Extents[axis]);
        if (dist < minDist)
        {
            minDist = dist;

            if (axis == 0)
                normalLocal = float3(sign(local.x), 0, 0);
            else if (axis == 1)
                normalLocal = float3(0, sign(local.y), 0);
            else
                normalLocal = float3(0, 0, sign(local.z));
        }
    }

    float3 normalWorld = RotateVectorByQuaternion(normalLocal, obb.Rotation); // local → world
    return normalize(normalWorld);
}

static bool CheckCollisionWithGridOBBs_WithNormal(float3 pos, out float3 outNormal)
{
    outNormal = float3(0, 1, 0); // fallback normal (e.g. ground)

    int3 cell = int3(floor((pos - worldMin) / cellSize));
    if (any(cell < 0) || any(cell >= gridDim))
        return false;

    uint flatIndex = cell.x + cell.y * gridDim.x + cell.z * gridDim.x * gridDim.y;
    CellInfo info = g_CellInfos[flatIndex];

    [loop]
    for (uint i = 0; i < info.count; ++i)
    {
        uint obbIdx = g_OBBIndices[info.startIndex + i];
        if (obbIdx >= obb_num)
            continue;

        OBB_INFO obb = OBB_List[obbIdx];
        if (CheckOBBCollision(pos, obb))
        {
            outNormal = ComputeClosestOBBNormal(pos, obb);
            return true;
        }
    }

    return false;
}


bool Check_Collision_OBB(inout Particle_Info p, float3 world_pos)
{
    switch (p.Type)
    {
        //=================================
        // need reflection angle
        case PARTICLE_TYPE_DRAGON_FIRE:
            {
                float3 normal;
                if (CheckCollisionWithGridOBBs_WithNormal(world_pos, normal))
                {
                    p.Velocity = reflect(p.Velocity, normalize(normal));
                    p.Color = float3(1.0f, 0.3f, 0.0f);
                }

            }
            break;
        
        
        //=================================        
        // not need reflection angle
        case PARTICLE_TYPE_SNOW:
            if (CheckCollisionWithGridOBBs(world_pos))
            {
                p.Velocity = float3(0, -1, 0);
                p.Lifetime += 0.01f;

            }
            break;
        
        case PARTICLE_TYPE_SPLASH:
            if (CheckCollisionWithGridOBBs(world_pos))
            {
                p.Velocity *= 0.2f;
                p.Color = float3(0.2f, 0.2f, 1.0f);
            }
            break;
        
        case PARTICLE_TYPE_PARTY:
            if (CheckCollisionWithGridOBBs(world_pos))
            {
                p.Velocity = float3(0, -1, 0);
                if (p.Lifetime < p.MaxLifetime - 5.0f)
                    p.Lifetime = p.MaxLifetime - 5.0f;
            }
            break;
        
        case PARTICLE_TYPE_DIFFUSE_Continuous:
        case PARTICLE_TYPE_DIFFUSE_Burst:
            if (CheckCollisionWithGridOBBs(world_pos))
            {
                p.Velocity = float3(0, -1, 0);
                if (p.Lifetime < p.MaxLifetime - 5.0f)
                    p.Lifetime = p.MaxLifetime - 5.0f;
            }
            break;
        
        case PARTICLE_TYPE_INTERVAL_BLEEDING:
        {
                if (CheckCollisionWithGridOBBs(world_pos))
                {
                    p.Acceleration = float3(0.0f, 0.0f, 0.0f);
                    p.Velocity = float3(0.0f, -0.1f, 0.0f);
                }
            }
            break;
        default:
            if (CheckCollisionWithGridOBBs(world_pos))
            {
                p.Active = 0;
            }
            break;
    }
    return false;
}

bool Check_Collision_Ground(inout Particle_Info p, float3 world_pos)
{
    if (world_pos.y > 3.0f)
        return false;

    switch (p.Type)
    {
        case PARTICLE_TYPE_DRAGON_FIRE:
        {
                float3 normal = float3(0, 1, 0);
                float speed = length(p.Velocity);
                float3 incident = normalize(p.Velocity);

                p.Velocity = reflect(incident, normal) * speed;
                p.Position.y += 3.5f;
                p.Color = float3(1.0f, 0.3f, 0.0f);
            }
            break;

        case PARTICLE_TYPE_SNOW:
        {
                p.Velocity = float3(0, 0, 0);
                p.Lifetime += 0.01f;
            }
            break;

        case PARTICLE_TYPE_SPLASH:
        {

            }
            break;

        case PARTICLE_TYPE_PARTY:
        {
                p.Velocity = float3(0,  0.0001f, 0);
                if (p.Lifetime < p.MaxLifetime - 5.0f)
                    p.Lifetime = p.MaxLifetime - 5.0f;
            }
            break;
        case PARTICLE_TYPE_INTERVAL_BLEEDING:
        {
                if (world_pos.y < 1.0f)
                {
                    p.Velocity = float3(0.0f, 0.0f, 0.0f);
                    p.Acceleration = float3(0.0f, 0.0f, 0.0f);
                }
            }
            break; 
        case PARTICLE_TYPE_HEAL:
        {
                p.Position.y += 0.5f;
            }
            break;
        
        case PARTICLE_TYPE_DIFFUSE_Continuous:
        case PARTICLE_TYPE_DIFFUSE_Burst:
        {
                if (world_pos.y < 1.0f)
                {
                    p.Position.y += 0.5f;
                    p.Velocity.y = abs(p.Velocity.y);
                }
            }
            break;
        
        default:
        {
                p.Active = 0;
            }
            break;
    }

    return true;
}


void Check_Collisions(inout Particle_Info p)
{
    float3 localPos = p.Position;
    float3 worldPos = mul(float4(localPos, 1.0f), gWorldMatrix).xyz;

    if (Check_Collision_Ground(p, worldPos))
        return;
    
    Check_Collision_OBB(p, worldPos);
}
//===============================================================

bool IsOutOfBounds(float3 pos, float3 minBound, float3 maxBound)
{
    return pos.x < minBound.x || pos.x > maxBound.x ||
           pos.y < minBound.y || pos.y > maxBound.y ||
           pos.z < minBound.z || pos.z > maxBound.z;
}

static bool DelayActive(inout Particle_Info p)
{
    if (p.Lifetime < 0.0f)
    {
        p.Lifetime += ElapsedTime;

        if (p.Lifetime >= 0.0f)
            p.Active = 1;

        return true; // 아직 대기 중
    }

    return false; // 이미 활성화됨
}


//===============================================================
// 파티클 동작별 업데이트
void Update_Snow(inout Particle_Info p, uint index)
{
    float3 new_direction = RandomSpreadDirection(index, Main_Direction, 2.0f);
    float speed = length(p.Velocity + p.Acceleration);
    
    p.Velocity = normalize(new_direction) * speed;
    
    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += 1.5f * ElapsedTime;
}


void Update_Water_Splash(inout Particle_Info p, uint index)
{
    p.Velocity += p.Acceleration * ElapsedTime;
    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += 4.0f * ElapsedTime;
}


float3 FireColorGradient(float lifeRatio, float outerT)
{
    float3 baseColor;

    if (lifeRatio < 0.1f)
    {
        baseColor = float3(1.0f, 1.0f, 1.0f); // initial white
    }
    else if (lifeRatio < 0.5f)
    {
        float t = saturate((lifeRatio - 0.2f) / 0.5f);
        float3 centerColor = float3(1.0f, 1.0f, 0.0f); // yellow
        float3 outerColor = float3(1.0f, 0.1f, 0.0f); // red
        baseColor = lerp(centerColor, outerColor, pow(outerT, 1.5f));
        baseColor = lerp(centerColor, baseColor, pow(t, 1.2f));
    }
    else
    {
        float t = saturate((lifeRatio - 0.7f) / 0.3f);
        float3 prevColor = float3(1.0f, 0.1f, 0.0f);
        float3 finalColor = float3(1.0f, 0.4f, 0.1f); // orange
        baseColor = lerp(prevColor, finalColor, pow(t, 1.5f));
    }

    return baseColor;
}

void Update_DragonFire(inout Particle_Info p, uint index)
{
    p.Acceleration = float3(0.0f, 0.8f, 0.0f);
    p.Velocity += p.Acceleration * ElapsedTime;

    float3 noise = float3(
        frac(sin(index * 13.13f + ElapsedTime) * 43758.5453f) - 0.5f,
        frac(sin(index * 27.27f + ElapsedTime * 0.5f) * 12345.6789f) - 0.5f,
        frac(sin(index * 39.39f + ElapsedTime * 1.5f) * 98765.4321f) - 0.5f
    );
    p.Velocity += noise * 0.2f;
    p.Position += p.Velocity * ElapsedTime;

    float lifeRatio = saturate(p.Lifetime / p.MaxLifetime);
    p.Size = lerp(0.1f, 1.0f, pow(lifeRatio, 1.2f)); // size grows over time

    float3 dir = normalize(p.Velocity);
    float angle = acos(dot(dir, normalize(Main_Direction)));
    float coneAngle = radians(20.0f);
    float outerT = saturate(angle / coneAngle); // 0 = center, 1 = edge

    p.Color = FireColorGradient(lifeRatio, outerT);
}

void Update_Party(inout Particle_Info p, uint index)
{
    
    p.Velocity += p.Acceleration * ElapsedTime;
    p.Velocity += RandomSpreadDirection(index, p.Acceleration, 2.0f);
    p.Position += p.Velocity * ElapsedTime;
    
    p.Rotate_Value += 2.5f * ElapsedTime;
    
}

void Update_Heal(inout Particle_Info p, uint index)
{
    p.Position += p.Velocity * ElapsedTime;    
    p.Rotate_Value += 2.5f * ElapsedTime;
}

void Update_Orbit(inout Particle_Info p, uint index)
{
    float angleNorm = p.Velocity.x;
    float angularSpeed = p.Velocity.y;
    float3 orbitNormal = p.Acceleration;

    angleNorm += (angularSpeed * ElapsedTime) / 6.2831853f;
    angleNorm = fmod(angleNorm, 1.0f);
    if (angleNorm < 0.0f)
        angleNorm += 1.0f; 
    p.Velocity.x = angleNorm;

    float theta = angleNorm * 6.2831853f;

    float3 up = orbitNormal;
    float3 ref = (abs(up.y) > 0.99f) ? float3(1, 0, 0) : float3(0, 1, 0);
    float3 right = normalize(cross(up, ref));
    float3 forward = normalize(cross(up, right));

    float3 localPos = focus_strength * (cos(theta) * right + sin(theta) * forward);
    p.Position = focus_point + localPos;
}


float3 RadialDiffuse_ColorGradient(inout Particle_Info p)
{
    float3 main_color = p.Color;
    float t = saturate(p.Lifetime / p.MaxLifetime);
    // t = smoothstep(0.0, 1.0, t);          // 또는: t = t * t;

    return lerp(main_color, float3(1.0, 1.0, 1.0), t);
}

void Update_RadialDiffuse(inout Particle_Info p, uint index, bool add_acceleration)
{
    if (add_acceleration)
    {
        p.Velocity += p.Acceleration * ElapsedTime;
    }

    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += 4.0f * ElapsedTime;
    
    
    float lifeRatio = saturate(p.Lifetime / p.MaxLifetime);
    
}

//===============================================================
// 인스턴싱 정보 추출

void Extract_Instance(in Particle_Info p)
{
    Render_Instance inst;

    float safeScale = max(p.Size, 0.1f); // clamp to minimum positive scale
    inst.Position_and_Scale = float4(p.Position.xyz, safeScale);
    inst.Velocity_and_Rotate = float4(p.Velocity, p.Rotate_Value);

    float alpha = 1.0f;

    if(p.Type == PARTICLE_TYPE_PARTY)
    {
        inst.Color = float4(p.Color, alpha);
    }
    else // fade
    {
        float normalizedLife = saturate(p.Lifetime / p.MaxLifetime);
        alpha = 1.0f - normalizedLife; 
        inst.Color = float4(p.Color, alpha);
    }

    InterlockedAdd(debug_buffer[3], 1);
    RenderInstanceBuffer.Append(inst);
    
}

//===============================================================
// 메인 Compute Shader

#define THREAD_COUNT 64
[numthreads(THREAD_COUNT, 1, 1)]
void Update_Continuous_CS(uint3 DTid : SV_DispatchThreadID)
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

    bool out_of_bounds = IsOutOfBounds(p.Position, EmitRegionMin, EmitRegionMax);


    if (p.Lifetime >= p.MaxLifetime || out_of_bounds)
    {
        p.Active = 0;
        InterlockedAdd(debug_buffer[2], 1);
    }
    else
    {
        if (p.Type == PARTICLE_TYPE_SNOW)
            Update_Snow(p, index);
        else if (p.Type == PARTICLE_TYPE_SPLASH)
            Update_Water_Splash(p, index);
        else if (p.Type == PARTICLE_TYPE_DRAGON_FIRE)
            Update_DragonFire(p, index);
        else if (p.Type == PARTICLE_TYPE_PARTY)
            Update_Party(p, index);
        else if (p.Type == PARTICLE_TYPE_HEAL)
            Update_Heal(p, index);

        Check_Collisions(p);
        Extract_Instance(p);
    }

    Particle_Info_Buffer[index] = p;
}


//===============================================================

void Update_Bleeding(inout Particle_Info p, uint index)
{
   // p.Velocity += RandomSpreadDirection(index, Main_Direction, 1.0f);
    p.Velocity += p.Acceleration * ElapsedTime;
    p.Acceleration.y += -9.8f;
    p.Position += p.Velocity * ElapsedTime;

    // 위치 갱신
    p.Rotate_Value += 1.0f * ElapsedTime;
}


#define THREAD_COUNT 64
[numthreads(THREAD_COUNT, 1, 1)]
void Update_Interval_CS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    
    if (index >= Max_Particle_N)
        return;

    Particle_Info p = Particle_Info_Buffer[index];

    // Check Reset Flag
    if (Reset_Flag != 0)
    {
        p.Active = 0;
        p.Sleep = 0;
        Particle_Info_Buffer[index] = p;
        return;
    }
    
    if (p.Sleep == 1)
    {
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

    bool out_of_bounds = IsOutOfBounds(p.Position, EmitRegionMin, EmitRegionMax);


    if (p.Lifetime >= p.MaxLifetime || out_of_bounds)
    {
        p.Active = 0;
        p.Sleep = 1;
        InterlockedAdd(debug_buffer[2], 1);
    }
    else
    {
        if (p.Type == PARTICLE_TYPE_INTERVAL_BLEEDING)
            Update_Bleeding(p, index);
    
        
        float3 localPos = p.Position;
        float3 worldPos = mul(float4(localPos, 1.0f), gWorldMatrix).xyz;


        Check_Collisions(p);
        Extract_Instance(p);
    }

    Particle_Info_Buffer[index] = p;
}

