#include "stdafx.h"
#include "Scene.h"

std::shared_ptr<Player> Scene::getPlayer(int id)
{
    auto it = player_data_map.find(id);
    if (it != player_data_map.end())
        return it->second;
    return nullptr;
}

std::shared_ptr<Player> Scene::addPlayer(int id, XMFLOAT3 pos, XMFLOAT3 look)
{
    auto player = std::make_shared<Player>(id);

    player->SetPosition(pos);
    player->SetLook(look);

    player_data_map[id] = player;

    return player;
}

void Scene::removePlayer(int id)
{
    player_data_map.erase(id);
}

void Scene::update_player_keyinput(int id, uint32_t keystate)
{
    auto player = getPlayer(id);
    if (!player) return;

    player->key_input(keystate);
}

void Scene::update_player_Position(int id, XMFLOAT3 new_pos)
{
    auto player = getPlayer(id);
    if (!player) return;

    player->SetPosition(new_pos);
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

    XMFLOAT3 new_lookV = { lookX, lookY, lookZ };

    player->SetPosition(x, y, z);
    player->SetLook(new_lookV);
    player->SetState(state);
}

void Scene::updatePlayerAnimation(int id, std::vector<float>& positions, std::vector<float>& weights)
{
    auto player = getPlayer(id);
    if (!player) return;


    player->SetAnimPositions(positions);
    player->SetAnimWeights(weights);
}

const std::unordered_map<int, std::shared_ptr<Player>>& Scene::getPlayers() const
{
    return player_data_map;
}