#pragma once
#include <unordered_map>
#include <array>
#include <memory>
#include "stdafx.h"
#include "Player.h"
#include "Monster.h"

#define MaxPlayer 6

class Scene
{
public:
    static int active_client_num;

protected:
    mutable std::recursive_mutex sceneMutex;
    std::unordered_map<int, std::shared_ptr<Player>> player_map;

    Scene_Type sceneType;
    bool Change_Scene_Trigger = false;

public:
    Scene(Scene_Type type = Scene_Type::Test);

    std::recursive_mutex& GetSceneMutex() const { return sceneMutex; }

    Scene_Type GetSceneType() const;

    void addPlayer(int id, std::shared_ptr<Player> player);
    void removePlayer(int id);
    std::shared_ptr<Player> getPlayer(int id);
    const std::unordered_map<int, std::shared_ptr<Player>>& getPlayers() const;

    // --- 상태 업데이트 함수 ---
    virtual void Update_Scene();

    void update_player_keyinput(int id, uint32_t keystate);
    void update_player_Position();
    void update_player_LookV(int id, XMFLOAT3 new_lookV);
    void updatePlayerPosition(int id, float x, float y, float z, float lookX, float lookY, float lookZ, Player_State state);
    void updatePlayerAnimation(int id, std::vector<float>& positions, std::vector<float>& weights);


    virtual Scene_Type CheckSceneTransition();
};


class Lobby_Scene : public Scene
{
private:
    // characterSelections[캐릭터 ID][클라이언트 ID] = 선택 여부
    std::array<std::array<bool, MaxPlayer>, MaxPlayer> characterSelections;

    // characterReady[캐릭터 ID] = Ready한 클라이언트 ID (또는 -1)
    std::array<int, MaxPlayer> characterReady;
public:
    Lobby_Scene() : Scene(Scene_Type::Lobby)
    {
        for (int i = 0; i < MaxPlayer; ++i)
        {
            characterReady[i] = -1;
            for (int j = 0; j < MaxPlayer; ++j)
            {
                characterSelections[i][j] = false;
            }
        }
    }

    virtual void Update_Scene();


    bool SelectCharacter(int clientId, int characterId, bool isReady);
    bool IsAllReadyAndValid();
    void ResetCharacterSlot(int clientId); // 클라이언트(clientId)가 선택한 캐릭터 슬롯을 초기화
    virtual Scene_Type CheckSceneTransition();
    
    const std::array<std::array<bool, MaxPlayer>, MaxPlayer>& GetCharacterSelections() const { return characterSelections; }
    const std::array<int, MaxPlayer>& GetCharacterReadyStates() const { return characterReady; }
};



class Board_Scene : public Scene
{
private:
    shared_ptr<Boat_Object> pirate_ship;
    array<int32_t, MaxPlayer> player_keyState;

public:
    Board_Scene() : Scene(Scene_Type::Board) {}

    virtual void Update_Scene();

    void Update_KeyState(int Client_ID, int32_t keyState);
    
    virtual Scene_Type CheckSceneTransition();

    XMFLOAT3 Get_PirateShip_Position() const;
    XMFLOAT3 Get_PirateShip_Look() const;
};

class Stage_Scene : public Scene
{
private:

public:
    Stage_Scene() : Scene(Scene_Type::Stage_1) {}

    virtual void Update_Scene();


    virtual Scene_Type CheckSceneTransition();

};
