#include "stdafx.h"
#include "Scene.h"

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

    if (scene_transitionTriggered)
        return Scene_Type::None;

    scene_transitionTriggered = true;
    return Scene_Type::Stage_1;
}

//======================================================

bool Lobby_Scene::SelectCharacter(int clientId, int characterId, bool isReady)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    // 해당 캐릭터가 이미 다른 클라이언트에 의해 선택된 경우
    auto it = characterSlots.find(characterId);
    if (it != characterSlots.end() && it->second.first != clientId)
    {
        return false; // 중복 선택 불가
    }

    // 먼저 이전 선택 제거
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

    characterSlots[characterId] = { clientId, isReady };
    return true;
}

bool Lobby_Scene::IsAllReadyAndValid()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (characterSlots.empty()) return false;

    if (characterSlots.size() < getPlayers().size()) return false;

    for (const auto& [charId, pair] : characterSlots)
    {
        if (!pair.second) return false; // 아직 준비 안 된 슬롯 존재
    }

    return true;
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

    if (!IsAllReadyAndValid() || scene_transitionTriggered)
        return Scene_Type::None;

    scene_transitionTriggered = true;
    return Scene_Type::Board;
}
