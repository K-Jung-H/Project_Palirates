#pragma once
#include "Monster.h"

class Monster;

enum class State
{
    Idle,
    Run,
    Knock_Down,
    Get_Up,
    Dive,
    Attack1,
    Attack2,
    Attack3,
    Get_Hit_F2,
    Get_Hit,
    Select_Idle,
    Jump,
    Attack_Normal,
    ETC
};


enum AnimationTrack
{
    TRACK_IDLE = 0,
    TRACK_RUN_FORWARD_LEFT = 1,
    TRACK_RUN_FORWARD = 2,
    TRACK_RUN_FORWARD_RIGHT = 3,
    TRACK_RUN_BACKWARD_LEFT = 4,
    TRACK_RUN_BACKWARD = 5,
    TRACK_RUN_BACKWARD_RIGHT = 6,
    TRACK_RUN_LEFT = 7,
    TRACK_RUN_RIGHT = 8,
    TRACK_DIVEROLL_FORWARD = 9,
    TRACK_KNOCK_DOWN = 10,
    TRACK_GET_UP = 11,
    TRACK_ATTACK1 = 12,
    TRACK_ATTACK2 = 13,
    TRACK_ATTACK3 = 14,
    TRACK_GET_HIT_F2 = 15,
    TRACK_SELECT_IDLE = 16,

    TRACK_ANUBIS_IDLE = 0,
    TRACK_ANUBIS_IDLE_BREAK = 1,
    TRACK_ANUBIS_IDLE_TO_ATTACK_IDLE = 2,
    TRACK_ANUBIS_WALK = 3,
    TRACK_ANUBIS_BACK_WALK = 4,
    TRACK_ANUBIS_ATTACK1 = 5,
    TRACK_ANUBIS_ATTACK2 = 6,
    TRACK_ANUBIS_SKILL = 7,
    TRACK_ANUBIS_GET_HIT = 8,
    TRACK_ANUBIS_DEAD = 9,

    TRACK_FISHMAN_IDLE = 0,
    TRACK_FISHMAN_IDLE_BREAK = 1,
    TRACK_FISHMAN_TAUNT = 2,
    TRACK_FISHMAN_WALK = 3,
    TRACK_FISHMAN_WALK_BACK = 4,
    TRACK_FISHMAN_ATTACK1 = 5,
    TRACK_FISHMAN_ATTACK2 = 6,
    TRACK_FISHMAN_GET_HIT = 7,
    TRACK_FISHMAN_DEAD = 8,

    TRACK_DRAGON_IDLE = 0,
    TRACK_DRAGON_IDLE_BREAK = 1,
    TRACK_DRAGON_IDLE_LANDING = 2,
    TRACK_DRAGON_IDLE_TAKE_OFF = 3,
    TRACK_DRAGON_ATTACK1 = 4,
    TRACK_DRAGON_BREATHE = 5,
    TRACK_DRAGON_RUN = 6,
    TRACK_DRAGON_GOT_HIT1 = 7,
    TRACK_DRAGON_GOT_HIT2 = 8,
    TRACK_DRAGON_FLY_IDLE = 9,
    TRACK_DRAGON_FLY_BREATHE = 10,
    TRACK_DRAGON_FLY_DIVE = 11,
    TRACK_DRAGON_DEAD = 12
};

class StateMachine
{
protected:
    State currentState = State::Idle;

public:
    StateMachine() = default;
    virtual ~StateMachine() {
        animController.reset();
    }

    StateMachine(State initialState = State::Idle)
        : currentState(initialState) {
    }

    std::shared_ptr<CAnimationController> animController;

    virtual void update(float Elapsed_time) {};
};

class MonsterStateMachine : public StateMachine
{
protected:
    Monster* m_pOwner;
public:
    MonsterStateMachine(Monster* owner)
        : StateMachine(State::Idle), m_pOwner(owner) {

    }

    void update(float Elapsed_time) override;
};

class FishManStateMachine : public MonsterStateMachine
{
public:
    FishManStateMachine(Monster* owner)
        : MonsterStateMachine(owner) {
    }
    ~FishManStateMachine() override = default;

    void update(float Elapsed_time) override;
};