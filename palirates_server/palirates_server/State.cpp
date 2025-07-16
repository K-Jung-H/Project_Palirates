#include "stdafx.h"
#include "State.h"
#include "Object_StateMachine.h"
#include "AnimationRegistry.h"
#include <memory>

// -------------------------
// IdleState
// -------------------------
void IdleState::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) {
        std::cout << "no monster" << std::endl;
        return;
    }
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
#include <iomanip> 
void AttackState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm || !sm->animController) return;

    int track = AnimationRegistry::GetMonsterAnimationTrack(monster->GetType(), State::Attack1);


    if (monster->Weapon_ptr && monster->GetID() == 50331651) {
        const XMFLOAT3& pos = monster->Weapon_ptr->GetPosition();
        std::cout << std::fixed << std::setprecision(3);
       // std::cout << "weapon got - " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    }
    if (monster->Weapon_ptr && monster->GetID() == 16777217) {
        const XMFLOAT3& pos = monster->Weapon_ptr->GetPosition();
        std::cout << std::fixed << std::setprecision(3);
        //std::cout << "weapon got - " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    }
   
    if (monster->Weapon_ptr && monster->GetID() == 0) {
        const XMFLOAT3& pos = monster->Weapon_ptr->GetPosition();
        std::cout << std::fixed << std::setprecision(3);
   //     std::cout << "weapon got - " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    }

    if (track >= 0 && track < sm->n_Ani) {
        const auto& animTrack = sm->animController->m_pAnimationTracks[track];

        if (animTrack.m_nType == ANIMATION_TYPE_ONCE && animTrack.m_bFinished) {
            sm->ChangeState(std::make_unique<IdleState>());
        }
    }
}

void AttackState::Exit(Monster* monster) {
    // 공격 상태에서 빠져나올 때 효과 종료 등 필요 시 처리
}
