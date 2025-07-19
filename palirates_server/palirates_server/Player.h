#pragma once
#include "GameObject.h"
#include "AnimationTrackEnum.h"
<<<<<<< HEAD
=======
#include "PlayerStateMachine.h"
>>>>>>> main
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

<<<<<<< HEAD
    std::shared_ptr<BoundingOrientedBox> m_localOBB;  // 변하지 않는 기본 값
    std::shared_ptr<BoundingOrientedBox> m_worldOBB;  // 충돌 검사용 
=======
    std::shared_ptr<BoundingOrientedBox> m_localOBB; 
    std::shared_ptr<BoundingOrientedBox> m_worldOBB;  

    std::unique_ptr<PlayerStateMachine> m_StateMachine;
>>>>>>> main

public:
    bool need_to_client_sync = false;

    Player(int model_index);
    virtual ~Player() {}

    int Get_Model_ID() const { return model_index; }

    Player_State GetState() const { return player_state; }
    void SetState(Player_State s) { player_state = s; }

    void key_input(uint32_t keyState);

    virtual void animate(float Elapsedtime);
<<<<<<< HEAD
    //virtual void update(float deltaTime) override;
=======
    virtual void update(float deltaTime) override;
>>>>>>> main

    virtual void UpdateWorldOBB();
    virtual std::shared_ptr<BoundingOrientedBox> Get_Collider_OBB() { return m_worldOBB; }
    void Set_Collider_OBB_Center(const XMFLOAT3& newWorldCenter);

    void InitAnimationController(const std::string& filepath, int animCount, int rootIdx, const std::unordered_set<int>& onceTracks) override;
<<<<<<< HEAD
=======
    void InitStateMachine();

    PlayerStateMachine* GetStateMachine() { return m_StateMachine.get(); }
>>>>>>> main
};
