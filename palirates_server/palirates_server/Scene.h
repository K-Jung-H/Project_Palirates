#pragma once
#include <unordered_map>
#include <memory>
#include "Player.h"

class Scene
{
private:
    std::unordered_map<int, std::shared_ptr<Player>> player_data_map;

public:
    std::shared_ptr<Player> getPlayer(int id);
    std::shared_ptr<Player> addPlayer(int id, XMFLOAT3 pos, XMFLOAT3 look);
    void removePlayer(int id);

    void update_player_keyinput(int id, uint32_t keystate);
    void update_player_Position(int id, XMFLOAT3 new_pos);
    void update_player_LookV(int id, XMFLOAT3 new_lookV);

    void updatePlayerPosition(int id, float x, float y, float z,
        float lookX, float lookY, float lookZ, Player_State state);

    void updatePlayerAnimation(int id, std::vector<float>& positions, std::vector<float>& weights);

    const std::unordered_map<int, std::shared_ptr<Player>>& getPlayers() const;
};