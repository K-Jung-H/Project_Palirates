
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

cbuffer CB_Particle_Update_Info : register(b0)
{
    float3 EmitRegionMin; 
    float ElapsedTime; 

    float3 EmitRegionMax; 
    uint Max_Particle_N;

    float3 Main_Direction; 
    float Init_Velocity_Value;
}

RWStructuredBuffer<Particle_Info> ParticleBuffer_Emit : register(u0);
AppendStructuredBuffer<Render_Instance> RenderInstanceBuffer : register(u1);
RWStructuredBuffer<uint> debug_buffer : register(u2);

#define XM_PI 3.14159265359f

#define FACE_LEFT    0 // -X
#define FACE_RIGHT   1 // +X
#define FACE_BOTTOM  2 // -Y
#define FACE_TOP     3 // +Y
#define FACE_BACK    4 // -Z
#define FACE_FRONT   5 // +Z

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

//===============================================================
// 면 중심에서 약간 퍼지도록 emit 위치 생성
float3 RandomEmitPosition(uint id, float3 min, float3 max, int face)
{
    float3 center = GetEmitFaceCenter(face, min, max);

    float seed = (float) id * 13.13f + ElapsedTime;

    float rx = frac(sin(seed * 1.5f) * 43758.5453);
    float ry = frac(sin(seed * 3.3f) * 33445.4321);
    float rz = frac(sin(seed * 7.7f) * 12345.6789);

    // AABB 크기
    float3 extent = (max - min) * 0.5f;

    // 면에서 퍼짐 가능한 방향 (해당 face 축 제외)
    float3 offset = float3(
        (face == 0 || face == 1) ? 0.0f : (rx - 0.5f) * extent.x,
        (face == 2 || face == 3) ? 0.0f : (ry - 0.5f) * extent.y,
        (face == 4 || face == 5) ? 0.0f : (rz - 0.5f) * extent.z
    );

    return center + offset;
}

float3 RandomSpreadDirection(uint id, float3 baseDir, float spreadAmount)
{
    // 랜덤 offset 생성 (XZ 평면 기준, 범위: -0.5 ~ +0.5)
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

float3 ConeEmitDirection(uint id, float3 baseDir, float coneAngle)
{
    float seed = frac(sin(id * 91.91) * 12345.6789f);
    float theta = coneAngle * sqrt(frac(sin(seed * 11.11) * 6789.1234f)); // 원뿔 각도 내 랜덤 각도
    float phi = frac(sin(seed * 19.19) * 9876.5432f) * 2.0f * XM_PI;

    float3 orthogonal = abs(baseDir.y) < 0.99f ? float3(0.0f, 1.0f, 0.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 right = normalize(cross(baseDir, orthogonal));
    float3 up = normalize(cross(baseDir, right));

    return normalize(
        baseDir * cos(theta) +
        right * sin(theta) * cos(phi) +
        up * sin(theta) * sin(phi)
    );
}

//===============================================================

void Emit_Snow(inout Particle_Info p, uint index)
{
    p.Position = RandomEmitPosition(index * (p.Type + 1), EmitRegionMin, EmitRegionMax, p.EmitFaceIndex);
    float3 dir = RandomSpreadDirection(index * (p.Type + 1), Main_Direction, 0.5f);
    p.Velocity = normalize(dir) * Init_Velocity_Value;

}

void Emit_Spark(inout Particle_Info p, uint index)
{
    float3 center = (EmitRegionMin + EmitRegionMax) * 0.5f;
    p.Position = center;
    float3 dir = RandomSpreadDirection(index * (p.Type + 1), Main_Direction, 2.0f);
    p.Velocity = normalize(dir) * Init_Velocity_Value;

}

void Emit_Water_Splash(inout Particle_Info p, uint index)
{
    float3 center = (EmitRegionMin + EmitRegionMax) * 0.5f;
    p.Position = center;

    float3 baseDir = normalize(Main_Direction);
    float3 right = normalize(cross(float3(0, 1, 0), baseDir));    
    float side = (index % 2 == 0) ? -1.0f : 1.0f;
    float3 liftedBaseDir = normalize(baseDir + float3(0, 0.5f, 0));
    float3 spreadDir = normalize(liftedBaseDir + right * side * 0.5f);
    float3 finalDir = RandomSpreadDirection(index, spreadDir, 0.5f);
    p.Velocity = normalize(finalDir) * Init_Velocity_Value;
    p.Color = float3(0.6f, 0.8f, 1.0f);
}

void Emit_Sand(inout Particle_Info p, uint index)
{
    
    p.Position = RandomEmitPosition(index * (p.Type + 1), EmitRegionMin, EmitRegionMax, p.EmitFaceIndex);
    float3 dir = RandomSpreadDirection(index * (p.Type + 1), Main_Direction, 0.5f);
    p.Velocity = normalize(dir) * Init_Velocity_Value;

}

void Emit_Sand_Storm(inout Particle_Info p, uint index)
{
    float3 baseDir = normalize(Main_Direction);
    float3 realMin = min(EmitRegionMin, EmitRegionMax);
    float3 realMax = max(EmitRegionMin, EmitRegionMax);
    float3 emitCenter = GetEmitFaceCenter(p.EmitFaceIndex, realMin, realMax);

    float3 offset = RandomSpreadDirection(index * (p.Type + 1), baseDir, 1.0f);
    float startRadius = 5.0f + frac(sin(index * 17.17f) * 1234.5678f) * 10.0f;
    p.Position = emitCenter + offset * startRadius;

    float3 tangent = normalize(cross(baseDir, offset));
    float3 initialDir = normalize(baseDir * 0.7f + tangent * 0.3f);

    // float noise = frac(sin(index * 31.31f) * 6789.1234f); 
    // initialDir += RandomSpreadDirection(index, baseDir, 0.2f) * 0.2f; // 방향 노이즈 (선택 적용)
    // initialDir = normalize(initialDir);

    float seed = frac(sin(index * 91.91f) * 10000.0f);
    float delay = lerp(0.0f, 2.0f, seed); // 최대 2초까지 랜덤 딜레이
    p.Lifetime = -delay;
    
    p.Velocity = initialDir * Init_Velocity_Value;
}

void Emit_DragonFire(inout Particle_Info p, uint index)
{
    float3 center = (EmitRegionMin + EmitRegionMax) * 0.5f;
    p.Position = center;

    float coneAngle = radians(20.0f);
    float3 dir = ConeEmitDirection(index, Main_Direction, coneAngle);

    float angle_from_center = acos(dot(normalize(dir), normalize(Main_Direction)));
    float t = saturate(angle_from_center / coneAngle);
    float speedMultiplier = lerp(1.5f, 0.7f, t); // center particles move faster

    p.Velocity = normalize(dir) * Init_Velocity_Value * speedMultiplier;
    p.Color = float3(1.0f, 1.0f, 1.0f); // initial white
    p.Size = 0.1f; // start small
}

//===============================================================

#define PARTICLE_TYPE_SNOW     0
#define PARTICLE_TYPE_SPARK     1
#define PARTICLE_TYPE_SPLASH    2
#define PARTICLE_TYPE_SAND      3
#define PARTICLE_TYPE_SAND_STORM 4
#define PARTICLE_TYPE_DRAGON_FIRE 5


#define THREAD_COUNT 64
[numthreads(THREAD_COUNT, 1, 1)]
void EmitCS(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;

    if (index >= Max_Particle_N)
        return;

    InterlockedAdd(debug_buffer[0], 1); // 호출 카운트

    Particle_Info p = ParticleBuffer_Emit[index];
    if (p.Active == 1)
        return;

    InterlockedAdd(debug_buffer[1], 1); // emit 카운트

    p.Active = 1;
    p.Lifetime = 0.0f;

    if (p.Type == PARTICLE_TYPE_SNOW)
        Emit_Snow(p, index);
    else if (p.Type == PARTICLE_TYPE_SPARK)
        Emit_Spark(p, index);
    else if (p.Type == PARTICLE_TYPE_SPLASH)
    {
        float seed = frac(sin(index * 91.91f) * 12345.6789f);
        float startDelay = seed * 0.1f;

        if (ElapsedTime >= startDelay)
        {
            p.Active = 1;
            p.Lifetime = 0.0f;
            Emit_Water_Splash(p, index);
        }
        else
        {
            return; 
        }
    }
    else if (p.Type == PARTICLE_TYPE_SAND)
        Emit_Sand(p, index);
    else if (p.Type == PARTICLE_TYPE_SAND_STORM)
        Emit_Sand_Storm(p, index);
    else if (p.Type == PARTICLE_TYPE_DRAGON_FIRE)
        Emit_DragonFire(p, index);
    
    
    
    float seedDelay = frac(sin(index * 97.13f + ElapsedTime * 33.33f) * 31415.9265f);
    float randomDelay = seedDelay * p.MaxLifetime;
    p.Lifetime = -randomDelay; // 음수 Lifetime 설정
    p.Active = 0; // 초기 비활성 상태

    
    ParticleBuffer_Emit[index] = p;
}