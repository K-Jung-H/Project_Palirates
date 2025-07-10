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
    scene_obj->SetPosition(1300.0f, -27.0f, 800.0f);
    scene_obj->SetScale(10, 10, 10, true);
    scene_obj->UpdateTransform(NULL);

    fixed_object_list.clear();
	FlattenGameObjectHierarchy(scene_obj, fixed_object_list);
    AssignToUniformCells();
}

void GameWorld::FlattenGameObjectHierarchy(std::shared_ptr<GameObject> node, std::vector<shared_ptr<GameObject>>& outList)
{
    if (!node) return;

    const std::string& name = node->Get_Name();


    if (!name.empty() && name.find("Env") == std::string::npos)
    {
        outList.push_back(node);
    }


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
        auto obj = fixed_object_list[i];
        obj->UpdateWorldOBB();

        if (!obj->m_OBB)
            continue;

        XMINT3 minCell, maxCell;
        Compute_CellBounds_From_OBB(obj->m_OBB, minCell, maxCell);

        for (int x = minCell.x; x <= maxCell.x; ++x)
            for (int y = minCell.y; y <= maxCell.y; ++y)
                for (int z = minCell.z; z <= maxCell.z; ++z)
                    uniform_cell_map[XMINT3(x, y, z)].push_back(i);
    }
}


XMINT3 GameWorld::Get_CellIndexFromPosition(const XMFLOAT3& pos) const
{
    return XMINT3{
        static_cast<int>(std::floor(pos.x / grid_cell_size)),
        static_cast<int>(std::floor(pos.y / grid_cell_size)),
        static_cast<int>(std::floor(pos.z / grid_cell_size))
    };
}
void GameWorld::Compute_CellBounds_From_OBB(const std::shared_ptr<BoundingOrientedBox>& obb, XMINT3& out_min_cell, XMINT3& out_max_cell) const
{
    if (!obb) return;

    XMFLOAT3 corners[8];
    obb->GetCorners(corners);

    BoundingBox aabb;
    BoundingBox::CreateFromPoints(aabb, 8, corners, sizeof(XMFLOAT3));

    XMFLOAT3 min = {
        aabb.Center.x - aabb.Extents.x,
        aabb.Center.y - aabb.Extents.y,
        aabb.Center.z - aabb.Extents.z
    };
    XMFLOAT3 max = {
        aabb.Center.x + aabb.Extents.x,
        aabb.Center.y + aabb.Extents.y,
        aabb.Center.z + aabb.Extents.z
    };

    out_min_cell = Get_CellIndexFromPosition(min);
    out_max_cell = Get_CellIndexFromPosition(max);
}

void GameWorld::Update_Collision(std::shared_ptr<Player> player_obj)
{
    if (!player_obj) return;

    constexpr int maxIterations = 10; // 무한 루프 방지를 위한 최대 반복 수
    constexpr float pushDist = 1.0f; // 한 번에 밀어내는 거리
    bool collisionOccurred;

    XMFLOAT3 playerPos = player_obj->GetPosition();

    XMVECTOR playerPosVec = XMLoadFloat3(&playerPos);

    for (int iteration = 0; iteration < maxIterations; ++iteration)
    {
        collisionOccurred = false;

        XMINT3 cellPos = Get_CellIndexFromPosition(playerPos);

        for (int x = cellPos.x - 1; x <= cellPos.x + 1; ++x)
        {
            for (int y = cellPos.y - 1; y <= cellPos.y + 1; ++y)
            {
                for (int z = cellPos.z - 1; z <= cellPos.z + 1; ++z)
                {
                    XMINT3 checkCell = { x, y, z };
                    auto it = uniform_cell_map.find(checkCell);
                    if (it == uniform_cell_map.end()) continue;

                    for (int objIndex : it->second)
                    {
                        auto other = fixed_object_list[objIndex];
                        if (!other) continue;

                        other->UpdateWorldOBB();
                        auto obb = other->m_OBB;
                        if (!obb) continue;

                        if (obb->Contains(playerPosVec) != DirectX::DISJOINT)
                        {
                            collisionOccurred = true;

                            // 충돌 발생 시 밀어내기 방향 계산
                            XMVECTOR obbCenter = XMLoadFloat3(&obb->Center);
                            XMVECTOR pushDir = XMVector3Normalize(playerPosVec - obbCenter);

                            if (XMVector3Equal(pushDir, XMVectorZero()))
                                pushDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

                            playerPosVec += XMVectorScale(pushDir, pushDist);
                        }
                    }
                }
            }
        }

        if (!collisionOccurred)
            break; 
        else
            XMStoreFloat3(&playerPos, playerPosVec); 
    }


    XMStoreFloat3(&playerPos, playerPosVec);
    player_obj->SetPosition(playerPos);
}


void GameWorld::Update_Monster(float elapsed_time)
{
}