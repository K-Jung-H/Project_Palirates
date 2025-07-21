#include "stdafx.h"
#include "State.h"
#include "Object_StateMachine.h"
#include "AnimationRegistry.h"
#include <memory>
#include <random>

inline float RandomFloat()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(gen);
}

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
    monster->stateElapsedTime += deltaTime;
    if (monster->stateElapsedTime > 1.0f) {
        monster->stateElapsedTime = 0.0f;

        if (RandomFloat() < 1.0f) {
            sm->ChangeState(std::make_unique<WalkState>());
        }
    }

}

void IdleState::Exit(Monster* monster) {
}

// -------------------------
// WalkState
// -------------------------
void WalkState::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) {
        return;
    }
    monster->PlayAnimation(State::Run);
    std::cout << "WalkState Enter" << std::endl;
}

void WalkState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm) return;

    /*auto target = monster->FindNearestPlayerInRange(10.0f);
    if (target) {
        monster->SetTarget(target);
        sm->ChangeState(std::make_unique<AttackState>());
    }*/
    //monster->Move();
    //if (monster->GetID() == 0)
        //std::cout << sm->animController->HipsPosition.x << ", " << sm->animController->HipsPosition.y << ", " << sm->animController->HipsPosition.z << "\n";
 /*   monster->stateElapsedTime += deltaTime;
    if (monster->stateElapsedTime > 3.0f) {
        monster->stateElapsedTime = 0.0f;
        if (RandomFloat() < 0.3f) {
            sm->ChangeState(std::make_unique<IdleState>());
        }
    }*/
}

void WalkState::Exit(Monster* monster) {
}

// -------------------------
// AttackState
// -------------------------
void AttackState::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) return;
    monster->PlayAnimation(State::Attack1);
    monster->StartAttackCooldown();
    if (monster->Weapon_ptr)
        monster->Weapon_ptr->SetCanCollide(true);
}
#include <iomanip> 
void AttackState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm || !sm->animController) return;

    int track = AnimationRegistry::GetMonsterAnimationTrack(monster->GetType(), State::Attack1);

    if (sm->animController->m_pAnimationTracks[track].m_bFinished) {
        sm->animController->m_pAnimationTracks[track].m_bFinished = false;
        sm->ChangeState(std::make_unique<IdleState>());
    }
   // if (monster->Weapon_ptr && monster->GetID() == 50331651) {
   //     const XMFLOAT3& pos = monster->Weapon_ptr->GetPosition();
   //     std::cout << std::fixed << std::setprecision(3);
   //    // std::cout << "weapon got - " << pos.x << " " << pos.y << " " << pos.z << std::endl;
   // }
   // if (monster->Weapon_ptr && monster->GetID() == 16777217) {
   //     const XMFLOAT3& pos = monster->Weapon_ptr->GetPosition();
   //     std::cout << std::fixed << std::setprecision(3);
   //     //std::cout << "weapon got - " << pos.x << " " << pos.y << " " << pos.z << std::endl;
   // }
   //
   // if (monster->Weapon_ptr && monster->GetID() == 0) {
   //     const XMFLOAT3& pos = monster->Weapon_ptr->GetPosition();
   //     std::cout << std::fixed << std::setprecision(3);
   ////     std::cout << "weapon got - " << pos.x << " " << pos.y << " " << pos.z << std::endl;
   // }

   // if (track >= 0 && track < sm->n_Ani) {
   //     const auto& animTrack = sm->animController->m_pAnimationTracks[track];

   //     if (animTrack.m_nType == ANIMATION_TYPE_ONCE && animTrack.m_bFinished) {
   //         sm->ChangeState(std::make_unique<IdleState>());
   //     }
   // }
}

void AttackState::Exit(Monster* monster) {
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