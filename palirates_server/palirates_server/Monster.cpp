#include "stdafx.h"
#include "Monster.h"
#include "Object_StateMachine.h"
#include "MonsterState.h"
#include "MonsterAnimationRegistry.h"
#include <unordered_set>
#include <array>

Monster::Monster(int id) : monster_id(id) {
    type = Monster_Type::ETC;
}

void Monster::update(float deltaTime) {
    if (m_StateMachine) {
        m_StateMachine->OnPrepareUpdate(deltaTime);
        m_StateMachine->update(deltaTime);
        m_StateMachine->SetWeight(deltaTime);
    }
    if (m_pSkinnedAnimationController) {
        m_pSkinnedAnimationController->AdvanceTime(deltaTime, this);
        /*if (type == Monster_Type::ETC) {
            std::cout << "test player AdvanceTime" << std::endl;
        }*/
    }
}

void Monster::PlayAnimation(State state) {
    //if (!m_pSkinnedAnimationController) return;

    int track = MonsterAnimationRegistry::GetAnimationTrack(type, state);

    if (track >= 0 && track < n_Animation) {
        for (int i = 0; i < n_Animation; ++i) {
            targetWeights[i] = 0.0f;
        }
        targetWeights[track] = 1.0f;

        auto& animTrack = m_pSkinnedAnimationController->m_pAnimationTracks[track];
        if (animTrack.m_nType == ANIMATION_TYPE_ONCE) {
            animTrack.m_bFinished = false;
            animTrack.m_fPosition = 0.0f;  
        }
    }
}

ServerSyncData Monster::MakeSyncData() {
    ServerSyncData data;
    data.position = GetPosition();
    data.lookVector = GetLook();
    if (m_pSkinnedAnimationController) {
        data.track_info_list = m_pSkinnedAnimationController->MakeSyncData();
    }
    return data;
}

GameObject* Monster::FindNearestPlayerInRange(float range) {
    if (!pPlayerList) return nullptr;

    const float rangeSq = range * range;
    float minDistSq = rangeSq;
    GameObject* nearest = nullptr;
    const XMFLOAT3 myPos = GetPosition();

    for (const auto& player : *pPlayerList) {
        if (!player || !player->Get_Active()) continue; 

        const XMFLOAT3 pPos = player->GetPosition();

        const float dx = pPos.x - myPos.x;
        const float dy = pPos.y - myPos.y;
        const float dz = pPos.z - myPos.z;
        const float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearest = player.get();
        }
    }

    return nearest;
}

void Monster::SetTarget(GameObject* target) {
}

void Monster::StartAttackCooldown() {
    stateElapsedTime = 0.0f;
}

bool Monster::IsAttackCooldownOver() const {
    return stateElapsedTime >= stateChangeInterval;
}

void Monster::InitAnimationController(const std::string& filepath, int animCount, int rootIdx, const std::unordered_set<int>& onceTracks)
{
    auto asset = GameObject::LoadGeometryAndAnimationFromFile(filepath.data());
    if (!asset || !asset->m_pAnimationSets) return;

    n_Animation = animCount;
    RootIndex = rootIdx;

    prevWeights.assign(n_Animation, 0.0f);
    targetWeights.assign(n_Animation, 0.0f);

    m_pSkinnedAnimationController = std::make_shared<CAnimationController>(n_Animation, asset);
    m_pSkinnedAnimationController->RootIndex = RootIndex;

    for (int i = 0; i < n_Animation; ++i) {
        m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
        m_pSkinnedAnimationController->SetTrackEnable(i, true);

        if (onceTracks.find(i) != onceTracks.end()) {
            m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
        }
    }

    m_pSkinnedAnimationController->m_pAnimationTracks[MonsterAnimationRegistry::GetAnimationTrack(type, State::Idle)].m_fWeight = 1.0f;
}

void Monster::InitStateMachine() {
    if (!m_StateMachine) return;

    m_StateMachine->animController = m_pSkinnedAnimationController;
    m_StateMachine->n_Ani = n_Animation;

    if (auto state = m_StateMachine->GetCurrentState())
        state->Enter(this, m_StateMachine.get());
}

// ---------------- Fishman ----------------

Fishman::Fishman(int id) : Monster(id) {
    type = Monster_Type::Fishman;

    RootMotionTrackSet = {
        TRACK_FISHMAN_WALK,
        TRACK_FISHMAN_WALK_BACK,
        TRACK_FISHMAN_ATTACK1,
        TRACK_FISHMAN_ATTACK2,
        TRACK_FISHMAN_GET_HIT,
        TRACK_FISHMAN_DEAD
    };

    std::unordered_set<int> OnceType = {
        TRACK_FISHMAN_ATTACK1,
        TRACK_FISHMAN_ATTACK2,
        TRACK_FISHMAN_GET_HIT,
        TRACK_FISHMAN_DEAD
    };

    InitAnimationController("Model/FishmanLP.bin", 9, 0, OnceType);

    m_StateMachine = std::make_unique<FishManStateMachine>(this);
    InitStateMachine();
}

// ---------------- Anubis ----------------

Anubis::Anubis(int id) : Monster(id) {
    type = Monster_Type::Anubis;

    RootMotionTrackSet = {
        TRACK_ANUBIS_IDLE,
        TRACK_ANUBIS_IDLE_BREAK,
        TRACK_ANUBIS_IDLE_TO_ATTACK_IDLE,
        TRACK_ANUBIS_WALK,
        TRACK_ANUBIS_BACK_WALK,
        TRACK_ANUBIS_ATTACK1,
        TRACK_ANUBIS_ATTACK2,
        TRACK_ANUBIS_SKILL,
        TRACK_ANUBIS_GET_HIT,
        TRACK_ANUBIS_DEAD
    };

    std::unordered_set<int> OnceType = {
        TRACK_ANUBIS_ATTACK1,
        TRACK_ANUBIS_ATTACK2,
        TRACK_ANUBIS_SKILL,
        TRACK_ANUBIS_GET_HIT,
        TRACK_ANUBIS_DEAD
    };

    InitAnimationController("Model/Anubis_lp.bin", 10, 0, OnceType);

    m_StateMachine = std::make_unique<FishManStateMachine>(this);
    InitStateMachine();
}

// ---------------- Dragon ----------------

Dragon::Dragon(int id) : Monster(id) {
    type = Monster_Type::Dragon;

    RootMotionTrackSet = {
        TRACK_DRAGON_ATTACK1,
        TRACK_DRAGON_RUN,
        TRACK_DRAGON_GOT_HIT1,
        TRACK_DRAGON_GOT_HIT2,
        TRACK_DRAGON_FLY_DIVE,
        TRACK_DRAGON_DEAD
    };

    std::unordered_set<int> OnceType = {
        TRACK_DRAGON_ATTACK1,
        TRACK_DRAGON_DEAD
    };

    InitAnimationController("Model/Dragon_LP.bin", 13, 16, OnceType);

    m_StateMachine = std::make_unique<FishManStateMachine>(this);
    InitStateMachine();
}

// ---------------- Test ----------------

TestPlayer::TestPlayer(int id) : Monster(id) {
    type = Monster_Type::ETC;

    RootMotionTrackSet = {
    };

    std::unordered_set<int> OnceType = {
    };

    InitAnimationController("Model/Captain_v17.bin", 17, 2, OnceType);

    m_StateMachine = std::make_unique<FishManStateMachine>(this);
    InitStateMachine();
}
