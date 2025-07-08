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
    //trackPositions.resize(n_Animation, 0.0f);
    //trackWeights.resize(n_Animation, 0.0f);
    //trackWeights[0] = 1.0f;

    prevWeights.resize(n_Animation, 0.0f);
    targetWeights.resize(n_Animation, 0.0f);

    auto asset = GameObject::LoadGeometryAndAnimationFromFile("Model/FishmanLP.bin");
    if (!asset || !asset->m_pAnimationSets) {
        std::cout << "[Init]  FishmanLP.bin �ε� ����\n";
        return;
    }

    auto* animSets = asset->m_pAnimationSets;

   /* std::cout << "\n[Init]  Model loaded\n";
    std::cout << "  Bone Count : " << animSets->m_nBoneFrames << "\n";

    for (int i = 0; i < animSets->m_nBoneFrames; ++i) {
        const auto* boneFrame = animSets->m_ppBoneFrameCaches[i];
        if (boneFrame)
            std::cout << "     [" << i << "] " << boneFrame->m_pstrFrameName << "\n";
    }

    std::cout << "  Animation Sets : "
        << animSets->m_pAnimationSet_list.size() << "\n";

    for (size_t i = 0; i < animSets->m_pAnimationSet_list.size(); ++i) {
        const auto& set = animSets->m_pAnimationSet_list[i];
        if (!set) continue;

        std::cout << "     [" << i << "] "
            << set->m_pstrAnimationSetName
            << "   (keyFrames=" << set->m_nKeyFrames
            << ", length=" << set->m_fLength << "s)\n";
    }*/

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
    Monster_Type::Anubis;

}

Dragon::Dragon(int id) : Monster(id)
{
    Monster_Type::Dragon;

}