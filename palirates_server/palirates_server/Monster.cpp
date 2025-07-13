#include "stdafx.h"
#include "Monster.h"
#include "Object_StateMachine.h"
#include "MonsterState.h"
#include "MonsterAnimationRegistry.h"
#include <unordered_set>

Monster::Monster(int id) : monster_id(id) {
    type = Monster_Type::ETC;
}

void Monster::update(float deltaTime) {
    if (m_StateMachine) {
        m_StateMachine->SetWeight(deltaTime);
        m_StateMachine->update(stateElapsedTime);
    }
    if (m_pSkinnedAnimationController) {
        m_pSkinnedAnimationController->AdvanceTime(deltaTime, this);
    }
}

void Monster::PlayAnimation(State state) {
    if (!m_pSkinnedAnimationController) return;

    int track = MonsterAnimationRegistry::GetAnimationTrack(type, state);

    if (track >= 0 && track < n_Animation) {
        for (int i = 0; i < n_Animation; ++i) {
            targetWeights[i] = 0.0f;
        }
        targetWeights[track] = 1.0f;
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
    return nullptr;
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
    prevWeights[0] = 1.0f;

    m_pSkinnedAnimationController = std::make_shared<CAnimationController>(n_Animation, asset);
    m_pSkinnedAnimationController->RootIndex = RootIndex;

    for (int i = 0; i < n_Animation; ++i) {
        m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
        m_pSkinnedAnimationController->SetTrackEnable(i, true);

        if (onceTracks.find(i) != onceTracks.end()) {
            m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
        }
    }
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

    m_StateMachine = std::make_unique<FishManStateMachine>(this);

    InitAnimationController("Model/FishmanLP.bin", 9, 0, OnceType);
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

    m_StateMachine = std::make_unique<AnubisStateMachine>(this);

    InitAnimationController("Model/Anubis_lp.bin", 10, 0, OnceType);
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

    m_StateMachine = std::make_unique<DragonStateMachine>(this);

    InitAnimationController("Model/Dragon_LP.bin", 13, 16, OnceType);
}
