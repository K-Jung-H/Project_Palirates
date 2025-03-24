#include "Scene.h"

void Scene::updatePlayerPosition(int playerId, float x, float y, float z, int state)
{
    if (players.find(playerId) != players.end())
    {
        players[playerId].setPosition(x, y, z);
        players[playerId].setState(state);
    }
    else
    {
        players[playerId] = GameCharacter(playerId, x, y, z, state);
    }
}

void Scene::printScene()
{
    std::cout << "현재 씬의 플레이어 목록:\n";
    for (const auto& [id, player] : players)
    {
        std::cout << "플레이어 " << id << ": (" << player.x << ", " << player.y << ", " << player.z << ") 상태: " << player.state << std::endl;
    }
}
