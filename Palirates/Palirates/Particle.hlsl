#include "Shaders.hlsl"

#define PARTICLE_TYPE_EMITTER		0
#define PARTICLE_TYPE_SHELL			1
#define PARTICLE_TYPE_FLARE01		2
#define PARTICLE_TYPE_FLARE02		3
#define PARTICLE_TYPE_FLARE03		4

#define SHELL_PARTICLE_LIFETIME		3.0f
#define FLARE01_PARTICLE_LIFETIME   	2.5f
#define FLARE02_PARTICLE_LIFETIME   	1.5f
#define FLARE03_PARTICLE_LIFETIME   2.0f

Buffer<float4> gRandomBuffer : register(t8);

struct VS_PARTICLE_INPUT
{
    float3 position : POSITION;
    float3 velocity : VELOCITY;
    float lifetime : LIFETIME;
    uint type : PARTICLETYPE;
};

VS_PARTICLE_INPUT VSParticleStreamOutput(VS_PARTICLE_INPUT input)
{
    return (input);
}


float4 RandomDirection(float fOffset)
{
    int u = uint(gfCurrentTime + fOffset + frac(gfCurrentTime) * 1000.0f) % 1024;
    return (normalize(gRandomBuffer.Load(u)));
}

float4 RandomDirectionOnSphere(float fOffset)
{
    int u = uint(gfCurrentTime + fOffset + frac(gfCurrentTime) * 1000.0f) % 256;
    return (normalize(gRandomBuffer.Load(u)));
}

void OutputParticleToStream(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
//    input.position += input.velocity * gfElapsedTime;

    input.velocity += gf3Gravity * gfElapsedTime;
    input.lifetime -= gfElapsedTime;

    output.Append(input);
}

void EmmitParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
    float4 f4Random = RandomDirection(input.type);
    if (input.lifetime <= 0.0f)
    {
        VS_PARTICLE_INPUT particle = input;

        particle.type = PARTICLE_TYPE_SHELL;

        particle.velocity = input.velocity + (f4Random.xyz * 16.0f);
        particle.lifetime = SHELL_PARTICLE_LIFETIME + (f4Random.y * 0.5f);

        output.Append(particle);

        input.lifetime = gfSecondsPerFirework * 0.2f + (f4Random.x * 0.4f);
    }

    output.Append(input);
}


void ShellParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
    if (input.lifetime <= 0.0f)
    {
        VS_PARTICLE_INPUT particle = input;
        float4 f4Random = float4(0.0f, 0.0f, 0.0f, 0.0f);

        particle.type = PARTICLE_TYPE_FLARE01;
        particle.lifetime = FLARE01_PARTICLE_LIFETIME;

        for (int i = 0; i < gnFlareParticlesToEmit; i++)
        {
            f4Random = RandomDirection(input.type + i);
            particle.velocity = input.velocity + (f4Random.xyz * 18.0f);

            output.Append(particle);
        }

        particle.type = PARTICLE_TYPE_FLARE02;

        for (int j = 0; j < abs(f4Random.x) * gnMaxFlareType2Particles; j++)
        {
            f4Random = RandomDirection(input.type + j);
            particle.velocity = input.velocity + (f4Random.xyz * 10.0f);
            particle.lifetime = FLARE02_PARTICLE_LIFETIME + (f4Random.x * 0.4f);

            output.Append(particle);
        }
    }
    else
    {
        OutputParticleToStream(input, output);
    }
}

void OutputEmberParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
    if (input.lifetime > 0.0f)
    {
        OutputParticleToStream(input, output);
    }
}

void GenerateEmberParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
    if (input.lifetime <= 0.0f)
    {
        VS_PARTICLE_INPUT particle = input;

        particle.type = PARTICLE_TYPE_FLARE03;
        particle.lifetime = FLARE03_PARTICLE_LIFETIME;
        
        for (int i = 0; i < 100; i++)
        {
            float4 f4Random = RandomDirectionOnSphere(input.type + i);
            particle.velocity = input.velocity + (f4Random.xyz * 25.0f);

            output.Append(particle);
        }
    }
    else
    {
        OutputParticleToStream(input, output);
    }
}

[maxvertexcount(128)]
void GSParticleStreamOutput(point VS_PARTICLE_INPUT input[1], inout PointStream<VS_PARTICLE_INPUT> output)
{
    VS_PARTICLE_INPUT particle = input[0];

    if (particle.type == PARTICLE_TYPE_EMITTER) 
        EmmitParticles(particle, output);
    else if (particle.type == PARTICLE_TYPE_SHELL)
        ShellParticles(particle, output);
    else if ((particle.type == PARTICLE_TYPE_FLARE01) || (particle.type == PARTICLE_TYPE_FLARE03))
        OutputEmberParticles(particle, output);
    else if (particle.type == PARTICLE_TYPE_FLARE02) 
        GenerateEmberParticles(particle, output);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


struct VS_INSTANCE_PARTICLE_DRAW_INPUT
{
    float3 position : POSITION;


    float3 instancePos : INSTANCE_POSITION;
    float3 velocity : INSTANCE_VELOCITY;
    float4 color : INSTANCE_COLOR;
};

struct VS_INSTANCE_PARTICLE_DRAW_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float4 color : COLOR;
};


VS_INSTANCE_PARTICLE_DRAW_OUTPUT VSParticleDraw(VS_INSTANCE_PARTICLE_DRAW_INPUT input)
{
    VS_INSTANCE_PARTICLE_DRAW_OUTPUT output = (VS_INSTANCE_PARTICLE_DRAW_OUTPUT) 0;
    
    // 파티클 위치 계산 (월드 좌표 적용)
    float4 particleWorldPosition = float4(input.position + input.instancePos, 1.0f);
    
    float4 positionW = mul(particleWorldPosition, gmtxGameObject);
    output.position = mul(mul(positionW, gmtxView), gmtxProjection);    
    output.positionW = positionW.xyz;
    
    // 색상과 기타 속성 설정
    output.color = input.color;

    
    return output;
}

// Pixel Shader
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PS_Deffered_ParticleDraw(VS_INSTANCE_PARTICLE_DRAW_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    output.Albedo_Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    output.world_Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.world_Normal_and_Camera_Distance = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Material_Light_Info = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Velocity = float4(0.0f, 0.0f, 0.0f, 0.0f);
    


    output.Albedo_Color = input.color;
    
    output.world_Position = float4(input.positionW, 1.0f);
    output.world_Normal_and_Camera_Distance.xyz = float3(0.0f, 1.0f, 0.0f);
    output.world_Normal_and_Camera_Distance.w = distance(input.positionW, gvCameraPosition);

    output.Material_Light_Info = float4(material_info.gRoughness, 0.0f, material_info.gSpecular_intensity, material_info.gEmissive_intensity);
    
    return output;
}



