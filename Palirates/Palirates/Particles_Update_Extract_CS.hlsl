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

#define PARTICLE_TYPE_SNOW       0
#define PARTICLE_TYPE_SPARK      1
#define PARTICLE_TYPE_SPLASH     2
#define PARTICLE_TYPE_SAND       3
#define PARTICLE_TYPE_SAND_STORM       4


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

void Update_Sand(inout Particle_Info p, uint index)
{
    p.Velocity += p.Acceleration * ElapsedTime;
    p.Velocity += RandomSpreadDirection(index, Main_Direction, 2.0f);
    p.Position += p.Velocity * ElapsedTime;
    p.Rotate_Value += 4.5f * ElapsedTime;
}

void Update_Focus_Sand(inout Particle_Info p, uint index)
{
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


//===============================================================
// 인스턴싱 정보 추출
void Extract_Instance(in Particle_Info p)
{
    Render_Instance inst;
    inst.Position = p.Position;
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
        else if (p.Type == PARTICLE_TYPE_SAND)
            Update_Focus_Sand(p, index);
        else if (p.Type == PARTICLE_TYPE_SAND_STORM)
            Update_Sand_Storm(p, index);

        Extract_Instance(p);
    }

    ParticleBuffer_Update[index] = p;
}
