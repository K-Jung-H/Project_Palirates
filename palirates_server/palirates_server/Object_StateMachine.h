#pragma once

#include "Monster.h"
#include "State.h"
#include "StateEnum.h"  

class Monster;

class StateMachine {
protected:
    State currentState = State::Idle;

public:
    StateMachine() = default;
    virtual ~StateMachine() {
        animController.reset();
    }

    StateMachine(State initialState)
        : currentState(initialState) {
    }

    std::shared_ptr<CAnimationController> animController;
    int n_Ani{ 0 };

    virtual void update(float deltaTime) {}
    virtual void SetWeight(float deltaTime) {}
};

class MonsterStateMachine : public StateMachine {
protected:
    Monster* m_pOwner = nullptr;
    std::unique_ptr<MonsterState> currentState;
    State currentStateEnum = State::Idle;

public:
    MonsterStateMachine(Monster* owner)
        : StateMachine(State::Idle), m_pOwner(owner) {
        currentState = std::make_unique<IdleState>();
        currentStateEnum = State::Idle;
        if (currentState) {
            currentState->Enter(m_pOwner, this);
        }
        else
            std::cout << "fail enter" << std::endl;
    }

    uint32_t m_stateVer = 0;

    void update(float deltaTime) override;
    virtual void SetWeight(float deltaTime);

    void ChangeState(std::unique_ptr<MonsterState> newState);

    State GetCurrentStateEnum() const { return currentStateEnum; }
    MonsterState* GetCurrentState() const { return currentState.get(); }
    Monster* GetOwner() const { return m_pOwner; }

    void OnPrepareUpdate(float deltaTime);
};

class FishManStateMachine : public MonsterStateMachine {
public:
    FishManStateMachine(Monster* owner)
        : MonsterStateMachine(owner) {
    }
    ~FishManStateMachine() override = default;
};

class AnubisStateMachine : public MonsterStateMachine {
public:
    AnubisStateMachine(Monster* owner)
        : MonsterStateMachine(owner) {
    }
    ~AnubisStateMachine() override = default;
};

class DragonStateMachine : public MonsterStateMachine {
public:
    DragonStateMachine(Monster* owner)
        : MonsterStateMachine(owner) {
    }
    ~DragonStateMachine() override = default;
};
