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
<<<<<<< HEAD
=======
    if (monster->Weapon_ptr)
        monster->Weapon_ptr->SetCanCollide(true);
>>>>>>> main
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
<<<<<<< HEAD
=======
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
>>>>>>> main

    if (track >= 0 && track < sm->n_Ani) {
        const auto& animTrack = sm->animController->m_pAnimationTracks[track];

        if (animTrack.m_nType == ANIMATION_TYPE_ONCE && animTrack.m_bFinished) {
            sm->ChangeState(std::make_unique<IdleState>());
        }
    }
}

void AttackState::Exit(Monster* monster) {
<<<<<<< HEAD
    // 공격 상태에서 빠져나올 때 효과 종료 등 필요 시 처리
}
=======
    if (monster->Weapon_ptr)
        monster->Weapon_ptr->SetCanCollide(false);
}

/////////////////////////// GetHit ///////////////////////////////

void GetHitState::Enter(Monster* monster, MonsterStateMachine* sm) {
    monster->SetCanCollide(false);
    monster->SetIsInvincible(true);
    for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
        sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
    }
    sm->animController->m_pAnimationTracks[AnimationRegistry::GetMonsterAnimationTrack(monster->GetType(), State::Get_Hit)].m_fPosition = 0.0f;
    monster->PlayAnimation(State::Get_Hit);
}

void GetHitState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (sm->animController->m_pAnimationTracks[AnimationRegistry::GetMonsterAnimationTrack(monster->GetType(), State::Get_Hit)].m_bFinished) {
        sm->animController->m_pAnimationTracks[AnimationRegistry::GetMonsterAnimationTrack(monster->GetType(), State::Get_Hit)].m_bFinished = false;
        sm->ChangeState(std::make_unique<IdleState>());
    }
}

void GetHitState::Exit(Monster* monster) {
    monster->SetCanCollide(true);
}
>>>>>>> main
