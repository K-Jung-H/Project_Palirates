#pragma once
#include "stdafx.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "Particle.h"

namespace DirectX
{
	inline bool operator==(const XMINT3& lhs, const XMINT3& rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}
}

struct UpdateContext 
{
	float dt = 0.f;
	bool stage_clear = false;
	std::shared_ptr<GameObject>* out_zoom_object = nullptr;
};


struct SceneLogic
{
	SceneLogic(XMFLOAT3 init_scene_area, XMFLOAT3 init_scene_center) { scene_area = init_scene_area; scene_center = init_scene_center; }
	virtual ~SceneLogic() = default;
	virtual void init(ParticleManager& pm) { p_mg = &pm; }

	virtual void setBoss(const std::shared_ptr<Monster>& b) { boss_ptr = b; }

	virtual void update(const UpdateContext& ctx) = 0;
	virtual void onEnter() {}
	virtual void onExit() {}

	void Set_Scene_Area(XMFLOAT3 new_scene_area, XMFLOAT3 new_scene_center) { scene_area = new_scene_area;  scene_center = new_scene_center; }

protected:
	XMFLOAT3 scene_area;
	XMFLOAT3 scene_center;

	std::weak_ptr<Monster> boss_ptr;
	ParticleManager* p_mg = nullptr;

};

class Dragon_Stage_SceneLogic final : public SceneLogic 
{
public:
	Dragon_Stage_SceneLogic(XMFLOAT3 init_scene_area, XMFLOAT3 init_scene_center) :SceneLogic(init_scene_area, init_scene_center) {}

	~Dragon_Stage_SceneLogic() override { onExit(); };
	void onExit() override;

	
	void init(ParticleManager& pm) override;
	void update(const UpdateContext& ctx) override;

private:
	std::shared_ptr<Particle_Object> dragon_fire;
};

class Anubis_Stage_SceneLogic final : public SceneLogic
{
public:
	Anubis_Stage_SceneLogic(XMFLOAT3 init_scene_area, XMFLOAT3 init_scene_center) :SceneLogic(init_scene_area, init_scene_center) {}

	~Anubis_Stage_SceneLogic() override { onExit(); }
	void onExit() override;



	void init(ParticleManager& pm) override;
	void update(const UpdateContext& ctx) override;

private:
	std::shared_ptr<Particle_Object> sand_env;
	std::shared_ptr<Particle_Object> sand_anubis_effect;
};

class Gargoyle_Stage_SceneLogic final : public SceneLogic
{
public:
	Gargoyle_Stage_SceneLogic(XMFLOAT3 init_scene_area, XMFLOAT3 init_scene_center) :SceneLogic(init_scene_area, init_scene_center) {}

	~Gargoyle_Stage_SceneLogic() override { onExit(); }
	void onExit() override;



	void init(ParticleManager& pm) override;
	void update(const UpdateContext& ctx) override;

private:
	std::shared_ptr<Particle_Object> gargoyle_skill_effect;
};

//=========================================================

struct XMINT3Hasher
{
	std::size_t operator()(const XMINT3& k) const noexcept
	{
		return std::hash<int>()(k.x) ^ std::hash<int>()(k.y << 1) ^ std::hash<int>()(k.z << 2);
	}
};

class GameWorld
{
protected:
	Scene_Type scene_type = Scene_Type::etc;
	XMFLOAT3 scene_area;
	XMFLOAT3 scene_center;

	bool Stage_Clear = false;
	bool Party_Start = false;

private:
	std::unique_ptr<SceneLogic> scene_logic;

	std::vector<shared_ptr<GameObject>> fixed_object_list;
	ParticleManager particle_manager;

	std::unordered_map<XMINT3, std::vector<UINT>, XMINT3Hasher> uniform_cell_map;

	float grid_cell_size = 100.0f;

	//=======================

	shared_ptr<Monster> boss_monster = NULL;
	shared_ptr<GameObject> zoom_object = NULL;


	std::array< shared_ptr<Particle_Object>, MaxPlayer> party_effect;
	shared_ptr<Particle_Object> dragon_fire;
	shared_ptr<Particle_Object> sand;

public:
	GameWorld(Scene_Type scene_type);
	~GameWorld();

	void Init(Scene_Type scene_type);
	bool Get_Clear_State() { return Stage_Clear; }
	void Set_Clear_State(bool stage_clear) { Stage_Clear = stage_clear; }

	shared_ptr<Monster> Get_Boss_Monster() { return boss_monster; }
	void Set_Boss_Moster(shared_ptr<Monster> boss_ptr);


	shared_ptr<GameObject> Get_ZoomObject() { return zoom_object; }

	void Load_Scene_Data(shared_ptr<GameObject> scene_obj);

	void Update_World(float dt);
	void Update_Collision(shared_ptr<Player> player_obj);
	void Update_Particle(float elapsed_time);

	void Add_Bleeding_Particle(XMFLOAT3& pos, XMFLOAT3& main_direction);
	void Stage_Clear_Particle_Update(std::array<std::shared_ptr<Player>, MaxPlayer> player_list);
	void Sand_Update();

	FrameParticleChanges Get_Particle_Sync_Data();
	std::vector<BoundingOrientedBox> Get_Cell_OBBs(const XMFLOAT3& Pos);


private:
	void FlattenGameObjectHierarchy_Filter(std::shared_ptr<GameObject> node, std::vector<shared_ptr<GameObject>>& outList);
	void AssignToUniformCells();

	XMINT3 Get_CellIndexFromPosition(const XMFLOAT3& pos) const;
	void Compute_CellBounds_From_OBB(const std::shared_ptr<BoundingOrientedBox>& obb, XMINT3& out_min_cell, XMINT3& out_max_cell) const;
};

const std::unordered_set<std::string> kExcludedNames =
{
	"SM_Env_Rock_02.bin",
	"SM_Env_Rock_03.bin",
	"SM_Env_Rock_Huge_03.bin",
	"SM_Env_Rock_Huge_04.bin",
	"SM_Env_Rock_Large_02.bin",
	"SM_Env_Rock_Large_03.bin",
	"SM_Env_Rock_Large_04.bin",
	"SM_Env_Rock_Large_05.bin",
	"SM_Env_Rock_Large_06.bin",
	"SM_Env_Rock_Large_07.bin",
	"SM_Env_Rock_Large_08.bin",
	"SM_Env_Rock_Skull_01.bin"
};