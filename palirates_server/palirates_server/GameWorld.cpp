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

        if (!obj->Get_Collider_OBB())
            continue;

        XMINT3 minCell, maxCell;
        Compute_CellBounds_From_OBB(obj->Get_Collider_OBB(), minCell, maxCell);

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

//void GameWorld::Update_Collision(std::shared_ptr<Player> player_obj)
//{
//    if (!player_obj) return;
//
//    player_obj->UpdateWorldOBB();
//    
//    shared_ptr<BoundingOrientedBox> player_obb = player_obj->Get_Collider_OBB();
//
//    constexpr int maxIterations = 10;
//    constexpr float pushDist = 1.0f;
//    bool collisionOccurred;
//
//    for (int iteration = 0; iteration < maxIterations; ++iteration)
//    {
//        collisionOccurred = false;
//
//        // 현재 OBB 위치 기준으로 셀 계산
//        XMINT3 cellPos = Get_CellIndexFromPosition(player_obb->Center);
//
//        for (int x = cellPos.x - 1; x <= cellPos.x + 1; ++x)
//        {
//            for (int y = cellPos.y - 1; y <= cellPos.y + 1; ++y)
//            {
//                for (int z = cellPos.z - 1; z <= cellPos.z + 1; ++z)
//                {
//                    XMINT3 checkCell = { x, y, z };
//                    auto it = uniform_cell_map.find(checkCell);
//                    if (it == uniform_cell_map.end()) continue;
//
//                    for (int objIndex : it->second)
//                    {
//                        auto other = fixed_object_list[objIndex];
//                        if (!other || !other->Get_Collider_OBB()) continue;
//
//                        other->UpdateWorldOBB();
//                        const auto& other_obb = *other->Get_Collider_OBB();
//
//                        // OBB 간 충돌 검사
//                        if (player_obb->Intersects(other_obb))
//                        {
//                            collisionOccurred = true;
//
//                            // 밀어내기 방향 계산
//                            XMVECTOR pCenter = XMLoadFloat3(&player_obb->Center);
//                            XMVECTOR oCenter = XMLoadFloat3(&other_obb.Center);
//                            XMVECTOR pushDir = XMVector3Normalize(pCenter - oCenter);
//
//                            if (XMVector3Equal(pushDir, XMVectorZero()))
//                                pushDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//
//                            // 밀어내기 적용
//                            pCenter += XMVectorScale(pushDir, pushDist);
//                            XMStoreFloat3(&player_obb->Center, pCenter);
//                        }
//                    }
//                }
//            }
//        }
//
//        if (!collisionOccurred)
//            break;
//    }
//
//    // 최종 OBB 위치의 Center 기준으로 플레이어 위치 역산
//    // OBB Center = player_pos + y_offset (0, height, 0)
//    float y_offset = player_obj->Get_Collider_OBB()->Center.y - player_obj->GetPosition().y;
//    XMFLOAT3 finalPos = {
//        player_obb->Center.x,
//        player_obb->Center.y - y_offset,
//        player_obb->Center.z
//    };
//
//    player_obj->SetPosition(finalPos);
//}


void GameWorld::Update_Collision(std::shared_ptr<Player> player_obj)
{
    if (!player_obj || !player_obj->Get_Collider_OBB()) return;

    constexpr float pushStrength = 2.0f;
    constexpr int maxIterations = 10;

    player_obj->UpdateWorldOBB(); // 갱신된 월드 OBB
    auto& player_worldOBB = *player_obj->Get_Collider_OBB();

    for (int iteration = 0; iteration < maxIterations; ++iteration)
    {
        bool collided = false;
        XMVECTOR totalPushDir = XMVectorZero();
        int hitCount = 0;

        XMINT3 cellPos = Get_CellIndexFromPosition(player_worldOBB.Center);
        std::unordered_set<const BoundingOrientedBox*> alreadyProcessed;

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
                        if (!other || !other->Get_Collider_OBB()) continue;

                        other->UpdateWorldOBB();
                        const auto& otherOBB = *other->Get_Collider_OBB();

                        if (alreadyProcessed.count(&otherOBB)) continue;

                        if (player_worldOBB.Intersects(otherOBB))
                        {
                            alreadyProcessed.insert(&otherOBB);
                            collided = true;

                            XMVECTOR pushDir = XMVector3Normalize(
                                XMLoadFloat3(&player_worldOBB.Center) - XMLoadFloat3(&otherOBB.Center)
                            );

                            if (!XMVector3Equal(pushDir, XMVectorZero()))
                            {
                                totalPushDir += pushDir;
                                ++hitCount;
                            }
                        }
                    }
                }
            }
        }

        if (!collided || hitCount == 0)
            break;

        // 평균 푸시 방향 계산
        XMVECTOR avgPushDir = XMVector3Normalize(totalPushDir);
        XMVECTOR newCenter = XMLoadFloat3(&player_worldOBB.Center) + XMVectorScale(avgPushDir, pushStrength);
        XMStoreFloat3(&player_worldOBB.Center, newCenter);
    }

    player_obj->Set_Collider_OBB_Center(player_worldOBB.Center);
}


void GameWorld::Update_Monster(float elapsed_time)
{
}