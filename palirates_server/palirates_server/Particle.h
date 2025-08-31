#pragma once
#include "GameObject.h"
#include <queue>

enum class Particle_Type
{
    snow = 0,
    splash = 1,
    dragon_breath = 2,
    party = 3,
    sand = 4,
    sand_storm = 5,
    heal = 6,
    orbit = 7,
    diffuse_burst = 8,
    diffuse_continuous = 9,
    //=======================
    bleed = 10,
    //=======================
    env_snow = 20,
    env_sand = 21,
    //=======================
    weapon_spear_skill_1 = 30,
    weapon_twin_sword_skill_1 = 31,

    //=======================
    etc = -1
};

struct Particle_Format
{
    Particle_Type particle_type;
    XMFLOAT3 particle_color;

    float lifetime;
    XMFLOAT3 area_xyz{};
    XMFLOAT3 main_direction{};
    XMFLOAT3 focus_point;
};

class Particle_Object : public GameObject
{
protected:
    Particle_Format particle_format;
    UINT particle_id = 0;
    UINT particle_status = -1;
    float LifeTime = 0.0f;
    bool Active = false;

    bool synchronize = false; // 매 프레임마다 클라이언트에게 월드 정보 전송

public:
    Particle_Object(UINT p_id, Particle_Format p_format);
    virtual ~Particle_Object();

    Particle_Format Get_Format() { return particle_format; };
    void Set_Format(Particle_Format p_format) { particle_format = p_format;; };


    UINT Get_Particle_ID() { return particle_id; }
    float Get_LifeTime() { return LifeTime; }

    void Set_Particle_Status(UINT new_index) { particle_status = new_index; }

    UINT Get_Particle_Status() { return particle_status; }

    void Set_Synchronize_Type(bool enable) { synchronize = enable; }
    bool Get_Synchronize_Type() const { return synchronize; }

    bool IsActive() { return Active; }
    void SetActive(bool active) { Active = active; }

    void Update(float elapsedtime);


};

struct FrameParticleChanges
{
    std::vector<std::shared_ptr<Particle_Object>> created;
    std::vector<UINT> removed; 
    std::vector<std::shared_ptr<Particle_Object>> pos_updated;
};

class ParticleManager
{
protected:
    std::vector<std::shared_ptr<Particle_Object>> created_this_frame;
    std::vector<UINT> removed_this_frame;
    std::vector<std::shared_ptr<Particle_Object>> pos_updated_this_frame;

private:
    std::unordered_map<uint32_t, std::shared_ptr<Particle_Object>> particle_map;
    std::queue<UINT> reusable_ids;
    UINT next_id = 1;

    std::mutex particle_manage_mutex;
private:
    uint32_t AllocateReusableID();
    uint32_t AllocateUniqueID();
    void ReleaseID(uint32_t id);



public:
    std::shared_ptr<Particle_Object> Create_Particle_Object(Particle_Format p_format);
    void Remove_Particle_Object(UINT particle_id);
    std::shared_ptr<Particle_Object> Get_Particle_Object(UINT particle_id) const;

    void Clear(); // Remove all particles

    void Update_Particle(float elapsed_time);
    FrameParticleChanges FlushFrameChanges(); // 매 프레임마다 클라에 전달할 내용

};