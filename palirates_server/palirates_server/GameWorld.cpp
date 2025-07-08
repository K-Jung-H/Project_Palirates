#include "stdafx.h"
#include "GameWorld.h"



GameWorld::GameWorld()
{
	Init();
}

GameWorld::~GameWorld()
{
}

void GameWorld::Init()
{
}

void GameWorld::Load_Scene_Data(shared_ptr<GameObject> scene_obj)
{
	fixed_object_list.clear();
	FlattenGameObjectHierarchy(scene_obj, fixed_object_list);
}

void GameWorld::FlattenGameObjectHierarchy(std::shared_ptr<GameObject> node, std::vector<shared_ptr<GameObject>>& outList)
{
    if (!node) return;

    outList.push_back(node);

    std::shared_ptr<GameObject> child = node->Get_Child();
    while (child)
    {
        FlattenGameObjectHierarchy(child, outList);
        child = child->Get_Sibling();
    }
}

void GameWorld::AssignToUniformCells()
{
    uniform_cell_map.clear();

    for (UINT i = 0; i < static_cast<UINT>(fixed_object_list.size()); ++i)
    {
        shared_ptr<GameObject> obj = fixed_object_list[i];

        XMFLOAT3 pos = obj->GetPosition(); 

        // Cell ÁÂÇ¥ °è»ê
        XMINT3 cellPos;
        cellPos.x = static_cast<int>(floor(pos.x / grid_cell_size));
        cellPos.y = static_cast<int>(floor(pos.y / grid_cell_size));
        cellPos.z = static_cast<int>(floor(pos.z / grid_cell_size));

        // ÇØ´ç ¼¿¿¡ ÀÎµ¦½º Ãß°¡
        uniform_cell_map[cellPos].push_back(i);
    }
}


void GameWorld::Update_Collision(shared_ptr<Player> player_obj)
{
}

void GameWorld::Update_Monster(float elapsed_time)
{
}