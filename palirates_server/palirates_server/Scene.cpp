#include "Scene.h"
#include "Player.h"

void Scene::updatePlayerPosition(int clientId, float x, float y, float z, int state)
{
    Player* player = getPlayerById(clientId);

    if (!player)
    {
        player = new Player(clientId, x, y, z, state);
        player->setPosition(x, y, z);
        player->setState(state);

        playerMap[clientId] = player;

        std::cout << "[INFO] [자동 생성] Player 객체 생성 및 등록: ID=" << clientId << std::endl;
    }
    else
    {
        player->setPosition(x, y, z);
        player->setState(state);
    }

    std::cout << "[DEBUG] updatePlayerPosition → ID=" << clientId
        << " Pos=(" << x << "," << y << "," << z << ")"
        << " State=" << state << std::endl;
}

void Scene::printScene()
{
    std::cout << "현재 씬의 플레이어 목록:\n";
    for (const auto& [id, player] : players)
    {
        std::cout << "플레이어 " << id << ": (" << player.x << ", " << player.y << ", " << player.z << ") 상태: " << player.state << std::endl;
    }
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