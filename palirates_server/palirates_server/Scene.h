#pragma once

#include <unordered_map>
#include <array>
#include <memory>
#include "stdafx.h"
#include "GameWorld.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "ServerAnimLoader.h"

class Scene
{
public:
    static int active_client_num;
    static std::array<int, MaxPlayer> player_model_list;

protected:
    mutable std::recursive_mutex sceneMutex;

    Scene_Type sceneType;
    bool Change_Scene_Trigger = false;

public:
    Scene(Scene_Type type = Scene_Type::Test);
    virtual void Init() {}

    std::recursive_mutex& GetSceneMutex() const { return sceneMutex; }
    Scene_Type GetSceneType() const;

    // --- 상태 업데이트 함수 ---
    virtual void Update_Scene(float elapsedTime);
    virtual void Remove_Player(int id) {}

    virtual bool IsAllReadyAndValid() { return false; }
    virtual Scene_Type CheckSceneTransition();

    virtual Effect_Sync_Data Get_Effect_Status();
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
        Init();
    }

    virtual void Init();
    virtual void Update_Scene(float elapsedTime);
    virtual void Remove_Player(int id);
    virtual bool IsAllReadyAndValid();
    virtual Scene_Type CheckSceneTransition();

    bool SelectCharacter(int clientId, int characterId, bool isReady);



    const std::array<std::array<bool, MaxPlayer>, MaxPlayer>& GetCharacterSelections() const { return characterSelections; }
    const std::array<int, MaxPlayer>& GetCharacterReadyStates() const { return characterReady; }
};



class Board_Scene : public Scene
{
private:
    shared_ptr<Boat_Object> pirate_ship;
    std::array<int32_t, MaxPlayer> player_keyState;
    std::array<pair<int, bool>, MaxPlayer> stage_select_state;

    int selected_stage_index = -1;

public:
    Board_Scene() : Scene(Scene_Type::Board)
    {
        pirate_ship = make_shared<Boat_Object>();
        Init();
    }

    virtual void Init();

    virtual void Update_Scene(float elapsedTime);
    virtual void Remove_Player(int id);
    virtual bool IsAllReadyAndValid();
    virtual Scene_Type CheckSceneTransition();


    void Update_KeyState(int Client_ID, int32_t keyState);
    void Select_State(int Client_ID, pair<int, bool> select_state);

    XMFLOAT3 Get_PirateShip_Position() const;
    XMFLOAT3 Get_PirateShip_Look() const;
};

class Stage_Scene : public Scene
{
protected:
    std::array<std::shared_ptr<Player>, MaxPlayer> player_list;
    std::array<int32_t, MaxPlayer> player_keyState;
    std::array<bool, MaxPlayer> check_clear_state;

    GameWorld game_world;
    std::shared_ptr<GameObject> scene_obj;

    vector<std::shared_ptr<GameObject>> monster_init_spawn_frame_list;
    vector<std::shared_ptr<GameObject>> player_init_spawn_frame_list;

    std::vector<std::shared_ptr<Monster>> Monster_List;
    std::unordered_map<int, size_t> id2idx;
    std::vector<int> monster_despawn_queue;
    std::vector<MonsterDamageInfo> monster_damage_queue;

    std::shared_ptr<Monster> Boss_Monster = NULL;
    std::shared_ptr<GameObject> zoomObject;

public:
    Stage_Scene(Scene_Type scene_type);
    virtual void Init();

    virtual void Update_Scene(float elapsedTime);
    virtual void Remove_Player(int id);
    virtual bool IsAllReadyAndValid() { return false; }
    virtual Scene_Type CheckSceneTransition();
    virtual Effect_Sync_Data Get_Effect_Status();

    void Add_Player(int id);

    std::shared_ptr<Player> Get_Player(int id);
    const std::array<std::shared_ptr<Player>, MaxPlayer> Get_PlayerList() const;

    void Update_Clear_State(int playerid, bool state);
    bool Check_Clear_Scene();

    void update_player_keyinput(int id, uint32_t keystate);
    void update_player_LookV(int id, XMFLOAT3 new_lookV);
    void update_player_State(int clientId, uint32_t inputFlags, const XMFLOAT3& position, const XMFLOAT3& lookDirection, const std::vector<Animation_Sync>& tracks, bool stateChanged, int stateNum);


public:
    void SpawnMonster_By_Scene_Data();
    std::shared_ptr<Monster> SpawnMonster(int id, const XMFLOAT3& pos, int hp);
    void DespawnMonster(int id);
    std::shared_ptr<Monster> GetMonster(int id);

    const std::vector<std::shared_ptr<Monster>>& GetMonsterList() const { return Monster_List; }


    const FrameParticleChanges Get_Particle_Sync_Data();

    void QueueDespawnCommand(int id) { monster_despawn_queue.emplace_back(id); }
    std::vector<int> FlushDespawnQueue();

    void QueueDamageCommand(MonsterDamageInfo id) { monster_damage_queue.emplace_back(id); }
    std::vector<MonsterDamageInfo> FlushDamageQueue();


    void server_DespawnMonster();
    void server_DespawnMonster_For_Clear();

    bool bStageClear = false;
};

class Stage_1_Scene : public Stage_Scene
{
public:
    Stage_1_Scene();
    virtual void Init();
};

class Stage_2_Scene : public Stage_Scene
{
public:
    Stage_2_Scene();
    virtual void Init();
};

class Stage_3_Scene : public Stage_Scene
{
public:
    Stage_3_Scene();
    virtual void Init();
};

class Stage_4_Scene : public Stage_Scene
{
public:
    Stage_4_Scene();
    virtual void Init();
};

class Stage_5_Scene : public Stage_Scene
{
public:
    Stage_5_Scene();
    virtual void Init();
};

class Stage_6_Scene : public Stage_Scene
{
public:
    Stage_6_Scene();
    virtual void Init();
};

class Stage_7_Scene : public Stage_Scene
{
public:
    Stage_7_Scene();
    virtual void Init();
};