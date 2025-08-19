#include "stdafx.h"
#include "GameWorld.h"

void Dragon_Stage_SceneLogic::onExit()
{
    if (dragon_fire)
    {
        dragon_fire->SetActive(false);
        dragon_fire.reset();
    }

    boss_ptr.reset();
}

void Dragon_Stage_SceneLogic::init(ParticleManager& pm)
{
    SceneLogic::init(pm);

    if (!dragon_fire)
    {
        Particle_Format p;
        p.area_xyz = XMFLOAT3{ 1000,1000,1000 };
        p.lifetime = 300;
        p.main_direction = XMFLOAT3{ 1,0,0 };
        p.particle_type = Particle_Type::dragon_breath;

        dragon_fire = p_mg->Create_Particle_Object(p);
        dragon_fire->SetActive(false);
    }
}

void Dragon_Stage_SceneLogic::update(const UpdateContext& ctx)
{
    auto boss = boss_ptr.lock();
    if (!boss) 
        return;

    if (boss->attackPhase == -1) 
        return;

    if (boss->Weapon_ptr.empty())
        return;

    auto boss_weapon = boss->Weapon_ptr[0];

    if (ctx.out_zoom_object) 
    {
        if (boss->attackPhase == 1) 
        {
            *ctx.out_zoom_object = boss_weapon ? boss_weapon : nullptr;
        }
        else 
        {
            *ctx.out_zoom_object = nullptr;
        }
    }

    auto dragon = dynamic_cast<Dragon*>(boss.get());
    if (!dragon) return;

    Particle_Format p;
    p.area_xyz = XMFLOAT3{ 1000,1000,1000 };
    p.lifetime = 300;
    p.main_direction = XMFLOAT3{ 1,0,0 };
    p.particle_type = Particle_Type::dragon_breath;

    if (boss->attackPhase == 2) 
    {
        if (!dragon_fire) 
        {
            dragon_fire = p_mg->Create_Particle_Object(p);
            dragon_fire->SetNeedSyncType(true);
        }
        dragon_fire->SetActive(true);
    }
    else 
    {
        if (dragon_fire) 
        {
            dragon_fire->SetActive(false);
            dragon_fire.reset();
        }
    }

    if (!boss_weapon || !dragon_fire || !dragon_fire->Get_Active()) 
        return;

    auto obb = boss_weapon->Get_Collider_OBB();
    if (!obb) return;

    XMVECTOR obbCenter = XMLoadFloat3(&obb->Center);
    XMVECTOR obbRotationQuat = XMLoadFloat4(&obb->Orientation);
    XMVECTOR defaultForward = XMVectorSet(0, 0, 1, 0);
    XMVECTOR rotatedForward = XMVector3Rotate(defaultForward, obbRotationQuat);

    float forwardOffset = 10.0f;
    float heightOffset = -5.0f;
    XMVECTOR offsetVec = rotatedForward * forwardOffset + XMVectorSet(0, heightOffset, 0, 0);
    XMVECTOR finalPos = obbCenter + offsetVec;

    XMFLOAT3 position, forward;
    XMStoreFloat3(&position, finalPos);
    XMStoreFloat3(&forward, rotatedForward);

    dragon_fire->SetPosition(position);
    dragon_fire->SetLook(forward);
}

void Anubis_Stage_SceneLogic::onExit()
{
    if (sand_env)
    {
        sand_env->SetActive(false);
        sand_env.reset();
    }

    if (sand_anubis_effect)
    {
        sand_anubis_effect->SetActive(false);
        sand_anubis_effect.reset();
    }
}

void Anubis_Stage_SceneLogic::init(ParticleManager& pm)
{
    SceneLogic::init(pm);

    if (!sand_env)
    {
        Particle_Format env_sand;
        env_sand.area_xyz = XMFLOAT3{ 10,10,10 };
        env_sand.lifetime = 10000.0f;
        env_sand.main_direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
        env_sand.particle_type = Particle_Type::sand;

        sand_env = p_mg->Create_Particle_Object(env_sand);
        sand_env->Set_Particle_Status(10000);
        sand_env->SetNeedSyncType(true);
        sand_env->SetPosition(XMFLOAT3(2176.0f, 500.0f, 1536.0f));
        sand_env->SetLook(XMFLOAT3(0.0f, 0.0f, -1.0f));
    }

    if (!sand_anubis_effect)
    {
        Particle_Format anubis_sand;
        anubis_sand.area_xyz = XMFLOAT3{ 10,10,10 };
        anubis_sand.lifetime = 10000.0f;
        anubis_sand.main_direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
        anubis_sand.particle_type = Particle_Type::sand;

        sand_anubis_effect = p_mg->Create_Particle_Object(anubis_sand);
        sand_anubis_effect->Set_Particle_Status(10000);
        sand_anubis_effect->SetNeedSyncType(true);
        sand_anubis_effect->SetPosition(XMFLOAT3(2176.0f, 500.0f, 1536.0f));
        sand_anubis_effect->SetLook(XMFLOAT3(0.0f, 0.0f, -1.0f));
    }


}

void Anubis_Stage_SceneLogic::update(const UpdateContext& ctx)
{
    auto boss = boss_ptr.lock();
    if (!boss)
        return;

    if (boss->attackPhase == -1)
        return;

    //if (sand_anubis_effect)
    //{
    //    UINT particle_state = sand_anubis_effect->Get_Particle_Status();
    //    if (particle_state == 0)
    //    {
    //        sand_anubis_effect->SetPosition(scene_center);
    //        sand_anubis_effect->Set_Area(scene_area);
    //        sand_anubis_effect->Set_Focus_Point(scene_center);
    //        sand_anubis_effect->Set_Main_Direction(XMFLOAT3(0.0f, 0.0f, -1.0f));

    //        sand_anubis_effect->Set_Speed(0.0f);
    //    }
    //    else if (particle_state == 1)
    //    {
    //        sand_anubis_effect->SetPosition(scene_center);
    //        sand_anubis_effect->Set_Area(scene_area);
    //        sand_anubis_effect->Set_Focus_Point(boss->GetPosition()); // anubis

    //        sand_anubis_effect->Set_Main_Direction(XMFLOAT3(0.0f, 0.0f, -1.0f));

    //        sand_anubis_effect->Set_Speed(0.0f);

    //    }
    //    else if (particle_state == 2)
    //    {
    //        sand_anubis_effect->SetPosition(boss->GetPosition()); // anubis
    //        sand_anubis_effect->Set_Area(scene_area);
    //        sand_anubis_effect->Set_Focus_Point(boss->GetPosition());
    //        sand_anubis_effect->Set_Main_Direction(XMFLOAT3(0.0f, 1.0f, 0.0f));

    //        // move
    //        sand_anubis_effect->Set_Speed(100.0f);
    //        sand_anubis_effect->Set_Direction(boss->GetLook());
    //    }

    //}
}


GameWorld::GameWorld(Scene_Type new_scene_type)
{
	Init(new_scene_type);
}

GameWorld::~GameWorld()
{
}

void GameWorld::Init(Scene_Type new_scene_type)
{
    if (scene_logic) 
    { 
        scene_logic->onExit(); 
        scene_logic.reset(); 
    }

    scene_type = new_scene_type;

    switch (new_scene_type)
    {
    case Stage_1:
    {
        scene_area = {};
        scene_center = {};
        scene_logic = std::make_unique<Dragon_Stage_SceneLogic>();
    }
        break;
    case Stage_2: 
    {
        scene_area = {};
        scene_center = {};
        scene_logic = std::make_unique<Anubis_Stage_SceneLogic>();
    }
        break;
    case Stage_3:
        break;
    case Stage_4:
        break;
    case Stage_5:
        break;
    case Stage_6:
        break;
    case Stage_7:
        break;

    case Lobby:
    case Board:
    case Test:
    case etc:
    case None:
    default:
        break;
    }

    if (scene_logic) 
    {
        scene_logic->init(particle_manager);
        scene_logic->setBoss(boss_monster);
        scene_logic->onEnter();
    }
}

void GameWorld::Load_Scene_Data(shared_ptr<GameObject> scene_obj)
{
    fixed_object_list.clear();
	FlattenGameObjectHierarchy_Filter(scene_obj, fixed_object_list);
    AssignToUniformCells();
}

void GameWorld::Update_World(float dt)
{
    if (!scene_logic) 
        return;

    UpdateContext ctx;
    {
        ctx.dt = dt;
        ctx.out_zoom_object = &zoom_object;
        ctx.stage_clear = Stage_Clear;
    }

    scene_logic->update(ctx);



}

void GameWorld::FlattenGameObjectHierarchy_Filter(std::shared_ptr<GameObject> node, std::vector<shared_ptr<GameObject>>& outList)
{
    if (!node) return;

    const std::string& name = node->Get_Name();


    if (!name.empty() && name.find("Env") == std::string::npos)
    {
        outList.push_back(node);
    }

    else if (std::find(kExcludedNames.begin(), kExcludedNames.end(), name) != kExcludedNames.end())
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

void GameWorld::Update_Particle(float elapsed_time)
{
    particle_manager.Update_Particle(elapsed_time);

}

void GameWorld::Add_Bleeding_Particle(XMFLOAT3& pos, XMFLOAT3& main_direction)
{
    Particle_Format p;
    p.area_xyz = XMFLOAT3{ 500,500,500 };
    p.lifetime = 3;
    p.main_direction = XMFLOAT3{ 0,1,0 };
    p.particle_type = Particle_Type::bleed;

    std::shared_ptr<Particle_Object> new_bleeding_particle = particle_manager.Create_Particle_Object(p);
    new_bleeding_particle->SetNeedSyncType(false);
    new_bleeding_particle->SetPosition(pos);
    new_bleeding_particle->SetLook(main_direction);

}



void GameWorld::Stage_Clear_Particle_Update(std::array<std::shared_ptr<Player>, MaxPlayer> player_list)
{
    for (int id = 0; id < MaxPlayer; ++id)
    {
        if (player_list[id])
        {
            XMFLOAT3 player_pos = player_list[id]->GetPosition();
            player_pos.y += 20.0f;
            if (party_effect[id])
            {
                party_effect[id]->SetPosition(player_pos);
            }
            else
            {
                Particle_Format p;
                p.area_xyz = XMFLOAT3{ 1000,1000,1000 }; 
                p.lifetime = 300;
                p.main_direction = XMFLOAT3{ 0,1,0 };
                p.particle_type = Particle_Type::party; 

                party_effect[id] = particle_manager.Create_Particle_Object(p);
                party_effect[id]->SetNeedSyncType(true);
                party_effect[id]->SetPosition(player_pos);
                party_effect[id]->SetLook(XMFLOAT3{ 0,1,0 });

            }
        }
    }
}

FrameParticleChanges GameWorld::Get_Particle_Sync_Data()
{
    return particle_manager.FlushFrameChanges();
}

void GameWorld::Sand_Update()
{
    shared_ptr<Particle_Object> particle_obj = particle_manager.Get_Particle_Object(1);
    UINT status = particle_obj->Get_Particle_Status();

    status += 1;
    status = status % 3;
    particle_obj->Set_Particle_Status(status);
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

void GameWorld::Set_Boss_Moster(shared_ptr<Monster> boss_ptr)
{
    boss_monster = boss_ptr;

    if (scene_logic) 
        scene_logic->setBoss(boss_monster);
}
