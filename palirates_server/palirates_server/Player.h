#pragma once
#include "GameObject.h"
#include "AnimationTrackEnum.h"
#include "PlayerStateMachine.h"
#include <unordered_set> 

enum class Player_State : int
{
    Idle,
    Walk,
    Run,
    Jump,
    Attack,
    Dash,
    ETC
};



class Player : public Skinned_GameObject
{
private:
    int model_index = -1;
    Player_State player_state = Player_State::Idle;

    std::shared_ptr<BoundingOrientedBox> m_localOBB; 
    std::shared_ptr<BoundingOrientedBox> m_worldOBB;  

    std::unique_ptr<PlayerStateMachine> m_StateMachine;
    float hp = 100.0f;
public:
    bool need_to_client_sync = false;
    UINT mosaic_value = 0;
    bool BreathHit = false;
    int Client_ID = -1;
    bool bCanControll = true;
    XMFLOAT3 CommandSetLook = XMFLOAT3(0.0f, 0.0f, 0.0f);
    Player(int model_index);
    virtual ~Player() {}

    int Get_Model_ID() const { return model_index; }

    Player_State GetState() const { return player_state; }
    void SetState(Player_State s) { player_state = s; }

    void key_input(uint32_t keyState);

    virtual void animate(float Elapsedtime);
    virtual void update(float deltaTime) override;
    void update_collision(float deltaTime, std::vector<BoundingOrientedBox> obblist);

    virtual void UpdateWorldOBB();
    virtual std::shared_ptr<BoundingOrientedBox> Get_Collider_OBB() { return m_worldOBB; }
    std::shared_ptr<BoundingOrientedBox> Get_Collider_OBB_T() { return m_OBB; }
    void Set_Collider_OBB_Center(const XMFLOAT3& newWorldCenter);

    void InitAnimationController(const std::string& filepath, int animCount, int rootIdx, const std::unordered_set<int>& onceTracks) override;
    void InitStateMachine();

    PlayerStateMachine* GetStateMachine() { return m_StateMachine.get(); }
    float GetHP() { return hp; }
    void SetHP(float setHP) { hp = setHP; }
    void HitDamage(float damage);

    virtual int PlayAnimation(State state);
    void SetRunDirectionTrack(int track);
};
