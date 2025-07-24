#pragma once

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


struct XMINT3Hasher
{
	std::size_t operator()(const XMINT3& k) const noexcept
	{
		return std::hash<int>()(k.x) ^ std::hash<int>()(k.y << 1) ^ std::hash<int>()(k.z << 2);
	}
};


class GameWorld
{
private:
	bool Stage_Clear = false;

	std::vector<shared_ptr<GameObject>> fixed_object_list;
	ParticleManager particle_manager;

	std::unordered_map<XMINT3, std::vector<UINT>, XMINT3Hasher> uniform_cell_map;

	float grid_cell_size = 100.0f;
	//=======================
	shared_ptr<Monster> boss_monster = NULL;
	shared_ptr<GameObject> zoom_object = NULL;

	shared_ptr<Particle_Object> dragon_fire;
	shared_ptr<Particle_Object> sand;
public:
	GameWorld();
	~GameWorld();

	void Init();
	bool Get_Clear_State() { return Stage_Clear; }
	void Set_Clear_State(bool stage_clear) { Stage_Clear = stage_clear; }

	shared_ptr<Monster> Get_Boss_Monster() { return boss_monster; }
	void Set_Boss_Moster(shared_ptr<Monster> boss_ptr) { boss_monster = boss_ptr; }
	void Boss_Update();

	shared_ptr<GameObject> Get_ZoomObject() { return zoom_object; }

	void Load_Scene_Data(shared_ptr<GameObject> scene_obj);
	void Update_Collision(shared_ptr<Player> player_obj);

	void Update_Particle(float elapsed_time);
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