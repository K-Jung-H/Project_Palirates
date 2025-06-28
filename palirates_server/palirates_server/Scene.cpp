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

    auto it = characterSlots.find(characterId);
    if (it != characterSlots.end())
    {
        int ownerId = it->second.first;
        bool ownerReady = it->second.second;

        if (ownerId != clientId && ownerReady)
        {
            return false;
        }
    }

    for (auto it = characterSlots.begin(); it != characterSlots.end(); )
    {
        if (it->second.first == clientId)
            it = characterSlots.erase(it);
        else
            ++it;
    }

    characterSlots[characterId] = { clientId, isReady };
    return true;
}

bool Lobby_Scene::IsAllReadyAndValid()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (characterSlots.empty()) return false;

    int readyCount = 0;

    for (const auto& [charId, pair] : characterSlots)
    {
        if (pair.second) // 레디 상태인 경우
            ++readyCount;
    }

    if (readyCount == active_client_num)
        int a = 1;

    return (readyCount == active_client_num);
}

void Lobby_Scene::ResetCharacterSlot(int clientId)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (auto it = characterSlots.begin(); it != characterSlots.end(); )
    {
        if (it->second.first == clientId)
        {
            it = characterSlots.erase(it);
        }
        else
        {
            ++it;
        }
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


void Board_Scene::Update_Scene()
{

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
