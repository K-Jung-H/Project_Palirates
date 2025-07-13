#include "stdafx.h"
#include "MonsterState.h"
#include "Object_StateMachine.h"
#include <memory>

// -------------------------
// IdleState
// -------------------------
void IdleState::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) return;
    monster->PlayAnimation(State::Idle);
}

void IdleState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm) return;

    auto target = monster->FindNearestPlayerInRange(10.0f);
    if (target) {
        monster->SetTarget(target);
        sm->ChangeState(std::make_unique<AttackState>());
    }
}

void IdleState::Exit(Monster* monster) {
    // Exit 시 별도 처리 필요 시 작성
}

// -------------------------
// AttackState
// -------------------------
void AttackState::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) return;
    monster->PlayAnimation(State::Attack1);
    monster->StartAttackCooldown();
}

void AttackState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm) return;

    if (monster->IsAttackCooldownOver()) {
        sm->ChangeState(std::make_unique<IdleState>());
    }
}

void AttackState::Exit(Monster* monster) {
    // 공격 상태에서 빠져나올 때 효과 종료 등 필요 시 처리
}
