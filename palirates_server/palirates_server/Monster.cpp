#include "stdafx.h"
#include "Monster.h"
#include "Object_StateMachine.h"


Monster::Monster(int id) : monster_id(id)
{
    trackPositions.resize(4, 1.0f);
    trackWeights.resize(4, 1.0f);
    trackWeights[0] = 1.0f;
}

Monster::Monster()
{
    type = Monster_Type::ETC;
    trackPositions.resize(4, 1.0f);
    trackWeights.resize(4, 1.0f);
    trackWeights[0] = 1.0f;
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

    Monster_Type::Fishman;
    n_Animation = 9;
   // RootIndex = 0;
    trackPositions.resize(n_Animation, 0.0f);
    trackWeights.resize(n_Animation, 0.0f);
    trackWeights[0] = 1.0f;

    auto asset = GameObject::LoadGeometryAndAnimationFromFile("Model/FishmanLP.bin");
    if (!asset || !asset->m_pAnimationSets) {
        std::cout << "[Init]  FishmanLP.bin 로드 실패\n";
        return;
    }

    auto* animSets = asset->m_pAnimationSets;

    std::cout << "\n[Init]  Model loaded\n";
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
    }
}

Anubis::Anubis(int id) : Monster(id)
{
    Monster_Type::Anubis;
    trackPositions.resize(9, 1.0f);
    trackWeights.resize(9, 1.0f);
    trackWeights[0] = 1.0f;
}

Dragon::Dragon(int id) : Monster(id)
{
    Monster_Type::Dragon;
    trackPositions.resize(12, 1.0f);
    trackWeights.resize(12, 1.0f);
    trackWeights[0] = 1.0f;
}