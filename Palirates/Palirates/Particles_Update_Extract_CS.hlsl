struct Particle_Info
{
    float3 Position;
    float Lifetime;

    float3 Velocity;
    float MaxLifetime;

    float3 Acceleration;
    float Rotate_Value;

    float3 Color;
    uint EmitFaceIndex;

    float Size;
    uint Type;
    uint Active;
    float padding0;
};

struct Render_Instance
{
    float4 Position_and_Scale;
    float4 Velocity_and_Rotate;
    float4 Color;
};

struct OBB_INFO
{
    float3 Center;
    uint Active;

    float3 Extents;
    uint Type;

    float4 Rotation;
};

struct CellInfo
{
    uint startIndex; // g_OBBIndices[] start index
    uint count; // OBB num
};

#define PARTICLE_TYPE_SNOW       0
#define PARTICLE_TYPE_SPARK      1
#define PARTICLE_TYPE_SPLASH     2
#define PARTICLE_TYPE_SAND       3
#define PARTICLE_TYPE_SAND_STORM       4
#define PARTICLE_TYPE_DRAGON_FIRE       5

cbuffer CB_Particle_Update_Info : register(b0)
{
    matrix gWorldMatrix;

    float3 EmitRegionMin;
    float ElapsedTime;

    float3 EmitRegionMax;
    uint Max_Particle_N;

    float3 Main_Direction;
    float Init_Velocity_Value;

    float3 focus_point;
    float focus_strength;

    uint obb_num;
    float3 padding0;
};

RWStructuredBuffer<Particle_Info> ParticleBuffer_Update : register(u0);
AppendStructuredBuffer<Render_Instance> RenderInstanceBuffer : register(u1);
RWStructuredBuffer<uint> debug_buffer : register(u2);

StructuredBuffer<OBB_INFO> OBB_List : register(t0);

cbuffer Grid_Info : register(b1)
{
    float3 worldMin;
    float cellSize;
    int3 gridDim;
    float padding;
};


StructuredBuffer<CellInfo> g_CellInfos : register(t1);
StructuredBuffer<uint> g_OBBIndices : register(t2);

//===============================================================

float3 RotateVectorByQuaternion(float3 v, float4 q)
{
    return v + 2.0f * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

bool CheckOBBCollision(float3 p_point, OBB_INFO obb)
{
    if (obb.Active == 0)
        return false;

    float3 delta = p_point - obb.Center;

    // Inverse quaternion (conjugate)
    float4 invRot = float4(-obb.Rotation.xyz, obb.Rotation.w);

    // Apply inverse rotation to point (convert to OBB local space)
    float3 localPos = RotateVectorByQuaternion(delta, invRot);

    // Check AABB bounds in OBB-local space
    return all(abs(localPos) <= obb.Extents);
}

bool CheckCollisionWithGridOBBs(float3 pos)
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


//===============================================================

float3 GetEmitFaceCenter(int face, float3 min, float3 max)
{
    float3 c = (min + max) * 0.5f;

    // FACE_X → 중심에서 해당 축만 min/max
    switch (face)
    {
        case 0:
            return float3(min.x, c.y, c.z); // LEFT (-X)
        case 1:
            return float3(max.x, c.y, c.z); // RIGHT (+X)
        case 2:
            return float3(c.x, min.y, c.z); // BOTTOM (-Y)
        case 3:
            return float3(c.x, max.y, c.z); // TOP (+Y)
        case 4:
            return float3(c.x, c.y, min.z); // BACK (-Z)
        case 5:
            return float3(c.x, c.y, max.z); // FRONT (+Z)
        default:
            return c;
    }
}


// 랜덤 퍼짐 방향 계산
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

    return normalize(baseDir + offset);
}

bool IsOutOfBounds(float3 pos, float3 minBound, float3 maxBound)
{
    return pos.x < minBound.x || pos.x > maxBound.x ||
           pos.y < minBound.y || pos.y > maxBound.y ||
           pos.z < minBound.z || pos.z > maxBound.z;
}

bool DelayActive(inout Particle_Info p)
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
    p.Velocity += p.Acceleration * ElapsedTime;
    p.Velocity += RandomSpreadDirection(index, Main_Direction, 2.0f);
    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += 2.5f * ElapsedTime;
}

void Update_Spark(inout Particle_Info p, uint index)
{
    p.Velocity += p.Acceleration * ElapsedTime;
    p.Velocity += RandomSpreadDirection(index, Main_Direction, 1.0f);
    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += 8.0f * ElapsedTime;
}

void Update_Water_Splash(inout Particle_Info p, uint index)
{
    p.Acceleration = float3(0.0f, -9.8f, 0.0f);
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

//===============================================================
// 인스턴싱 정보 추출

void Extract_Instance(in Particle_Info p)
{
    Render_Instance inst;

    float safeScale = max(p.Size, 0.1f); // clamp to minimum positive scale
    inst.Position_and_Scale = float4(p.Position.xyz, safeScale);

    inst.Velocity_and_Rotate = float4(p.Velocity, p.Rotate_Value);

    float normalizedLife = saturate(p.Lifetime / p.MaxLifetime);
    float alpha = 1.0f - normalizedLife;
    inst.Color = float4(p.Color, alpha);

    InterlockedAdd(debug_buffer[3], 1);
    RenderInstanceBuffer.Append(inst);
    
}

//===============================================================
// 메인 Compute Shader

#define THREAD_COUNT 64
[numthreads(THREAD_COUNT, 1, 1)]
void Update_Spread_CS(uint3 DTid : SV_DispatchThreadID)
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
        else if (p.Type == PARTICLE_TYPE_SPARK)
            Update_Spark(p, index);
        else if (p.Type == PARTICLE_TYPE_SPLASH)
            Update_Water_Splash(p, index);
        else if (p.Type == PARTICLE_TYPE_DRAGON_FIRE)
            Update_DragonFire(p, index);
        
        
        float3 localPos = p.Position;
        float3 worldPos = mul(float4(localPos, 1.0f), gWorldMatrix).xyz;

        if (CheckCollisionWithGridOBBs(worldPos))
        {
            p.Velocity = float3(0.0f, 0.0f, 0.0f);
            p.Acceleration = float3(0.0f, 0.0f, 0.0f);
            p.Color = float3(0.0f, 0.0f, 1.0f);
            ParticleBuffer_Update[index] = p;
            return;
        }
    
        Extract_Instance(p);
    }

    ParticleBuffer_Update[index] = p;
}
