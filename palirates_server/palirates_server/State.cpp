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
    currentTrackIdx = monster->PlayAnimation(State::Idle);

}

void IdleState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm) return;

    auto nearestPos = monster->FindNearestPlayerInRange(monster->detectionRange);
    if (nearestPos) {
        monster->SetTarget(*nearestPos);
        if (GET_MONSTER_TYPE(monster->GetID()) != int(Monster_Type::Dragon))
            sm->ChangeState(std::make_unique<WalkState>());
        else
            sm->ChangeState(std::make_unique<DragonBreatheState>());
    }

    if (GET_MONSTER_TYPE(monster->GetID()) != int(Monster_Type::Dragon)) {
        monster->stateElapsedTime += deltaTime;
        if (monster->stateElapsedTime > 1.0f) {
            monster->stateElapsedTime = 0.0f;

            if (RandomFloat() < 1.0f) {
                sm->ChangeState(std::make_unique<WalkState>());
            }
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
    if (auto controller = monster->GetSkinnedAnimationController()) {
        controller->AdvanceTime(0.0f, monster);
        controller->m_xmf3PrevHipsPosition = controller->HipsPosition;
    }
    //std::cout << "WalkState Enter" << std::endl;
}

void WalkState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm) return;
    monster->stateElapsedTime += deltaTime;
    auto nearestPos = monster->FindNearestPlayerInRange(100.0f);
    if (nearestPos) {
        monster->RotateTowardsTarget(nearestPos.value(), deltaTime, 1.0f);
       /* if (GetDistance(nearestPos.value(), monster->GetPosition()) <= monster->attackRange) {
            sm->ChangeState(std::make_unique<AttackState>());
        }*/
    }
    else {
        if (monster->stateElapsedTime > 1.0f) {
            monster->stateElapsedTime = 0.0f;
            if (RandomFloat() < 0.3f) {
                XMFLOAT3 targetPos = (RandomFloat() < 0.5f) ? monster->GetRight() : XMFLOAT3{
         -monster->GetRight().x,
         -monster->GetRight().y,
         -monster->GetRight().z
                };
                monster->SetTarget(targetPos);

                /*float angle = (RandomFloat() * 1.0f + 10.0f) * (RandomFloat() < 0.5f ? -1.0f : 1.0f);
                float radians = XMConvertToRadians(angle);
                XMMATRIX rot = XMMatrixRotationY(radians);
                XMVECTOR vec = XMLoadFloat3(&monster->GetLook());
                vec = XMVector3TransformNormal(vec, rot);

                XMFLOAT3 rotatedDir;
                XMStoreFloat3(&rotatedDir, vec);
                monster->SetTarget(rotatedDir);*/
            }
        }

        if (monster->m_shouldRotate) {
            monster->RotateTowardsDirection(monster->m_targetLookDir, deltaTime);
            XMFLOAT3 curLook = monster->GetLook();
            XMFLOAT3 tgtLook = monster->m_targetLookDir;

            float dot = Vector3::DotProduct(Vector3::Normalize(curLook), Vector3::Normalize(tgtLook));
            if (dot > 0.98f) {
                monster->m_shouldRotate = false;
            }
        }
    }
}

void WalkState::Exit(Monster* monster) {
}

// -------------------------
// AttackState
// -------------------------
void AttackState::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) return;
    if (GET_MONSTER_TYPE(monster->GetID()) == int(Monster_Type::Dragon)) {
        if (RandomFloat() < 1.0f) {
            currentTrackIdx = monster->PlayAnimation(State::Attack2);
        }
    }
    else currentTrackIdx = monster->PlayAnimation(State::Attack1);
    monster->StartAttackCooldown();
    if (monster->Weapon_ptr)
        monster->Weapon_ptr->SetCanCollide(true);
}
#include <iomanip> 
void AttackState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm || !sm->animController) return;

    if (sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished) {
        sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished = false;
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
    currentTrackIdx = monster->PlayAnimation(State::Get_Hit);
}

void GetHitState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished) {
        sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished = false;
        sm->ChangeState(std::make_unique<IdleState>());
    }
}

void GetHitState::Exit(Monster* monster) {
    monster->SetCanCollide(true);
}

// -------------------------
// DragonBreatheState
// -------------------------

void DragonBreatheState::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) return;

    else currentTrackIdx = monster->PlayAnimation(State::Attack2);
    monster->StartAttackCooldown();
    if (monster->Weapon_ptr)
        monster->Weapon_ptr->SetCanCollide(true);
}

void DragonBreatheState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm || !sm->animController) return;

    if (sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished) {
        sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished = false;
        sm->ChangeState(std::make_unique<IdleState>());
    }

}

void DragonBreatheState::Exit(Monster* monster) {
    if (monster->Weapon_ptr)
        monster->Weapon_ptr->SetCanCollide(false);
}

