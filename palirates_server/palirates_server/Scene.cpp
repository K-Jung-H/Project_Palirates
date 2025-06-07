#include "stdafx.h"
#include "Scene.h"

std::shared_ptr<Player> Scene::getPlayer(int id)
{
    auto it = players.find(id);
    if (it != players.end())
        return it->second;
    return nullptr;
}

std::shared_ptr<Player> Scene::addPlayer(int id, XMFLOAT3 pos, XMFLOAT3 look)
{
    auto player = std::make_shared<Player>(id, pos, look);
    players[id] = player;
    return player;
}

void Scene::removePlayer(int id)
{
    players.erase(id);
}

void Scene::updatePlayerPosition(int id, float x, float y, float z,
    float lookX, float lookY, float lookZ, EState state)
{
    auto player = getPlayer(id);
    if (!player) return;

    player->setPosition(x, y, z);
    player->setLookVec(lookX, lookY, lookZ);
    player->setState(state);
}

void Scene::updatePlayerAnimation(int id,
    const std::vector<float>& positions,
    const std::vector<float>& weights)
{
    auto player = getPlayer(id);
    if (!player) return;

    player->animPositions = positions;
    player->animWeights = weights;
}

const std::unordered_map<int, std::shared_ptr<Player>>& Scene::getPlayers() const
{
    return players;
}