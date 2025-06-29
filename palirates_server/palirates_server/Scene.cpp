#include "stdafx.h"
#include "Scene.h"

int Scene::active_client_num;

Scene::Scene(Scene_Type type)
    : sceneType(type)
{
}

const std::unordered_map<int, std::shared_ptr<Player>>& Scene::getPlayers() const
{
    return player_map;
}

std::shared_ptr<Player> Scene::getPlayer(int id)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    auto it = player_map.find(id);
    if (it != player_map.end())
    {
        return it->second;
    }
    return nullptr;
}

void Scene::addPlayer(int id, std::shared_ptr<Player> player)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    player_map[id] = player;
}

void Scene::removePlayer(int id)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    player_map.erase(id);
}

void Scene::update_player_keyinput(int id, uint32_t keystate)
{
    auto player = getPlayer(id);
    if (!player) return;
    player->key_input(keystate);
}

void Scene::update_player_Position()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    for (auto& [id, player] : player_map)
    {
        player->update();
    }
}

void Scene::update_player_LookV(int id, XMFLOAT3 new_lookV)
{
    auto player = getPlayer(id);
    if (!player) return;
    player->SetLook(new_lookV);
}

void Scene::updatePlayerPosition(int id, float x, float y, float z, float lookX, float lookY, float lookZ, Player_State state)
{
    auto player = getPlayer(id);
    if (!player) return;
    player->SetPosition(x, y, z);
    player->SetLook({ lookX, lookY, lookZ });
    player->SetState(state);
}

void Scene::updatePlayerAnimation(int id, std::vector<float>& positions, std::vector<float>& weights)
{
    auto player = getPlayer(id);
    if (!player) return;
    player->SetAnimPositions(positions);
    player->SetAnimWeights(weights);
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

void Scene::Update_Scene()
{

}

//======================================================
bool Lobby_Scene::SelectCharacter(int clientId, int characterId, bool isReady)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (characterId < 0 || characterId >= MaxPlayer) return false;
    if (clientId < 0 || clientId >= MaxPlayer) return false;

    // Ready 하려는 캐릭터가 이미 다른 사람이 Ready 했다면 거부
    if (isReady && characterReady[characterId] != -1 && characterReady[characterId] != clientId)
        return false;

    // 모든 캐릭터에서 clientId 선택 해제
    for (int i = 0; i < MaxPlayer; ++i)
        characterSelections[i][clientId] = false;

    // 선택한 캐릭터에 표시
    characterSelections[characterId][clientId] = true;

    // Ready 상태 갱신
    if (isReady)
        characterReady[characterId] = clientId;
    else if (characterReady[characterId] == clientId)
        characterReady[characterId] = -1;

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

void Lobby_Scene::ResetCharacterSlot(int clientId)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (int i = 0; i < MaxPlayer; ++i)
    {
        characterSelections[i][clientId] = false;

        if (characterReady[i] == clientId)
            characterReady[i] = -1;
    }
}


Scene_Type Lobby_Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);


    if (Change_Scene_Trigger)
        return Scene_Type::Board;
    else
        return Scene_Type::None;

}


void Lobby_Scene::Update_Scene()
{

    Change_Scene_Trigger = IsAllReadyAndValid();

}


//======================================================

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


void Board_Scene::Update_Scene()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    float deltaTime = 0.016f;
    auto boat = dynamic_pointer_cast<Boat_Object>(pirate_ship);
    
    if (!boat) return;

    int fwd = 0, back = 0, left = 0, right = 0;
 
    for (int i = 0; i < MaxPlayer; ++i)
    {
        int32_t key = player_keyState[i];
        if (key & INPUT_W) fwd++;
        if (key & INPUT_S) back++;
        if (key & INPUT_A) left++;
        if (key & INPUT_D) right++;
    }

    if (fwd) boat->MoveForward(10.0f * fwd);
    if (back) boat->MoveForward(-10.0f * back);
    if (left) boat->Add_Rotate(-1.0f * left);
    if (right) boat->Add_Rotate(1.0f * right);

    boat->Animate(deltaTime);

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

Scene_Type Stage_Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (Change_Scene_Trigger)
        return Scene_Type::Stage_1;
    else
        return Scene_Type::None;

}


void Stage_Scene::Update_Scene()
{

}
