#include "stdafx.h"
#include "Scene.h"
#include "server.h"
#include "AnimationRegistry.h"
#include "PlayerState.h"

int Scene::active_client_num;
std::array<int, MaxPlayer> Scene::player_model_list = { -1, -1, -1, -1, -1, -1 };

Scene::Scene(Scene_Type type)
    : sceneType(type)
{
}

Scene_Type Scene::GetSceneType() const 
{
    return sceneType;
}

Scene_Type Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (Change_Scene_Trigger)
        return Scene_Type::Stage_1;
    else
        return Scene_Type::None;
}

void Scene::Update_Scene(float elapsedTime)
{

}

Effect_Sync_Data Scene::Get_Effect_Status()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    Effect_Sync_Data effect_data{};
    return effect_data;
}


//======================================================
void Lobby_Scene::Init()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (int i = 0; i < MaxPlayer; ++i)
    {
        characterReady[i] = -1;
        for (int j = 0; j < MaxPlayer; ++j)
        {
            characterSelections[i][j] = false;
        }
    }

}

void Lobby_Scene::Update_Scene(float elapsedTime)
{
}

void Lobby_Scene::Remove_Player(int id)
{
    if (id < 0 || id >= MaxPlayer)
        return;

    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (int i = 0; i < MaxPlayer; ++i)
    {
        characterSelections[i][id] = false;

        if (characterReady[i] == id)
            characterReady[i] = -1;
    }
}

bool Lobby_Scene::SelectCharacter(int clientId, int characterId, bool isReady)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (characterId < 0 || characterId >= MaxPlayer) return false;
    if (clientId < 0 || clientId >= MaxPlayer) return false;

    for (int i = 0; i < MaxPlayer; ++i)
        characterSelections[i][clientId] = false;

    characterSelections[characterId][clientId] = true;

    if (isReady)
    {
        if (characterReady[characterId] != -1 && characterReady[characterId] != clientId)
            return false;

        characterReady[characterId] = clientId;
    }
    else
    {
        if (characterReady[characterId] == clientId)
            characterReady[characterId] = -1;
    }

    return true;
}

bool Lobby_Scene::IsAllReadyAndValid()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (active_client_num == 0) return false;

    int readyCount = 0;

    for (int i = 0; i < MaxPlayer; ++i)
    {
        if (characterReady[i] != -1)
            ++readyCount;
    }

    return (readyCount == active_client_num);
}

Scene_Type Lobby_Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    Change_Scene_Trigger = IsAllReadyAndValid();

    if (Change_Scene_Trigger)
    {
        for (int characterId = 0; characterId < MaxPlayer; ++characterId) // Save - Client ID + Model Index
        {
            int clientId = characterReady[characterId];
            if (clientId != -1)
            {
                player_model_list[clientId] = characterId;
            }
        }

        return Scene_Type::Board;
    }
    else
        return Scene_Type::None;

}



//======================================================
void Board_Scene::Init()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    Change_Scene_Trigger = false;
    selected_stage_index = -1;

    if(pirate_ship)
        pirate_ship->SetPosition(0.0f, 0.0f, 0.0f);

#ifdef TEST_MODE
    if (pirate_ship)
        pirate_ship->SetPosition(0.0f, 0.0f, 1400.0f);
#endif

    for (int i = 0; i < MaxPlayer; i++)
    {
        player_keyState[i] = 0;
        stage_select_state[i] = { -1, false };
    }
}

void Board_Scene::Update_Scene(float elapsedTime)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (!pirate_ship) return;


    int fwd = 0, back = 0, left = 0, right = 0;

    for (int i = 0; i < MaxPlayer; ++i)
    {
        int modelId = Scene::player_model_list[i];
        if (modelId == -1) continue; 

        int weight = (modelId == 0) ? 2 : 1; 

        int32_t key = player_keyState[i];
        if (key & INPUT_W) fwd += weight;
        if (key & INPUT_S) back += weight;
        if (key & INPUT_A) left += weight;
        if (key & INPUT_D) right += weight;
    }

    if (fwd)
        pirate_ship->MoveForward(100.0f * fwd);
    if (back)
        pirate_ship->MoveForward(-100.0f * back);
    if (left)
        pirate_ship->Add_Rotate(-100.0f * left);
    if (right)
        pirate_ship->Add_Rotate(100.0f * right);

    pirate_ship->Animate(elapsedTime);
    pirate_ship->HandleBoundaryReflection(1500);

    Change_Scene_Trigger = IsAllReadyAndValid();
}

void Board_Scene::Remove_Player(int id)
{
    if (id < 0 || id >= MaxPlayer)
        return;

    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    player_keyState[id] = 0;
    stage_select_state[id] = { -1, false };
}


bool Board_Scene::IsAllReadyAndValid()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (active_client_num == 0) return false;

    int readyCount = 0;
    int selectedStage = -1;

    for (int i = 0; i < MaxPlayer; ++i)
    {
        const auto& [stage, is_ready] = stage_select_state[i];

        if (!is_ready)
            continue;

        if (readyCount == 0)
        {
            selectedStage = stage; 
        }
        else if (stage != selectedStage)
        {
            return false; 
        }

        ++readyCount;
    }

    if (readyCount == active_client_num)
    {
        selected_stage_index = selectedStage; 
        return true;
    }

    return false;
}


Scene_Type Board_Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);


    if (Change_Scene_Trigger)
    {
        if (selected_stage_index == 0)        return Scene_Type::Stage_1;
        else if (selected_stage_index == 1)        return Scene_Type::Stage_2;
        else if (selected_stage_index == 2)        return Scene_Type::Stage_3;
        else if (selected_stage_index == 3)        return Scene_Type::Stage_4;
        else if (selected_stage_index == 4)        return Scene_Type::Stage_5;
        else if (selected_stage_index == 5)        return Scene_Type::Stage_6;
        else if (selected_stage_index == 6)        return Scene_Type::Stage_7;
        else return Scene_Type::None;
    }

    return Scene_Type::None;
}

void Board_Scene::Update_KeyState(int Client_ID, int32_t keyState)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (Client_ID >= 0 && Client_ID < MaxPlayer)
        player_keyState[Client_ID] = keyState; 
    else
        cout << "Error - [Update_KeyState]: Wrong_Index \n";
    
}

void Board_Scene::Select_State(int Client_ID, pair<int, bool> select_state)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (Client_ID >= 0 && Client_ID < MaxPlayer)
    {
        stage_select_state[Client_ID] = select_state; 
    }
    else
        cout << "Error - [Select_State]: Wrong_Index \n";

}




XMFLOAT3 Board_Scene::Get_PirateShip_Position() const
{
    if (pirate_ship)
        return pirate_ship->GetPosition();
    return XMFLOAT3(0.0f, 0.0f, 0.0f); 
}

XMFLOAT3 Board_Scene::Get_PirateShip_Look() const
{
    if (pirate_ship)
        return pirate_ship->GetLook();

    return XMFLOAT3(0.0f, 0.0f, 1.0f); 
}

//======================================================

Stage_Scene::Stage_Scene(Scene_Type scene_type) : Scene (scene_type)
{
    game_world = make_shared<GameWorld>(scene_type);
}


void Stage_Scene::Init()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (shared_ptr<Player> player_ptr : player_list)
        player_ptr.reset();

    Monster_List.clear();
    id2idx.clear();

    std::fill(check_clear_state.begin(), check_clear_state.end(), false);

    if (bStageClear)
        return;

    //======================================================

    std::shared_ptr<GameObject> monster_hierarchy_list = scene_obj->FindFrame("Monsters");
    std::shared_ptr<GameObject> player_hierarchy_list = scene_obj->FindFrame("Players");

    monster_init_spawn_frame_list.clear();
    player_init_spawn_frame_list.clear();
    game_world->Set_Boss_Moster(NULL);

    GameObject::FlattenGameObjectHierarchy(monster_hierarchy_list, monster_init_spawn_frame_list);
    GameObject::FlattenGameObjectHierarchy(player_hierarchy_list, player_init_spawn_frame_list);

    scene_obj->UpdateTransform(NULL);
    
    if (monster_init_spawn_frame_list.size())
        SpawnMonster_By_Scene_Data();
}

void Stage_Scene::Update_Scene(float elapsedTime)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (game_world->Get_Boss_Monster() == NULL && Boss_Monster != NULL)
        game_world->Set_Boss_Moster(Boss_Monster);


    for (shared_ptr<Player> player_ptr : player_list)
    {
        if (player_ptr)
        {
            auto obbList = game_world->Get_Cell_OBBs(player_ptr->GetPosition());
            //game_world.Update_Collision(player_ptr);
            player_ptr->update(elapsedTime);
            player_ptr->update_collision(elapsedTime, obbList);

            if (player_ptr->Weapon_ptr.empty()) continue;
            player_ptr->UpdateTransform();
            for (auto& w : player_ptr->Weapon_ptr) {
                if (!w->CanCollide()) continue;
                w->UpdateWorldOBB();
                auto worldWeaponOBB = w->Get_Collider_OBB();
                //cout << worldWeaponOBB->Center.x << ", " << worldWeaponOBB->Center.y << ", " << worldWeaponOBB->Center.z << "\n";
                for (auto m : Monster_List) {
                    if (!m) continue;
                    if (!m->CanCollide()) continue;
                    if (m->IsInvincible()) continue;

                    if (Vector3::Distance(player_ptr->GetPosition(), m->GetPosition()) > 50.0f) {
                        continue;
                    }

                    m->UpdateTransform();
                    auto monsterOBB = m->Get_Collider_OBB();
                    if (!monsterOBB) continue;

                    BoundingOrientedBox worldMonsterOBB;
                    monsterOBB->Transform(worldMonsterOBB,
                        XMLoadFloat4x4(&m->m_xmf4x4World));

                    if (worldWeaponOBB->Intersects(worldMonsterOBB)) {
                        std::cout << "Collision detected! Player Weapon and Monster ID" << m->GetID() << "\n";
                        XMFLOAT3 toPlayer = Vector3::Subtract(player_ptr->GetPosition(), m->GetPosition());
                        toPlayer.y = 0.0f;
                        if (Vector3::LengthSquared(toPlayer) > 0.0001f) {
                            toPlayer = Vector3::Normalize(toPlayer);
                            m->SetLook(toPlayer);
                        }

                        MonsterHitInfo data;

                        data.monsterID = m->GetID();

                        m->HitDamage(30.0f);
                        float hp = m->GetHP();
                        if (hp <= 0.0f) {
                            m->GetStateMachine()->ChangeState(std::make_unique<DeadState>());
                            data.hitCmd = false;
                        }
                        else {
                            m->GetStateMachine()->ChangeState(std::make_unique<GetHitState>());
                            data.hitCmd = true;
                        }
                        QueueDamageCommand(data);
                    }
                }
            }

            /*if (!player_ptr->Weapon_ptr->CanCollide()) continue;

            player_ptr->UpdateTransform();
            player_ptr->Weapon_ptr->UpdateWorldOBB();
            auto worldWeaponOBB = *player_ptr->Weapon_ptr->Get_Collider_OBB();
            for (auto m : Monster_List) {
                if (!m) continue;
                if (!m->CanCollide()) continue;
                if (m->IsInvincible()) continue;

                if (Vector3::Distance(player_ptr->GetPosition(), m->GetPosition()) > 50.0f) {
                    continue;
                }

                m->UpdateTransform();
                auto monsterOBB = m->Get_Collider_OBB();
                if (!monsterOBB) continue;

                BoundingOrientedBox worldMonsterOBB;
                monsterOBB->Transform(worldMonsterOBB,
                    XMLoadFloat4x4(&m->m_xmf4x4World));

                if (worldWeaponOBB.Intersects(worldMonsterOBB)) {
                    std::cout << "Collision detected! Player Weapon and Monster ID" << m->GetID() << "\n";
                    XMFLOAT3 toPlayer = Vector3::Subtract(player_ptr->GetPosition(), m->GetPosition());
                    toPlayer.y = 0.0f;
                    if (Vector3::LengthSquared(toPlayer) > 0.0001f) {
                        toPlayer = Vector3::Normalize(toPlayer);
                        m->SetLook(toPlayer);
                    }

                    MonsterHitInfo data;
                    
                    data.monsterID = m->GetID();
                    
                    m->HitDamage(30.0f);
                    float hp = m->GetHP();
                    if (hp <= 0.0f) {
                        m->GetStateMachine()->ChangeState(std::make_unique<DeadState>());
                        data.hitCmd = false;
                    }
                    else {
                        m->GetStateMachine()->ChangeState(std::make_unique<GetHitState>());
                        data.hitCmd = true;
                    }
                    QueueDamageCommand(data);
                }
            }*/
        }
    }
    for (auto m : Monster_List) 
    {
        if (!m) continue;
        auto obbList = game_world->Get_Cell_OBBs(m->GetPosition());
        m->update(elapsedTime);
        m->update_collision(elapsedTime, obbList);
       
        if (m->bDead) {
            DespawnMonster(m->GetID());
            continue;
        }
        if (m->bHittingCmd) {
            MonsterHitInfo data;
            data.monsterID = m->GetID();
            data.hitCmd = false;
            QueueDamageCommand(data);
            m->bHittingCmd = false;
        }
        if (m->Weapon_ptr.empty()) continue;
        m->UpdateTransform();
        for (auto& w : m->Weapon_ptr) {
            if (!w->CanCollide()) continue;
            w->UpdateWorldOBB();
            auto worldWeaponOBB = w->Get_Collider_OBB();
            cout << "worldWeaponOBB : " << worldWeaponOBB ->Center.x << ", " << worldWeaponOBB->Center.y << ", " << worldWeaponOBB->Center.z << "\n";
            for (std::shared_ptr<Player> player_ptr : player_list) {
                if (!player_ptr) continue;

                if (player_ptr->bDead) continue;
                if (!player_ptr->CanCollide()) continue;
                if (player_ptr->IsInvincible()) continue;

                if (w->BreathObject) {
                    if (Vector3::Distance(player_ptr->GetPosition(), m->GetPosition()) > 200.0f) {
                        continue;
                    }
                }
                else {
                    if (Vector3::Distance(player_ptr->GetPosition(), m->GetPosition()) > 50.0f) {
                        continue;
                    }
                }

                player_ptr->UpdateTransform();
                auto playerOBB = player_ptr->Get_Collider_OBB();
                if (!playerOBB) continue;

                player_ptr->UpdateWorldOBB();
                auto worldPlayerOBB = *player_ptr->Get_Collider_OBB();

                if (worldWeaponOBB->Intersects(worldPlayerOBB)) {
                    std::cout << "Collision detected! Monster Weapon and Player ID " << player_ptr->GetID() << "\n";
                    XMFLOAT3 toMonter = Vector3::Subtract(m->GetPosition(), player_ptr->GetPosition());
                    toMonter.y = 0.0f;
                    if (Vector3::LengthSquared(toMonter) > 0.0001f) {
                        toMonter = Vector3::Normalize(toMonter);
                        player_ptr->SetLook(toMonter);
                    }
                    float damage;

                    if (w->BreathObject)
                    {
                        damage = 10.0f;
                        player_ptr->BreathHit = true;
                    }
                    else // Normal Hit
                    {
                        damage = 30.0f;

                        XMVECTOR weaponCenter = XMLoadFloat3(&worldWeaponOBB->Center);
                        XMVECTOR playerCenter = XMLoadFloat3(&worldPlayerOBB.Center);

                        XMVECTOR direction = XMVector3Normalize(playerCenter - weaponCenter);

                        XMFLOAT3 contactPos;
                        XMStoreFloat3(&contactPos, XMVectorLerp(weaponCenter, playerCenter, 0.5f));

                        XMFLOAT3 contactDir;
                        XMStoreFloat3(&contactDir, direction);

                        game_world->Add_Bleeding_Particle(contactPos, contactDir);
                    }

                    player_ptr->HitDamage(damage);

                    float hp = player_ptr->GetHP();

                    if (hp <= 0.0f)
                        player_ptr->GetStateMachine()->ChangeState(std::make_unique<PlayerDeadState>());
                    else
                        player_ptr->GetStateMachine()->ChangeState(std::make_unique<PlayerGetHitState>());

                }
            }
        }
        /*if (!m->Weapon_ptr->CanCollide()) continue;
        m->UpdateTransform();
        m->Weapon_ptr->UpdateWorldOBB();
        auto worldWeaponOBB = *m->Weapon_ptr->Get_Collider_OBB();*/
        //for (std::shared_ptr<Player> player_ptr : player_list) {
        //    if (!player_ptr) continue;

        //    if (player_ptr->bDead) continue;
        //    if (!player_ptr->CanCollide()) continue;
        //    if (player_ptr->IsInvincible()) continue;

        //    if (m->Weapon_ptr->BreathObject) {
        //        if (Vector3::Distance(player_ptr->GetPosition(), m->GetPosition()) > 200.0f) {
        //            continue;
        //        }
        //    }
        //    else {
        //        if (Vector3::Distance(player_ptr->GetPosition(), m->GetPosition()) > 50.0f) {
        //            continue;
        //        }
        //    }

        //    player_ptr->UpdateTransform();
        //    auto playerOBB = player_ptr->Get_Collider_OBB();
        //    if (!playerOBB) continue;

        //    player_ptr->UpdateWorldOBB();
        //    auto worldPlayerOBB = *player_ptr->Get_Collider_OBB();

        //    if (worldWeaponOBB.Intersects(worldPlayerOBB)) {
        //        std::cout << "Collision detected! Monster Weapon and Player ID " << player_ptr->GetID() << "\n";

        //        float damage;

        //        if (m->Weapon_ptr->BreathObject)
        //        {
        //            damage = 10.0f;
        //            player_ptr->BreathHit = true;
        //        }
        //        else // Normal Hit
        //        {
        //            damage = 30.0f;

        //            XMVECTOR weaponCenter = XMLoadFloat3(&worldWeaponOBB.Center);
        //            XMVECTOR playerCenter = XMLoadFloat3(&worldPlayerOBB.Center);

        //            XMVECTOR direction = XMVector3Normalize(playerCenter - weaponCenter);

        //            XMFLOAT3 contactPos;
        //            XMStoreFloat3(&contactPos, XMVectorLerp(weaponCenter, playerCenter, 0.5f));

        //            XMFLOAT3 contactDir;
        //            XMStoreFloat3(&contactDir, direction);

        //            game_world->Add_Bleeding_Particle(contactPos, contactDir);
        //        }

        //        player_ptr->HitDamage(damage);

        //        float hp = player_ptr->GetHP();

        //        if (hp <= 0.0f)
        //            player_ptr->GetStateMachine()->ChangeState(std::make_unique<PlayerDeadState>());
        //        else
        //            player_ptr->GetStateMachine()->ChangeState(std::make_unique<PlayerGetHitState>());

        //    }
        //}
    }
    for (auto m : Monster_List) 
    {
        XMFLOAT3 pos = m->GetPosition();
        pos.x = std::clamp(pos.x, 0.0f, g_mapSize.x);
        pos.z = std::clamp(pos.z, 0.0f, g_mapSize.y);
        m->SetPosition(pos);
    }


    game_world->Update_World(elapsedTime);
    game_world->Update_Particle(elapsedTime);

    if (Monster_List.size() == 0)
        bStageClear = true;

    if (bStageClear)
        game_world->Stage_Clear_Particle_Update(player_list);

}

void Stage_Scene::Update_Clear_State(int playerid, bool state)
{
    if (playerid < 0 || playerid >= MaxPlayer)
        return;

    if (bStageClear)
        check_clear_state[playerid] = state;
}

bool Stage_Scene::Check_Clear_Scene()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (active_client_num == 0)
        return false;

    int readyCount = 0;

    for (int i = 0; i < MaxPlayer; ++i)
    {
        bool change_ready = check_clear_state[i];

        if (!change_ready)
            continue;


        ++readyCount;
    }

    if (readyCount == active_client_num)
        return true;

    return false;
}

Scene_Type Stage_Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (bStageClear)
        Change_Scene_Trigger = Check_Clear_Scene();

    if (Change_Scene_Trigger)
        return Scene_Type::Board;
    else
        return Scene_Type::None;
}


Effect_Sync_Data Stage_Scene::Get_Effect_Status()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    Effect_Sync_Data effect_data;
    

    int Player_ID = 0;
    for (std::shared_ptr<Player> player_ptr : player_list)
    {
        if (player_ptr)
        {
            effect_data.mosaic_value[Player_ID] = player_ptr->mosaic_value;
        }

        ++Player_ID;
    }


    zoomObject = game_world->Get_ZoomObject();
    if (zoomObject != NULL)
    {
        effect_data.zoom_blur_active = true;
        effect_data.zoom_w_position = zoomObject->GetPosition();
    }
    else
    {
        effect_data.zoom_blur_active = false;
        effect_data.zoom_w_position = XMFLOAT3(0, 0, 0);
    }

    int total_monster_count = static_cast<int>(monster_init_spawn_frame_list.size() - 1);
    int normal_monster_count = total_monster_count - 1; 
    int alive_monster_count = static_cast<int>(Monster_List.size());

    int defeated_monsters = normal_monster_count - alive_monster_count;

    int clear_value_1 = 50;
    if (game_world->Get_Boss_Monster())
        clear_value_1 = 50 * (1 - game_world->Get_Boss_Monster()->Get_Active());

    int clear_value_2 = 50 * defeated_monsters / normal_monster_count;

    int total_value = clear_value_1 + clear_value_2;

    if (total_value < 30)
    {
        effect_data.monster_x_ray = false;
        effect_data.fog_trigger = true;
        effect_data.fogStart = 1.0f;
        effect_data.fogEnd = 500.0f;
        effect_data.fogDensity = 0.5f;
    }
    else if (total_value < 50)
    {
        effect_data.monster_x_ray = false;
        effect_data.fog_trigger = true;
        effect_data.fogStart = 1.0f;
        effect_data.fogEnd = 500.0f;
        effect_data.fogDensity = 3.0f;
    }
    else if (total_value < 70)
    {
        effect_data.monster_x_ray = true;
        effect_data.fog_trigger = true;
        effect_data.fogStart = 1.0f;
        effect_data.fogEnd = 1500.0f;
        effect_data.fogDensity = 3.0f;
    }
    else if (total_value >= 100)
    {
        effect_data.monster_x_ray = false;
        effect_data.fog_trigger = false;
    }


    if (bMonster_x_ray_State)
        effect_data.monster_x_ray = true;

    if (bFog_State || bStageClear)
        effect_data.fog_trigger = false;

    effect_data.fog_trigger = false;

    return effect_data;
}


const std::array<std::shared_ptr<Player>, MaxPlayer> Stage_Scene::Get_PlayerList() const
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    return player_list;
}

std::shared_ptr<Player> Stage_Scene::Get_Player(int id)
{
    if (id < 0 || id >= MaxPlayer)
        return nullptr;

    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    return player_list[id];
}

void Stage_Scene::Add_Player(int id)
{
    if (id < 0 || id >= MaxPlayer)
        return;

    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    
    player_list[id] = make_shared<Player>(player_model_list[id]);
    player_list[id]->Set_Child(player_list[id]->m_pRootModel);
    XMFLOAT3 player_pos = player_init_spawn_frame_list[id + 1]->GetPosition();

    player_list[id]->SetPosition(player_pos);
    player_list[id]->SetupWeaponCollider();
    if (player_list[id]->Get_Model_ID() == 2) {
        auto weapon = player_list[id]->Weapon_ptr;

        for (auto w : weapon) {
            w->CustomOBBScale = XMFLOAT3(0.2f, 0.2f, 0.3f);
        }
    }
    player_list[id]->UpdateTransform();
    player_list[id]->m_pOwnerScene = this;
    player_list[id]->Client_ID = id;
}

void Stage_Scene::Remove_Player(int id)
{
    if (id < 0 || id >= MaxPlayer)
        return;

    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    player_list[id].reset();
}

void Stage_Scene::update_player_keyinput(int id, uint32_t keystate)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (id < 0 || id >= MaxPlayer || !player_list[id])
        return;

    player_list[id]->key_input(keystate);
}


void Stage_Scene::update_player_LookV(int id, XMFLOAT3 new_lookV)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (id < 0 || id >= MaxPlayer || !player_list[id])
        return;

    player_list[id]->SetLook(new_lookV);
}

void Stage_Scene::update_player_State(int clientId, uint32_t inputFlags, const XMFLOAT3& position, const XMFLOAT3& lookDirection, bool stateChanged, int stateNum)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (clientId < 0 || clientId >= MaxPlayer || !player_list[clientId])
        return;

    //player_list[clientId]->SetPosition(position);
    player_list[clientId]->SetLook(lookDirection);
    
    player_list[clientId]->key_input(inputFlags);
    //player_list[clientId]->currkeyState = inputFlags;
}

void Stage_Scene::SpawnMonster_By_Scene_Data()
{
    return;

    int index{}, m_id{};
    std::shared_ptr<Monster> moster_ptr = NULL;

    for (std::shared_ptr<GameObject>monster_frame : monster_init_spawn_frame_list)
    {

        string name = monster_frame->Get_Name();
        XMFLOAT3 pos = monster_frame->GetPosition();
        pos.y = 0;
        if (name.find("Fishman") != string::npos)
        {
            //m_id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Fishman), index++);
            m_id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Creature1), index++);
            moster_ptr = SpawnMonster(m_id, XMFLOAT3(pos), 100);
        }
        else if (name.find("Anubis") != string::npos)
        {
//            m_id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Anubis), index++);
//            moster_ptr = SpawnMonster(m_id, XMFLOAT3(pos), 100);

//            if (!Boss_Monster)
//                Boss_Monster = moster_ptr;
        }
        else if (name.find("Dragon") != string::npos)
        {
//            m_id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Dragon), index++);
//            moster_ptr = SpawnMonster(m_id, XMFLOAT3(pos), 100);

//            if (!Boss_Monster)
//                Boss_Monster = moster_ptr;
        }
        else if (name.find("Monster") != string::npos)
            continue;
        else
            continue;
    }
}

std::shared_ptr<Monster> Stage_Scene::SpawnMonster(int id, const XMFLOAT3& pos, int hp)
{
    if (id2idx.find(id) != id2idx.end())
        return NULL;                            

    int mType = GET_MONSTER_TYPE(id);
    std::shared_ptr<Monster> m;

    if (mType == static_cast<int>(Monster_Type::Fishman)) {
        m = std::make_shared<Fishman>(1);
    }
    else if (mType == static_cast<int>(Monster_Type::Creature1)) {
        m = std::make_shared<Creature1>(1);
    }
    else if (mType == static_cast<int>(Monster_Type::Anubis)) {
        m = std::make_shared<Anubis>(1);
    }
    else if (mType == static_cast<int>(Monster_Type::Dragon)) {
        m = std::make_shared<Dragon>(1);
    }
    else if (mType == static_cast<int>(Monster_Type::Gargoyle)) {
        m = std::make_shared<Gargoyle>(1);
    }
    else if (mType == static_cast<int>(Monster_Type::ETC)) {
        m = std::make_shared<TestPlayer>(1);
    }
    else {
        return NULL;
    }

    m->Set_Child(m->m_pRootModel);
    m->SetupWeaponCollider();
    m->SetID(id);
    m->SetPosition(pos);
    m->SetPlayerListPtr(&player_list);

    id2idx[id] = Monster_List.size();       
    Monster_List.emplace_back(std::move(m));
    return Monster_List.back();
}

void Stage_Scene::DespawnMonster(int id)
{
    std::lock_guard<std::recursive_mutex> lock(GetSceneMutex());

    auto it = id2idx.find(id);
    if (it == id2idx.end()) {
        std::cout << "map find fail : " << id << std::endl;
        return;
    }

    size_t idx = it->second;              
    size_t last = Monster_List.size() - 1;  

    if (idx != last) {
        std::swap(Monster_List[idx], Monster_List[last]);
        id2idx[Monster_List[idx]->GetID()] = idx; 
    }
    Monster_List.pop_back();
    id2idx.erase(it);

    QueueDespawnCommand(id);
}

std::shared_ptr<Monster> Stage_Scene::GetMonster(int id)
{
    auto it = id2idx.find(id);
    return (it == id2idx.end()) ? nullptr : Monster_List[it->second];
}


const FrameParticleChanges Stage_Scene::Get_Particle_Sync_Data()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    return game_world->Get_Particle_Sync_Data();
}

std::vector<int> Stage_Scene::FlushDespawnQueue()
{
    std::vector<int> temp = monster_despawn_queue;
    monster_despawn_queue.clear();
    return temp;
}

std::vector<MonsterHitInfo> Stage_Scene::FlushDamageQueue()
{
    std::vector<MonsterHitInfo> temp = monster_damage_queue;
    monster_damage_queue.clear();
    return temp;
}

std::vector<StateChangeInfo> Stage_Scene::FlushStateChangeQueue()
{
    std::vector<StateChangeInfo> temp = player_state_change_queue;
    player_state_change_queue.clear();
    return temp;
}

void Stage_Scene::server_DespawnMonster()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    int delete_id = Monster_List.back()->GetID();
    DespawnMonster(delete_id);
}

void Stage_Scene::server_DespawnMonster_For_Clear()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    std::vector<int> ids_to_delete;
    for (auto& m_ptr : Monster_List)
        ids_to_delete.push_back(m_ptr->GetID());

    for (int id : ids_to_delete)
        DespawnMonster(id); 
}

void Stage_Scene::server_Fog_Control()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    bFog_State = !bFog_State;
}

void Stage_Scene::server_X_Ray_Control()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    bMonster_x_ray_State = !bMonster_x_ray_State;
}

void Stage_Scene::server_bleeding()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    XMFLOAT3 pos = player_list[0]->GetPosition();
    pos.y += 30.0f;
    XMFLOAT3 dir = player_list[0]->GetLook();
    game_world->Add_Bleeding_Particle(pos, dir);
}

void Stage_Scene::server_Sand_Control()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    game_world->Sand_Update();
}

void Stage_Scene::server_Mosaic_Control()
{
    for (std::shared_ptr<Player> player_ptr : player_list)
    {
        if (player_ptr)
        {
            player_ptr->mosaic_value += 1;
        }
    }
}
//=========================================================


Stage_1_Scene::Stage_1_Scene() : Stage_Scene(Stage_1)
{

    scene_obj = std::make_shared<GameObject>();
    scene_obj = GameObject::Load_Scene("Scene/Scene_File_7/map1.bin");
	g_mapSize = XMFLOAT2(3840.0f, 2816.0f);
    scene_obj->SetPosition(1250.0f, -35.0f, -1200.0f);
    scene_obj->SetScale(10, 10, 10, true);
    scene_obj->UpdateTransform(NULL);
    game_world->Load_Scene_Data(scene_obj);

    Init();
}


void Stage_1_Scene::Init()
{
    Stage_Scene::Init();

    if (!bStageClear)
    {
        int id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Dragon), 1);
        Boss_Monster = SpawnMonster(id, XMFLOAT3(583.0f, 0.0f, 1332.0f), 100);
    }
}

//=========================================================

Stage_2_Scene::Stage_2_Scene() : Stage_Scene(Stage_2)
{
    scene_obj = std::make_shared<GameObject>();
    scene_obj = GameObject::Load_Scene("Scene/Scene_File_7/map2.bin");
    g_mapSize = XMFLOAT2(3072.0f, 4352.0f);
    scene_obj->SetPosition(2000.0f, 35.0f, 2000.0f);
    scene_obj->SetScale(10, 10, 10, true);
    scene_obj->UpdateTransform(NULL);
    game_world->Load_Scene_Data(scene_obj);

    Init();
}


void Stage_2_Scene::Init()
{
    Stage_Scene::Init();

    if (!bStageClear)
    {
        int id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Anubis), 1);
        Boss_Monster = SpawnMonster(id, XMFLOAT3(1864.0f, 0.0f, 1990.0f), 100);
    }
}

//=========================================================

Stage_3_Scene::Stage_3_Scene() : Stage_Scene(Stage_3)
{
    scene_obj = std::make_shared<GameObject>();
    scene_obj = GameObject::Load_Scene("Scene/Scene_File_7/map3.bin");
    g_mapSize = XMFLOAT2(3840.0f, 2816.0f);
    scene_obj->SetPosition(2000.0f, 0.0f, 2000.0f);
    scene_obj->SetScale(10, 10, 10, true);
    scene_obj->UpdateTransform(NULL);
    game_world->Load_Scene_Data(scene_obj);

    Init();
}


void Stage_3_Scene::Init()
{
    Stage_Scene::Init();

    if (!bStageClear)
    {
        int id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Dragon), 1);
        Boss_Monster = SpawnMonster(id, XMFLOAT3(583.0f, 0.0f, 1332.0f), 100);
    }
}

//=========================================================

Stage_4_Scene::Stage_4_Scene() : Stage_Scene(Stage_4)
{
    scene_obj = std::make_shared<GameObject>();
    scene_obj = GameObject::Load_Scene("Scene/Scene_File_7/map4.bin");
    g_mapSize = XMFLOAT2(3072.0f, 4352.0f);
    scene_obj->SetPosition(2000.0f, 35.0f, 2000.0f);
    scene_obj->SetScale(10, 10, 10, true);
    scene_obj->UpdateTransform(NULL);
    game_world->Load_Scene_Data(scene_obj);

    Init();
}


void Stage_4_Scene::Init()
{
    Stage_Scene::Init();

    if (!bStageClear)
    {
        int id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Gargoyle), 1);
        Boss_Monster = SpawnMonster(id, XMFLOAT3(1864.0f, 0.0f, 1990.0f), 100);
    }


}

//=========================================================

Stage_5_Scene::Stage_5_Scene() : Stage_Scene(Stage_5)
{

    scene_obj = std::make_shared<GameObject>();
    scene_obj = GameObject::Load_Scene("Scene/Scene_File_7/map1.bin");
    g_mapSize = XMFLOAT2(3840.0f, 2816.0f);
    scene_obj->SetPosition(1250.0f, -35.0f, -1200.0f);
    scene_obj->SetScale(10, 10, 10, true);
    scene_obj->UpdateTransform(NULL);
    game_world->Load_Scene_Data(scene_obj);

    Init();
}


void Stage_5_Scene::Init()
{
    Stage_Scene::Init();

    if (!bStageClear)
    {
        int id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Dragon), 1);
        Boss_Monster = SpawnMonster(id, XMFLOAT3(583.0f, 0.0f, 1332.0f), 100);
    }
}

//=========================================================

Stage_6_Scene::Stage_6_Scene() : Stage_Scene(Stage_6)
{

    scene_obj = std::make_shared<GameObject>();
    scene_obj = GameObject::Load_Scene("Scene/Scene_File_7/map2.bin");
    g_mapSize = XMFLOAT2(3072.0f, 4352.0f);
    scene_obj->SetPosition(2000.0f, 35.0f, 2000.0f);
    scene_obj->SetScale(10, 10, 10, true);
    scene_obj->UpdateTransform(NULL);
    game_world->Load_Scene_Data(scene_obj);

    Init();
}


void Stage_6_Scene::Init()
{
    Stage_Scene::Init();

    if (!bStageClear)
    {
        int id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Anubis), 1);
        Boss_Monster = SpawnMonster(id, XMFLOAT3(1864.0f, 0.0f, 1990.0f), 100);
    }
}

//=========================================================

Stage_7_Scene::Stage_7_Scene() : Stage_Scene(Stage_7)
{
    scene_obj = std::make_shared<GameObject>();
    scene_obj = GameObject::Load_Scene("Scene/Scene_File_7/map1.bin");
    g_mapSize = XMFLOAT2(3840.0f, 2816.0f);
    scene_obj->SetPosition(1250.0f, -35.0f, -1200.0f);
    scene_obj->SetScale(10, 10, 10, true);
    scene_obj->UpdateTransform(NULL);
    game_world->Load_Scene_Data(scene_obj);

    Init();
}


void Stage_7_Scene::Init()
{
    Stage_Scene::Init();

    if (!bStageClear)
    {
        int id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Dragon), 1);
        Boss_Monster = SpawnMonster(id, XMFLOAT3(583.0f, 0.0f, 1332.0f), 100);
    }
}