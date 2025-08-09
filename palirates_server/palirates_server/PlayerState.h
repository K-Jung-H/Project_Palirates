#pragma once

#include <memory>
#include "StateEnum.h"

class Player;
class PlayerStateMachine;

class PlayerState {
public:
    virtual ~PlayerState() = default;

    virtual void Enter(Player* monster, PlayerStateMachine* sm) = 0;
    virtual void Update(Player* monster, float deltaTime, PlayerStateMachine* sm) = 0;
    virtual void Exit(Player* monster) = 0;

    virtual State GetStateEnum() const = 0;
    void PrepareForStateEnter(State state, Player* player, PlayerStateMachine* sm);
};

class PlayerNormalState : public PlayerState {
public:
    void Enter(Player* monster, PlayerStateMachine* sm) override;
    void Update(Player* monster, float deltaTime, PlayerStateMachine* sm) override;
    void Exit(Player* monster) override;

    State GetStateEnum() const override { return State::Idle; }
};

class PlayerRunState : public PlayerState {
public:
    void Enter(Player* monster, PlayerStateMachine* sm) override;
    void Update(Player* monster, float deltaTime, PlayerStateMachine* sm) override;
    void Exit(Player* monster) override;

    State GetStateEnum() const override { return State::Run; }
};

class PlayerAttack1State : public PlayerState {
public:
    void Enter(Player* monster, PlayerStateMachine* sm) override;
    void Update(Player* monster, float deltaTime, PlayerStateMachine* sm) override;
    void Exit(Player* monster) override;

    State GetStateEnum() const override { return State::Attack1; }
};

class PlayerAttack2State : public PlayerState {
public:
    void Enter(Player* monster, PlayerStateMachine* sm) override;
    void Update(Player* monster, float deltaTime, PlayerStateMachine* sm) override;
    void Exit(Player* monster) override;

    State GetStateEnum() const override { return State::Attack2; }
};

class PlayerAttack3State : public PlayerState {
public:
    void Enter(Player* monster, PlayerStateMachine* sm) override;
    void Update(Player* monster, float deltaTime, PlayerStateMachine* sm) override;
    void Exit(Player* monster) override;

    State GetStateEnum() const override { return State::Attack3; }
};

class PlayerGetHitState : public PlayerState {
public:
    void Enter(Player* monster, PlayerStateMachine* sm) override;
    void Update(Player* monster, float deltaTime, PlayerStateMachine* sm) override;
    void Exit(Player* monster) override;

    State GetStateEnum() const override { return State::Get_Hit_F2; }
};

class PlayerDeadState : public PlayerState {
public:
    void Enter(Player* monster, PlayerStateMachine* sm) override;
    void Update(Player* monster, float deltaTime, PlayerStateMachine* sm) override;
    void Exit(Player* monster) override;

    State GetStateEnum() const override { return State::Knock_Down; }
};

class PlayerGetUpState : public PlayerState {
public:
    void Enter(Player* monster, PlayerStateMachine* sm) override;
    void Update(Player* monster, float deltaTime, PlayerStateMachine* sm) override;
    void Exit(Player* monster) override;

    State GetStateEnum() const override { return State::Get_Up; }
};

class PlayerDiveState : public PlayerState {
public:
    void Enter(Player* monster, PlayerStateMachine* sm) override;
    void Update(Player* monster, float deltaTime, PlayerStateMachine* sm) override;
    void Exit(Player* monster) override;

    State GetStateEnum() const override { return State::Dive; }
};

