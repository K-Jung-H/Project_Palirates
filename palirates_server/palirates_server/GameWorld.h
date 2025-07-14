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
	std::vector<shared_ptr<GameObject>> fixed_object_list;
	ParticleManager particle_manager;

	std::unordered_map<XMINT3, std::vector<UINT>, XMINT3Hasher> uniform_cell_map;


	float grid_cell_size = 100.0f;
	//=======================

	vector<shared_ptr<Monster>> monster_list;

public:
	GameWorld();
	~GameWorld();

	void Init();

	void Load_Scene_Data(shared_ptr<GameObject> scene_obj);
	void Update_Collision(shared_ptr<Player> player_obj);

	void Update_Monster(float elapsed_time);
	void Update_Particle(float elapsed_time);
	FrameParticleChanges Get_Particle_Sync_Data();

private:
	void FlattenGameObjectHierarchy(std::shared_ptr<GameObject> node, std::vector<shared_ptr<GameObject>>& outList);
	void AssignToUniformCells();

	XMINT3 Get_CellIndexFromPosition(const XMFLOAT3& pos) const;
	void Compute_CellBounds_From_OBB(const std::shared_ptr<BoundingOrientedBox>& obb, XMINT3& out_min_cell, XMINT3& out_max_cell) const;
};

