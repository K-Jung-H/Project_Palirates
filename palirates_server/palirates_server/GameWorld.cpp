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
    //Particle_Format p;
    //p.area_xyz = XMFLOAT3{ 1000,2000,3000 };
    //p.lifetime = 300;
    //p.main_direction = XMFLOAT3{ 0,0,1 };
    //p.particle_type = Particle_Type::dragon_breath;

    //shared_ptr<Particle_Object> p_obj = particle_manager.Create_Particle_Object(p);
    //p_obj->SetNeedSyncType(true);
    //p_obj->SetPosition(1500, 50, 800);
    //p_obj->SetLook(XMFLOAT3{ 0,0,-1 });
}

void GameWorld::Load_Scene_Data(shared_ptr<GameObject> scene_obj)
{
    fixed_object_list.clear();
	FlattenGameObjectHierarchy_Filter(scene_obj, fixed_object_list);
    AssignToUniformCells();
}

void GameWorld::FlattenGameObjectHierarchy_Filter(std::shared_ptr<GameObject> node, std::vector<shared_ptr<GameObject>>& outList)
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
        FlattenGameObjectHierarchy_Filter(child, outList);
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

                            // Compute push direction (from other to player)
                            XMVECTOR pushDir = XMLoadFloat3(&player_worldOBB.Center) - XMLoadFloat3(&otherOBB.Center);

                            if (!XMVector3Equal(pushDir, XMVectorZero()))
                            {
                                pushDir = XMVector3Normalize(pushDir);
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
        else
            player_obj->need_to_client_sync = true;


        // 평균 푸시 방향 계산
        XMVECTOR avgPushDir = XMVector3Normalize(totalPushDir);

        // 이동할 때만 Y 성분 제거 → 방향은 유지, 수직 이동만 차단
        XMVECTOR newCenter = XMLoadFloat3(&player_worldOBB.Center) + XMVectorSet(
            XMVectorGetX(avgPushDir) * pushStrength,
            0.0f,
            XMVectorGetZ(avgPushDir) * pushStrength,
            0.0f
        );
        XMStoreFloat3(&player_worldOBB.Center, newCenter);
    }

    player_obj->Set_Collider_OBB_Center(player_worldOBB.Center);
}

void GameWorld::Update_Monster(float elapsed_time)
{
}

void GameWorld::Update_Particle(float elapsed_time)
{
    particle_manager.Update_Particle(elapsed_time);
}

FrameParticleChanges GameWorld::Get_Particle_Sync_Data()
{
    return particle_manager.FlushFrameChanges();
}

std::vector<BoundingOrientedBox> GameWorld::Get_Cell_OBBs(const XMFLOAT3& Pos)
{
    std::vector<BoundingOrientedBox> obbs;
    XMINT3 cellPos = Get_CellIndexFromPosition(Pos);
    auto it = uniform_cell_map.find(cellPos);
    if (it == uniform_cell_map.end()) return obbs;

    for (int objIndex : it->second)
    {
        auto obj = fixed_object_list[objIndex];
        if (!obj || !obj->Get_Collider_OBB()) continue;

        obj->UpdateWorldOBB();
        obbs.push_back(*obj->Get_Collider_OBB());
    }

    return obbs;
}