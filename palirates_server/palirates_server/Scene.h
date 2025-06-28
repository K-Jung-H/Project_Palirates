#pragma once
#include <unordered_map>
#include <memory>
#include "stdafx.h"
#include "Player.h"
#include "Monster.h"


class Scene
{
private:
    mutable std::recursive_mutex sceneMutex;
    std::unordered_map<int, std::shared_ptr<Player>> player_map;
    Scene_Type sceneType;

public:
    Scene(Scene_Type type = Scene_Type::Test);

    std::recursive_mutex& GetSceneMutex() const;

    Scene_Type GetSceneType() const;
    void SetSceneType(Scene_Type type);

    void addPlayer(int id, std::shared_ptr<Player> player);
    void removePlayer(int id);
    std::shared_ptr<Player> getPlayer(int id);
    const std::unordered_map<int, std::shared_ptr<Player>>& getPlayers() const;

    // --- 상태 업데이트 함수 ---
    void update_player_keyinput(int id, uint32_t keystate);
    void update_player_Position();
    void update_player_LookV(int id, XMFLOAT3 new_lookV);
    void updatePlayerPosition(int id, float x, float y, float z, float lookX, float lookY, float lookZ, Player_State state);
    void updatePlayerAnimation(int id, std::vector<float>& positions, std::vector<float>& weights);
};