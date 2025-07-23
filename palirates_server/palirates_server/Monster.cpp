#include "stdafx.h"
#include "Monster.h"
#include "Object_StateMachine.h"
#include "State.h"
#include "AnimationRegistry.h"
#include <unordered_set>
#include <array>

Monster::Monster(int id) : monster_id(id) {
    type = Monster_Type::ETC;
}

void Monster::update(float deltaTime) {
    if (m_StateMachine) {
        //if (!bGetHit) {
            m_StateMachine->OnPrepareUpdate(deltaTime);
            m_StateMachine->update(deltaTime);
            m_StateMachine->SetWeight(deltaTime);
        //}
    }
    //if (m_pSkinnedAnimationController) {
    //    m_pSkinnedAnimationController->AdvanceTime(deltaTime, this);
    //}

    if (bIsInvincible) {
        invincibleTimeRemaining += deltaTime;
        if (invincibleTimeRemaining >= invincibleDuration) {
            bIsInvincible = false;
            invincibleTimeRemaining = 0.0f;
        }
    }
}

void Monster::update_collision(float deltaTime, std::vector<BoundingOrientedBox> obblist) {
    if (m_pSkinnedAnimationController) {
        m_pSkinnedAnimationController->AdvanceTime(deltaTime, this, &obblist);
    }
}

int Monster::PlayAnimation(State state) {
    int track = AnimationRegistry::GetMonsterAnimationTrack(type, state);

    if (track >= 0 && track < n_Animation) {
        for (int i = 0; i < n_Animation; ++i) {
            targetWeights[i] = 0.0f;
        }
        targetWeights[track] = 1.0f;

        auto& animTrack = m_pSkinnedAnimationController->m_pAnimationTracks[track];
        if (animTrack.m_nType == ANIMATION_TYPE_ONCE) {
            animTrack.m_bFinished = false;
        }
        animTrack.m_fPosition = 0.0f;
    }
    stateElapsedTime = 0.0f;

    return track;
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

std::optional<XMFLOAT3> Monster::FindNearestPlayerInRange(float range) {
    if (!pPlayerList) return std::nullopt;

    const float rangeSq = range * range;
    float minDistSq = rangeSq;
    GameObject* nearest = nullptr;
    const XMFLOAT3 myPos = GetPosition();

    for (const auto& player : *pPlayerList) {
        if (!player || !player->Get_Active()) continue;

        const XMFLOAT3 pPos = player->GetPosition();
        float dx = pPos.x - myPos.x;
        float dy = pPos.y - myPos.y;
        float dz = pPos.z - myPos.z;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearest = player.get();
        }
    }

    if (!nearest) return std::nullopt;
    return nearest->GetPosition();
}

void Monster::SetTarget(const XMFLOAT3& targetPos) {
    m_targetPos = targetPos;
    m_shouldRotate = true;
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
    m_pRootModel = asset->m_pModelRootObject;
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

    m_pSkinnedAnimationController->m_pAnimationTracks[AnimationRegistry::GetMonsterAnimationTrack(type, State::Idle)].m_fWeight = 1.0f;
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
    detectionRange = 50.0f;
    attackRange = 20.0f;
    type = Monster_Type::Fishman;
    SetType(Object_Type::monster);
    WeaponName = "spear_lp";
    RootMotionTrackSet = {
        TRACK_FISHMAN_WALK,
        TRACK_FISHMAN_WALK_BACK,
       // TRACK_FISHMAN_ATTACK1,
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
    m_fScale = 10.0f;
    SetScale(m_fScale, m_fScale, m_fScale);
    auto body = std::make_shared<BoundingOrientedBox>(
        XMFLOAT3(0.0f, 0.8f, 0.0f),    
        XMFLOAT3(0.4f, 0.8f, 0.4f),   
        XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) 
    );
    Set_Collider_OBB(body);
}

// ---------------- Anubis ----------------

Anubis::Anubis(int id) : Monster(id) {
    detectionRange = 50.0f;
    attackRange = 20.0f;
    type = Monster_Type::Anubis;
    SetType(Object_Type::monster);
    WeaponName = "Staff_LP";
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
    m_fScale = 15.0f;
    SetScale(m_fScale, m_fScale, m_fScale);
    auto body = std::make_shared<BoundingOrientedBox>(
        XMFLOAT3(0.0f, 0.9f, 0.0f),
        XMFLOAT3(0.3f, 0.9f, 0.3f),
        XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    Set_Collider_OBB(body);
}

// ---------------- Dragon ----------------

Dragon::Dragon(int id) : Monster(id) {
    detectionRange = 100.0f;
    attackRange = 50.0f;
    type = Monster_Type::Dragon;
    SetType(Object_Type::monster);
    WeaponName = "HeadA_LP";
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
        TRACK_DRAGON_BREATHE,
        TRACK_DRAGON_GOT_HIT1,
        TRACK_DRAGON_GOT_HIT2,
        TRACK_DRAGON_DEAD
    };

    InitAnimationController("Model/Dragon_LP.bin", 13, 16, OnceType);

    m_StateMachine = std::make_unique<FishManStateMachine>(this);
    InitStateMachine();
    m_fScale = 15.0f;
    SetScale(m_fScale, m_fScale, m_fScale);
   /* auto body = std::make_shared<BoundingOrientedBox>(
        XMFLOAT3(0.0f, 0.8f, 0.0f),
        XMFLOAT3(0.4f, 0.8f, 0.4f),
        XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    Set_Collider_OBB(body);*/
}

// ---------------- Test ----------------

TestPlayer::TestPlayer(int id) : Monster(id) {
    type = Monster_Type::ETC;
    SetType(Object_Type::monster);
    WeaponName = "SM_Wep_Cutlass_01";
    RootMotionTrackSet = {
    };

    std::unordered_set<int> OnceType = {
        TRACK_ATTACK1
    };

    InitAnimationController("Model/Captain_v17.bin", 17, 2, OnceType);
    m_fScale = 10.0f;
    SetScale(m_fScale, m_fScale, m_fScale);
    m_StateMachine = std::make_unique<FishManStateMachine>(this);
    InitStateMachine();
}
