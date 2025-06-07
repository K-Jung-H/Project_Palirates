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
    int player_id;
    Player_State player_state;

public:

    Player(int playerId);
    virtual ~Player() {}

    Player_State GetState() { return player_state; }
    void SetState(Player_State newState) { player_state = newState; }

    void key_input(uint32_t keyState);
    virtual void animate(float Elapsedtime);
    virtual void update();

};
