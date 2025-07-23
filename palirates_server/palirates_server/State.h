#pragma once

#include <memory>
#include "StateEnum.h"

class Monster;
class MonsterStateMachine;

class MonsterState {
public:
    virtual ~MonsterState() = default;

    virtual void Enter(Monster* monster, MonsterStateMachine* sm) = 0;
    virtual void Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) = 0;
    virtual void Exit(Monster* monster) = 0;

    virtual State GetStateEnum() const = 0;

    int currentTrackIdx = -1;
};

class IdleState : public MonsterState {
public:
    void Enter(Monster* monster, MonsterStateMachine* sm) override;
    void Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) override;
    void Exit(Monster* monster) override;
    State GetStateEnum() const override { return State::Idle; }
};

class WalkState : public MonsterState {
public:
    void Enter(Monster* monster, MonsterStateMachine* sm) override;
    void Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) override;
    void Exit(Monster* monster) override;
    State GetStateEnum() const override { return State::Run; }
};

class AttackState : public MonsterState {
public:
    void Enter(Monster* monster, MonsterStateMachine* sm) override;
    void Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) override;
    void Exit(Monster* monster) override;
    State GetStateEnum() const override { return State::Attack1; }
};

class GetHitState : public MonsterState {
public:
    void Enter(Monster* monster, MonsterStateMachine* sm) override;
    void Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) override;
    void Exit(Monster* monster) override;
    State GetStateEnum() const override { return State::Get_Hit; }
};

class DragonBreatheState : public MonsterState {
public:
    void Enter(Monster* monster, MonsterStateMachine* sm) override;
    void Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) override;
    void Exit(Monster* monster) override;
    State GetStateEnum() const override { return State::Attack2; }
};
