#pragma once
#include "GameObject.h"



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
    int player_id = -1;
    Player_State player_state = Player_State::Idle;

public:
    Player(int playerId);
    virtual ~Player() {}

    int GetID() const { return player_id; }
    Player_State GetState() const { return player_state; }
    void SetState(Player_State s) { player_state = s; }

    void key_input(uint32_t keyState);

    virtual void animate(float Elapsedtime);
    virtual void update();

    XMFLOAT3 inputDirection = { 0.0f, 0.0f, 0.0f };
    float test_value = 0;
};
