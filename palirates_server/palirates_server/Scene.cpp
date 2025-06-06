#include "stdafx.h"
#include "Scene.h"
#include "Player.h"

void Scene::updatePlayerPosition(int clientId, float x, float y, float z, float lookX, float lookY, float lookZ, EState state)
{
    Player* player = getPlayerById(clientId);

    XMFLOAT3 new_pos = { x,y,z };
    XMFLOAT3 new_lookV = { lookX, lookY, lookZ };

    if (!player)
    {
        player = new Player(clientId, new_pos, new_lookV);


        player->setPosition(x, y, z);
        player->setState(state);
        player->setLookVec(lookX, lookY, lookZ);

        playerMap[clientId] = player;

        std::cout << "[INFO] [자동 생성] Player 객체 생성 및 등록: ID=" << clientId << std::endl;
    }
    else
    {
        player->setPosition(x, y, z);
        player->setState(state);
        player->setLookVec(lookX, lookY, lookZ);
    }

}

void Scene::Update_Player(int clientId, uint32_t keyState, XMFLOAT3 lookVector)
{
    Player* player = getPlayerById(clientId);

    if (player == nullptr)
    {
        Player* new_player = Add_Player(clientId);
        playerMap[clientId] = new_player;
        std::cout << "[INFO] [자동 생성] Player 객체 생성 및 등록: ID=" << clientId << std::endl;
    }

    if (player)
    {
        player->setLookVec(lookVector);
        player->Update(keyState);
    }

}

Player* Scene::Add_Player(int clientId)
{
    XMFLOAT3 Default_Position= { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 Default_LookVector = { 0.0f, 1.0f, 0.0f };
    Player* new_player = new Player(clientId, Default_Position, Default_LookVector);

    return new_player;
}


Player* Scene::getPlayerById(int id)
{
    auto it = playerMap.find(id);
    if (it != playerMap.end())
    {
        return it->second;
    }
    return nullptr;
}

void Scene::updatePlayerAnimation(int playerId, const std::vector<float>& trackPositions, const std::vector<float>& trackWeights)
{
    GameCharacter* player = getPlayer(playerId);
    if (!player) return;

    player->animPositions = trackPositions;
    player->animWeights = trackWeights;
}

GameCharacter* Scene::getPlayer(int id)
{
    if (players.find(id) == players.end()) return nullptr;
    return &players[id];
}

void Scene::addMonster(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp, int state, Monster_Type type)
{
    Monster m(id, x, y, z, lookX, lookY, lookZ, hp, state, type);
    addMonster(id, m);
}

void Scene::addMonster(int id, const Monster& monster)
{
    monsterMap[id] = monster;
}

void Scene::printScene()
{
    std::cout << "현재 씬의 플레이어 목록:\n";
    for (const auto& [id, player] : players)
    {
        std::cout << "플레이어 " << id << ": (" << player.x << ", " << player.y << ", " << player.z << ") 상태: " << static_cast<int>(player.state) << std::endl;
    }
}