cbuffer cbUnifiedBuffer : register(b0)
{
    float gfElapsedTime; 
    uint MaxParticle;
};


struct VS_PARTICLE_INPUT
{
    float3 position : POSITION; 
    float3 velocity : VELOCITY; 
    float lifetime : LIFETIME; 
    uint type : PARTICLETYPE; 
};


RWStructuredBuffer<VS_PARTICLE_INPUT> ParticleBuffer : register(u0);


[numthreads(256, 1, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;

    //if (index >= MaxParticle)
    //    return;

    VS_PARTICLE_INPUT particle = ParticleBuffer[index];
    particle.position += particle.velocity * gfElapsedTime;
    particle.lifetime -= gfElapsedTime;

    
        ParticleBuffer[index] = particle;
}