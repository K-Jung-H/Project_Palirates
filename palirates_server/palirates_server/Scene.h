#pragma once
#include <unordered_map>
#include <memory>
#include "Player.h"

class Scene
{
private:
    std::unordered_map<int, std::shared_ptr<Player>> players;

public:
    std::shared_ptr<Player> getPlayer(int id);
    std::shared_ptr<Player> addPlayer(int id, XMFLOAT3 pos, XMFLOAT3 look);
    void removePlayer(int id);

    void updatePlayerPosition(int id, float x, float y, float z,
        float lookX, float lookY, float lookZ, EState state);

    void updatePlayerAnimation(int id,
        const std::vector<float>& positions,
        const std::vector<float>& weights);

    const std::unordered_map<int, std::shared_ptr<Player>>& getPlayers() const;
};