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
    int model_index = -1;
    Player_State player_state = Player_State::Idle;

public:
    Player(int model_index);
    virtual ~Player() {}

    int Get_Model_ID() const { return model_index; }

    Player_State GetState() const { return player_state; }
    void SetState(Player_State s) { player_state = s; }

    void key_input(uint32_t keyState);

    virtual void animate(float Elapsedtime);
    virtual void update();

};
