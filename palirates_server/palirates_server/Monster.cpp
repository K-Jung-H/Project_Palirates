#include "stdafx.h"
#include "Monster.h"
#include "Object_StateMachine.h"


Monster::Monster(int id) : monster_id(id)
{

}

Monster::Monster()
{
    type = Monster_Type::ETC;

}

ServerSyncData Monster::MakeSyncData()
{
    ServerSyncData data;
    data.position = GetPosition();
    data.lookVector = GetLook();
    if (GetSkinnedAnimationController()) {
        data.track_info_list = GetSkinnedAnimationController()->MakeSyncData();
    }
    return data;
}

Fishman::Fishman(int id) : Monster(id)
{
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

    Monster_Type::Fishman;
    n_Animation = 9;
    RootIndex = 0;

    prevWeights.resize(n_Animation, 0.0f);
    prevWeights[0] = 1.0f;
    targetWeights.resize(n_Animation, 0.0f);

    auto asset = GameObject::LoadGeometryAndAnimationFromFile("Model/FishmanLP.bin");
    if (!asset || !asset->m_pAnimationSets) {
        return;
    }

    m_pSkinnedAnimationController = std::make_shared<CAnimationController>(n_Animation, asset);
    m_pSkinnedAnimationController->RootIndex = RootIndex;
    for (int i = 0; i < n_Animation; ++i) {
        m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
        m_pSkinnedAnimationController->SetTrackEnable(i, true);
    }
    for (int i = 0; i < n_Animation; ++i) {
        if (OnceType.find(i) != OnceType.end()) {
            m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
        }
    }
}

Anubis::Anubis(int id) : Monster(id)
{
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

    Monster_Type::Anubis;
    n_Animation = 10;
    RootIndex = 0;

    prevWeights.resize(n_Animation, 0.0f);
    prevWeights[0] = 1.0f;
    targetWeights.resize(n_Animation, 0.0f);

    auto asset = GameObject::LoadGeometryAndAnimationFromFile("Model/Anubis_lp.bin");
    if (!asset || !asset->m_pAnimationSets) {
        return;
    }

    m_pSkinnedAnimationController = std::make_shared<CAnimationController>(n_Animation, asset);
    m_pSkinnedAnimationController->RootIndex = RootIndex;
    for (int i = 0; i < n_Animation; ++i) {
        m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
        m_pSkinnedAnimationController->SetTrackEnable(i, true);
    }
    for (int i = 0; i < n_Animation; ++i) {
        if (OnceType.find(i) != OnceType.end()) {
            m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
        }
    }
}

Dragon::Dragon(int id) : Monster(id)
{
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

    Monster_Type::Dragon;
    n_Animation = 13;
    RootIndex = 16;

    prevWeights.resize(n_Animation, 0.0f);
    prevWeights[0] = 1.0f;
    targetWeights.resize(n_Animation, 0.0f);

    auto asset = GameObject::LoadGeometryAndAnimationFromFile("Model/Dragon_LP.bin");
    if (!asset || !asset->m_pAnimationSets) {
        return;
    }

    m_pSkinnedAnimationController = std::make_shared<CAnimationController>(n_Animation, asset);
    m_pSkinnedAnimationController->RootIndex = RootIndex;
    for (int i = 0; i < n_Animation; ++i) {
        m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
        m_pSkinnedAnimationController->SetTrackEnable(i, true);
    }
    for (int i = 0; i < n_Animation; ++i) {
        if (OnceType.find(i) != OnceType.end()) {
            m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
        }
    }
}