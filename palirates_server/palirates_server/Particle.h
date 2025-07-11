#pragma once
#include "GameObject.h"
#include <queue>

enum class Particle_Type
{
    bleed,
    sand,
    dragon_breath,
    etc,
};

struct Particle_Format
{
    Particle_Type particle_type;
    XMFLOAT3 area_xyz{};
    XMFLOAT3 main_direction{};
    float lifetime;
};

class Particle_Object : public GameObject
{
private:
    Particle_Format particle_format;
    UINT particle_id = 0;

    float LifeTime = 0.0f;
    bool Active = false;

public:
    Particle_Object(UINT p_id, Particle_Format p_format);
    virtual ~Particle_Object();

    Particle_Format Get_Format() { return particle_format; };

    void Update(float elapsedtime);
};

class ParticleManager
{
private:
    std::unordered_map<uint32_t, std::shared_ptr<Particle_Object>> particle_map;
    std::queue<UINT> reusable_ids;
    UINT next_id = 1;

private:
    uint32_t AllocateID();
    void ReleaseID(uint32_t id);


public:
    std::shared_ptr<Particle_Object> Create_Particle_Object(Particle_Format p_format);
    void Remove_Particle_Object(UINT particle_id);
    std::shared_ptr<Particle_Object> Get_Particle_Object(UINT particle_id) const;

    void Clear(); // Remove all particles

    void Update_Particle(float elapsed_time);

};