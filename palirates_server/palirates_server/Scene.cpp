#include "stdafx.h"
#include "Scene.h"
#include "server.h"

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
        // 다른 누군가가 이미 Ready 한 상태면 실패
        if (characterReady[characterId] != -1 && characterReady[characterId] != clientId)
            return false;

        // Ready 상태 등록
        characterReady[characterId] = clientId;
    }
    else
    {
        // Ready 해제는 자기 자신인 경우만 해제
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

    if(pirate_ship)
        pirate_ship->SetPosition(0.0f, 0.0f, 1000.0f);

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

        int weight = (modelId == 0) ? 2 : 1;  // 모델 0번이면 영향력 2배

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

    // 씬 전환 체크
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
            selectedStage = stage; // 기준 stage 설정
        }
        else if (stage != selectedStage)
        {
            return false; // 서로 다른 stage 선택
        }

        ++readyCount;
    }

    return (readyCount == active_client_num);
}


Scene_Type Board_Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);


    if (Change_Scene_Trigger)
        return Scene_Type::Stage_1;

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
void Stage_Scene::Init()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (shared_ptr<Player> player_ptr : player_list)
        player_ptr.reset();

    int id;
    for (int i = 0; i < 3; ++i) {
        if (i % 3 == 0)
            id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Fishman), i);
        else if (i % 3 == 1)
            id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Fishman), i);
        else if (i % 3 == 2)
            id = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Dragon), i);
        else continue;
        SpawnMonster(id, XMFLOAT3(1500 + i * 10, 0, 700), 100);
        std::cout << "몬스터 스폰 " << id << std::endl;
    }
}

void Stage_Scene::Update_Scene(float elapsedTime)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (auto m : Monster_List) {
        auto con = m->GetSkinnedAnimationController();
        if (con) {
            con->AdvanceTime(elapsedTime, m.get());
            if (m->GetStateMachine())
                m->GetStateMachine()->update(elapsedTime);
        }
        else {
            //std::cout << "con 없음" << std::endl;
        }
    }

    // === 테스트용 스폰/디스폰 타이머 ===
    static float spawnTimer = 0.0f;
    static float despawnTimer = 0.0f;
    static float spawnInterval = 1.0f;
    static float despawnInterval = 1.0f;
    static int nextIndex = 12;

    spawnTimer += elapsedTime;
    despawnTimer += elapsedTime;

    // === 랜덤 스폰 (50% 확률) ===
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.0f;

        float chance = static_cast<float>(rand()) / RAND_MAX;
        if (chance < 0.5f) {
            int newID = 0;
            if (chance < 0.2f)
                newID = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Fishman), nextIndex++);
            else if (chance < 0.4f)
                newID = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Fishman), nextIndex++);
            else if (chance < 0.5f)
                newID = ENCODE_MONSTER_ID(static_cast<int>(Monster_Type::Dragon), nextIndex++);
            float randX = 1400.f + static_cast<float>(rand() % 100);
            float randZ = 700.f + static_cast<float>(rand() % 100);
            SpawnMonster(newID, { randX, 0.f, randZ }, 100);
            std::cout << "[테스트] 몬스터 스폰됨: " << newID << std::endl;
        }
    }

    // === 랜덤 디스폰 (30% 확률) ===
    if (despawnTimer >= despawnInterval) {
        despawnTimer = 0.0f;

        float chance = static_cast<float>(rand()) / RAND_MAX;
        if (!Monster_List.empty() && chance < 0.5f) {
            int idx = rand() % Monster_List.size();
            int id = Monster_List[idx]->GetID();
            DespawnMonster(id);
            std::cout << "[테스트] 몬스터 삭제됨: " << id << std::endl;
        }
    }
}

Scene_Type Stage_Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (Change_Scene_Trigger)
        return Scene_Type::Stage_1;
    else
        return Scene_Type::None;

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

void Stage_Scene::update_player_State(int clientId, uint32_t inputFlags, const XMFLOAT3& position, const XMFLOAT3& lookDirection, const std::vector<Animation_Sync>& tracks, bool stateChanged)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (clientId < 0 || clientId >= MaxPlayer || !player_list[clientId])
        return;

    player_list[clientId]->SetPosition(position);
    player_list[clientId]->SetLook(lookDirection);
    
    //    player_list[clientId]->key_input(inputFlags);

    if (!tracks.empty())
    {
        player_list[clientId]->SetTrackInfoList(tracks);
        player_list[clientId]->SetStateChanged(stateChanged);
    }
}

void Stage_Scene::SpawnMonster(int id, const XMFLOAT3& pos, int hp, bool bIsRun)
{
    if (id2idx.find(id) != id2idx.end())
        return;                            

    int mType = GET_MONSTER_TYPE(id);
    std::shared_ptr<Monster> m;

    if (mType == static_cast<int>(Monster_Type::Fishman)) {
        m = std::make_shared<Fishman>(1);
    }
    else if (mType == static_cast<int>(Monster_Type::Anubis)) {
        m = std::make_shared<Anubis>(1);
    }
    else if (mType == static_cast<int>(Monster_Type::Dragon)) {
        m = std::make_shared<Dragon>(1);
    }
    else {
        return;
    }
    m->SetID(id);
    m->SetPosition(pos);

    id2idx[id] = Monster_List.size();       
    Monster_List.emplace_back(std::move(m));

    if (bIsRun)
        Server::Get()->BroadcastMonsterSpawn(GetSceneType(), id, pos, hp);
}

void Stage_Scene::DespawnMonster(int id)
{
    std::lock_guard<std::recursive_mutex> lock(GetSceneMutex());

    auto it = id2idx.find(id);
    if (it == id2idx.end()) return;

    size_t idx = it->second;              
    size_t last = Monster_List.size() - 1;  

    if (idx != last) {
        std::swap(Monster_List[idx], Monster_List[last]);
        id2idx[Monster_List[idx]->GetID()] = idx; 
    }
    Monster_List.pop_back();
    id2idx.erase(it);

    QueueDespawnCommand(id);
    //Server::Get()->BroadcastMonsterDespawn(GetSceneType(), id);
}

std::shared_ptr<Monster> Stage_Scene::GetMonster(int id)
{
    auto it = id2idx.find(id);
    return (it == id2idx.end()) ? nullptr : Monster_List[it->second];
}

std::vector<int> Stage_Scene::FlushDespawnQueue()
{
    std::vector<int> temp = monster_despawn_queue;
    monster_despawn_queue.clear();
    return temp;
}