#include "stdafx.h"
#include "Particle.h"


Particle_Object::Particle_Object(UINT p_id, Particle_Format p_format)
{
    particle_id = p_id;
	particle_format = p_format;
    LifeTime = p_format.lifetime;
    Active = true;
}

Particle_Object::~Particle_Object()
{
}

void Particle_Object::Update(float elapsedtime)
{
    if (!Active)
        return;

    LifeTime -= elapsedtime;

    if (LifeTime <= 0.0f)
    {
        Active = false;
        return;
    }

    // etc
}

//====================================================

UINT ParticleManager::AllocateID()
{
    if (!reusable_ids.empty())
    {
        uint32_t id = reusable_ids.front();
        reusable_ids.pop();
        return id;
    }
    return next_id++;
}

void ParticleManager::ReleaseID(uint32_t id)
{
    reusable_ids.push(id);
}

std::shared_ptr<Particle_Object> ParticleManager::Create_Particle_Object(Particle_Format p_format)
{
    std::lock_guard<std::mutex> lock(particle_manage_mutex);
    UINT id = AllocateID();

    auto particle = std::make_shared<Particle_Object>(id, p_format);
    particle_map[id] = particle;
    created_this_frame.push_back(particle);

    return particle;
}

void ParticleManager::Remove_Particle_Object(UINT particle_id)
{
    std::lock_guard<std::mutex> lock(particle_manage_mutex);

    auto it = particle_map.find(particle_id);
    if (it != particle_map.end())
    {
        particle_map.erase(it);
        ReleaseID(particle_id);

        removed_this_frame.push_back(particle_id);
    }
}

std::shared_ptr<Particle_Object> ParticleManager::Get_Particle_Object(UINT particle_id) const
{
    auto it = particle_map.find(particle_id);
    if (it != particle_map.end())
        return it->second;
    return nullptr;
}

void ParticleManager::Clear()
{
    particle_map.clear();
    std::queue<uint32_t> empty;
    std::swap(reusable_ids, empty);
    next_id = 1;
}

void ParticleManager::Update_Particle(float elapsed_time)
{
    std::vector<UINT> to_remove;
    {
        std::lock_guard<std::mutex> lock(particle_manage_mutex);


        for (auto& [id, particle] : particle_map)
        {
            particle->Update(elapsed_time);

            if (!particle->IsActive())
            {
                to_remove.push_back(id);
                continue;
            }

            if (particle->IsContinuousSyncType())
                pos_updated_this_frame.push_back(particle);
        }
    }

    for (UINT id : to_remove)
    {
        Remove_Particle_Object(id);
    }
}

FrameParticleChanges ParticleManager::FlushFrameChanges()
{
    std::lock_guard<std::mutex> lock(particle_manage_mutex);

    FrameParticleChanges changes;
    changes.created = std::move(created_this_frame);
    changes.removed = std::move(removed_this_frame);
    changes.pos_updated = std::move(pos_updated_this_frame);

    created_this_frame.clear();
    removed_this_frame.clear();
    pos_updated_this_frame.clear();

    return changes;
}