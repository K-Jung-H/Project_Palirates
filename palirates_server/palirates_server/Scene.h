#pragma once
#include <unordered_map>
#include <memory>
#include "stdafx.h"
#include "Player.h"
#include "Monster.h"


class Scene
{
protected:
    mutable std::recursive_mutex sceneMutex;
    std::unordered_map<int, std::shared_ptr<Player>> player_map;
    Scene_Type sceneType;
    bool scene_transitionTriggered = false;

public:
    Scene(Scene_Type type = Scene_Type::Test);

    std::recursive_mutex& GetSceneMutex() const;

    Scene_Type GetSceneType() const;

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

    virtual void Update() {}
    virtual Scene_Type CheckSceneTransition();
};


class Lobby_Scene : public Scene
{
private:
    std::unordered_map<int, std::pair<int, bool>> characterSlots;
public:
    Lobby_Scene() : Scene(Scene_Type::Lobby) {}

    bool SelectCharacter(int clientId, int characterId, bool isReady);
    bool IsAllReadyAndValid();
    void ResetCharacterSlot(int clientId); // 클라이언트(clientId)가 선택한 캐릭터 슬롯을 초기화
    virtual Scene_Type CheckSceneTransition();


};

class Board_Scene : public Scene
{
private:
    shared_ptr<GameObject> pirate_ship;

public:
    Board_Scene() : Scene(Scene_Type::Board) {}

};

class Stage_Scene : public Scene
{

public:
    Stage_Scene() : Scene(Scene_Type::Stage_1) {}

};
