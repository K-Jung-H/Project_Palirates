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
    XMFLOAT3 pos = monster->GetPosition();
    if (pos.y > 0.0f) {
        pos.y -= deltaTime * 30.0f;
        if (pos.y < 0.0f)
            pos.y = 0.0f;
        monster->SetPosition(pos);
    }
    auto nearestPos = monster->FindNearestPlayerInRange(monster->detectionRange);
    if (nearestPos) {
        monster->SetTarget(*nearestPos);
        if (GET_MONSTER_TYPE(monster->GetID()) != int(Monster_Type::Dragon))
            sm->ChangeState(std::make_unique<WalkState>());
        else {
            if (Vector3::Distance(nearestPos.value(), monster->GetPosition()) <= monster->attackRange)
                sm->ChangeState(std::make_unique<Attack1State>());
            else sm->ChangeState(std::make_unique<DragonBreatheState>());
        }
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
        //controller->AdvanceTime(0.0f, monster);
        controller->m_xmf3PrevHipsPosition = controller->HipsPosition;
    }
    //std::cout << "WalkState Enter" << std::endl;
}

void WalkState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm) return;
    XMFLOAT3 pos = monster->GetPosition();
    if (pos.y > 0.0f) {
        pos.y -= deltaTime * 30.0f;
        if (pos.y < 0.0f)
            pos.y = 0.0f;
        monster->SetPosition(pos);
    }
    monster->stateElapsedTime += deltaTime;
    auto nearestPos = monster->FindNearestPlayerInRange(monster->detectionRange);
    if (nearestPos) {
        monster->RotateTowardsTarget(nearestPos.value(), deltaTime, 80.0f);
        if (GET_MONSTER_TYPE(monster->GetID()) == int(Monster_Type::Anubis)) {
            if (Vector3::Distance(nearestPos.value(), monster->GetPosition()) <= monster->attackRange) {
                if (RandomFloat() < 0.5f) {
                    sm->ChangeState(std::make_unique<Attack1State>());
                }
                else {
                    sm->ChangeState(std::make_unique<AnubisSkillState>());
                }
            }
        }
        else if (GET_MONSTER_TYPE(monster->GetID()) == int(Monster_Type::Gargoyle)) {
            if (Vector3::Distance(nearestPos.value(), monster->GetPosition()) <= monster->attackRange) {
                /*if (RandomFloat() < 0.5f) {
                    sm->ChangeState(std::make_unique<Attack1State>());
                }
                else {
                    sm->ChangeState(std::make_unique<GargoyleSkillState>());
                }*/
                sm->ChangeState(std::make_unique<GargoyleSkillState>());
            }
        }
        else {
            if (Vector3::Distance(nearestPos.value(), monster->GetPosition()) <= monster->attackRange) {
                sm->ChangeState(std::make_unique<Attack2State>());
            }
        }
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
            }
        }

        if (monster->m_shouldRotate) {
            monster->RotateTowardsDirection(monster->m_targetPos, deltaTime);
            XMFLOAT3 curLook = monster->GetLook();
            XMFLOAT3 tgtLook = monster->m_targetPos;

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
// Attack1State
// -------------------------
void Attack1State::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) return;
    currentTrackIdx = monster->PlayAnimation(State::Attack1);
    monster->StartAttackCooldown();
   /* if (monster->Weapon_ptr)
        monster->Weapon_ptr->SetCanCollide(true);*/
    for (auto& w : monster->Weapon_ptr) {
        w->SetCanCollide(true);
    }
}

void Attack1State::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm || !sm->animController) return;
    XMFLOAT3 pos = monster->GetPosition();
    if (pos.y > 0.0f) {
        pos.y -= deltaTime * 30.0f;
        if (pos.y < 0.0f)
            pos.y = 0.0f;
        monster->SetPosition(pos);
    }
    auto nearestPos = monster->FindNearestPlayerInRange(monster->attackRange);
    if (nearestPos) {
        monster->SetTarget(*nearestPos);
        monster->RotateTowardsTarget(nearestPos.value(), deltaTime, 100.0f);
    }
 
    if (sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished) {
        sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished = false;
        sm->ChangeState(std::make_unique<IdleState>());
    }
}

void Attack1State::Exit(Monster* monster) {
    /*if (monster->Weapon_ptr)
        monster->Weapon_ptr->SetCanCollide(false);*/
    for (auto& w : monster->Weapon_ptr) {
        w->SetCanCollide(false);
    }
}

// -------------------------
// Attack2State
// -------------------------
void Attack2State::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) return;
    currentTrackIdx = monster->PlayAnimation(State::Attack2);
    monster->StartAttackCooldown();
    /* if (monster->Weapon_ptr)
         monster->Weapon_ptr->SetCanCollide(true);*/
    for (auto& w : monster->Weapon_ptr) {
        w->SetCanCollide(true);
    }
}

void Attack2State::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm || !sm->animController) return;
    XMFLOAT3 pos = monster->GetPosition();
    if (pos.y > 0.0f) {
        pos.y -= deltaTime * 30.0f;
        if (pos.y < 0.0f)
            pos.y = 0.0f;
        monster->SetPosition(pos);
    }
    auto nearestPos = monster->FindNearestPlayerInRange(monster->attackRange);
    if (nearestPos) {
        monster->SetTarget(*nearestPos);
        monster->RotateTowardsTarget(nearestPos.value(), deltaTime, 100.0f);
    }

    if (sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished) {
        sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished = false;
        sm->ChangeState(std::make_unique<IdleState>());
    }
}

void Attack2State::Exit(Monster* monster) {
    /*if (monster->Weapon_ptr)
        monster->Weapon_ptr->SetCanCollide(false);*/
    for (auto& w : monster->Weapon_ptr) {
        w->SetCanCollide(false);
    }
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
    monster->bHittingCmd = true;
}

/////////////////////////// Dead ///////////////////////////////

void DeadState::Enter(Monster* monster, MonsterStateMachine* sm) {
    monster->SetCanCollide(false);
    monster->SetIsInvincible(true);
    for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
        sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
    }
    sm->animController->m_pAnimationTracks[AnimationRegistry::GetMonsterAnimationTrack(monster->GetType(), State::Knock_Down)].m_fPosition = 0.0f;
    currentTrackIdx = monster->PlayAnimation(State::Knock_Down);
    monster->stateElapsedTime = 0.0f;
}

void DeadState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    constexpr float DeadTime = 5.0f;
    monster->stateElapsedTime += deltaTime;
    if (monster->stateElapsedTime >= DeadTime) {
        monster->bDead = true;
        monster->stateElapsedTime = 0.0f;
    }
    /*if (sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished) {
        sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished = false;
        sm->ChangeState(std::make_unique<IdleState>());
    }*/
}

void DeadState::Exit(Monster* monster) {
    monster->SetCanCollide(true);
}

// -------------------------
// DragonBreatheState
// -------------------------

void DragonBreatheState::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) return;
    monster->attackPhase = 1;
    currentTrackIdx = monster->PlayAnimation(State::Jump);
	/*monster->Weapon_ptr->BreathObject = true;
    monster->Weapon_ptr->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 50.0f);*/
    for (auto& w : monster->Weapon_ptr) {
        w->BreathObject = true;
        w->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 50.0f);
    }
}

void DragonBreatheState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm || !sm->animController) return;

    if (monster->attackPhase == 1) {
		XMFLOAT3 pos = monster->GetPosition();
        pos.y += deltaTime * 30.0f;
		monster->SetPosition(pos);
        auto nearestPos = monster->FindNearestPlayerInRange(monster->detectionRange);
        if (nearestPos) {
            monster->SetTarget(*nearestPos);
            monster->RotateTowardsTarget(nearestPos.value(), deltaTime, 100.0f);
        }

        if (pos.y > 60.0f) {
            monster->attackPhase = 2;
            currentTrackIdx = monster->PlayAnimation(State::Attack3);
            /*if (monster->Weapon_ptr)
                monster->Weapon_ptr->SetCanCollide(true);*/
            for (auto& w : monster->Weapon_ptr) {
                w->SetCanCollide(true);
            }
		}
    }
    else if (monster->attackPhase == 2) {
        auto nearestPos = monster->FindNearestPlayerInRange(monster->detectionRange);
        if (nearestPos) {
            monster->RotateTowardsTarget(nearestPos.value(), deltaTime, 100.0f);
        }
        if (sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished) {
            monster->attackPhase = 3;
            sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished = false;
            currentTrackIdx = monster->PlayAnimation(State::Jump);
            /*if (monster->Weapon_ptr)
                monster->Weapon_ptr->SetCanCollide(false);*/
            for (auto& w : monster->Weapon_ptr) {
                w->SetCanCollide(false);
            }
        }
    }
    else if (monster->attackPhase == 3) {
        XMFLOAT3 pos = monster->GetPosition();
        pos.y -= deltaTime * 30.0f;
        monster->SetPosition(pos);
        if (pos.y <= 0.0f) {
            monster->attackPhase = 1;
			pos.y = 0.0f;
            monster->SetPosition(pos);
            sm->ChangeState(std::make_unique<IdleState>());
        }
	}


}

void DragonBreatheState::Exit(Monster* monster) {
    for (auto& w : monster->Weapon_ptr) {
        w->SetCanCollide(false);
        w->BreathObject = false;
        w->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
    }
    monster->attackPhase = -1;
    /*if (monster->Weapon_ptr)
    {
        monster->Weapon_ptr->SetCanCollide(false);
        monster->attackPhase = -1;
    }
    monster->Weapon_ptr->BreathObject = false;
    monster->Weapon_ptr->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 1.0f);*/
}

// -------------------------
// AnubisSkillState
// -------------------------

void AnubisSkillState::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) return;
    monster->attackPhase = 1;
    currentTrackIdx = monster->PlayAnimation(State::Attack3);
    /*monster->Weapon_ptr->BreathObject = true;
    monster->Weapon_ptr->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 50.0f);*/
    for (auto& w : monster->Weapon_ptr) {
        w->BreathObject = true;
        w->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 50.0f);
        w->SetCanCollide(true);
    }
}

void AnubisSkillState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm || !sm->animController) return;

    if (monster->attackPhase == 1) {
        if (sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished) {
            sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished = false;
            for (auto& w : monster->Weapon_ptr) {
                w->SetCanCollide(false);
            }
            sm->ChangeState(std::make_unique<IdleState>());
        }
    }
    else if (monster->attackPhase == 2) {
    }
    else if (monster->attackPhase == 3) {
    }


}

void AnubisSkillState::Exit(Monster* monster) {
    for (auto& w : monster->Weapon_ptr) {
        w->SetCanCollide(false);
        w->BreathObject = false;
        w->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
    }
    monster->attackPhase = -1;
    /*if (monster->Weapon_ptr)
    {
        monster->Weapon_ptr->SetCanCollide(false);
        monster->attackPhase = -1;
    }
    monster->Weapon_ptr->BreathObject = false;
    monster->Weapon_ptr->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 1.0f);*/
}

// -------------------------
// GargoyleSkillState
// -------------------------

void GargoyleSkillState::Enter(Monster* monster, MonsterStateMachine* sm) {
    if (!monster) return;
    monster->attackPhase = 1;
    currentTrackIdx = monster->PlayAnimation(State::Attack2);
    /*monster->Weapon_ptr->BreathObject = true;
    monster->Weapon_ptr->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 50.0f);*/
    for (auto& w : monster->Weapon_ptr) {
        w->BreathObject = true;
        //w->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 50.0f);
        //w->SetCanCollide(true);
    }
}

void GargoyleSkillState::Update(Monster* monster, float deltaTime, MonsterStateMachine* sm) {
    if (!monster || !sm || !sm->animController) return;

    if (monster->attackPhase == 1) {
        if (sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished) {
            sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished = false;
            for (auto& w : monster->Weapon_ptr) {
                w->SetCanCollide(true);
                float scale = 1.5f;
                w->CustomOBBScale = XMFLOAT3(0.4f * scale, 1.0f * scale, 4.0f * scale);
            }
            currentTrackIdx = TRACK_GARGOYLE_SKILL_2;
            monster->currStateTrackIdx = TRACK_GARGOYLE_SKILL_2;
            
            for (int i = 0; i < monster->n_Animation; ++i) {
                monster->targetWeights[i] = 0.0f;
            }
            monster->targetWeights[TRACK_GARGOYLE_SKILL_2] = 1.0f;
            sm->animController->m_pAnimationTracks[TRACK_GARGOYLE_SKILL_2].m_bFinished = false;
            sm->animController->m_pAnimationTracks[TRACK_GARGOYLE_SKILL_2].m_fPosition = 0.0f;
            
            monster->attackPhase = 2;
        }
    }
    else if (monster->attackPhase == 2) {
        static float timer = 0.0f;
        timer += deltaTime;
        if (timer >= 3.0f) {
            currentTrackIdx = TRACK_GARGOYLE_SKILL_3;
            monster->currStateTrackIdx = TRACK_GARGOYLE_SKILL_3;

            for (int i = 0; i < monster->n_Animation; ++i) {
                monster->targetWeights[i] = 0.0f;
            }
            monster->targetWeights[TRACK_GARGOYLE_SKILL_3] = 1.0f;
            sm->animController->m_pAnimationTracks[TRACK_GARGOYLE_SKILL_3].m_bFinished = false;
            sm->animController->m_pAnimationTracks[TRACK_GARGOYLE_SKILL_3].m_fPosition = 0.0f;
            timer = 0.0f;
            monster->attackPhase = 3;
        }
    }
    else if (monster->attackPhase == 3) {
        if (sm->animController->m_pAnimationTracks[currentTrackIdx].m_bFinished) {
            sm->ChangeState(std::make_unique<IdleState>());
        }
    }


}

void GargoyleSkillState::Exit(Monster* monster) {
    for (auto& w : monster->Weapon_ptr) {
        w->SetCanCollide(false);
        w->BreathObject = false;
        w->CustomOBBScale = XMFLOAT3(0.4f, 1.0f, 4.0f);
    }
    monster->attackPhase = -1;
    /*if (monster->Weapon_ptr)
    {
        monster->Weapon_ptr->SetCanCollide(false);
        monster->attackPhase = -1;
    }
    monster->Weapon_ptr->BreathObject = false;
    monster->Weapon_ptr->CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 1.0f);*/
}
