#include "stdafx.h"
#include "Player.h"
#include "AnimationRegistry.h"

Player::Player(int playerId) : Skinned_GameObject()
{
    model_index = playerId;
    player_state = Player_State::Idle;

    m_localOBB = std::make_shared<BoundingOrientedBox>(
        XMFLOAT3(0.0f, 8.0f, 0.0f),
        XMFLOAT3(4.0f, 8.0f, 4.0f),
        XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
    );

    m_worldOBB = std::make_shared<BoundingOrientedBox>();

    //type = Monster_Type::ETC;
    SetType(Object_Type::player);
    WeaponName = "SM_Wep_Cutlass_01";
    RootMotionTrackSet = {
    };

    std::unordered_set<int> OnceType = {
        TRACK_ATTACK1
    };

    InitAnimationController("Model/Captain_v17.bin", 17, 2, OnceType);
    //SetScale(10.0f, 10.0f, 10.0f);
    //m_StateMachine = std::make_unique<FishManStateMachine>(this);
    //InitStateMachine();
}

void Player::key_input(uint32_t keyState)
{

}

void Player::animate(float Elapsedtime)
{
   
}

//void Player::update(float deltaTime)
//{
//
//}

void Player::UpdateWorldOBB()
{
    if (!m_localOBB || !m_worldOBB) return;

    XMMATRIX worldMatrix = XMLoadFloat4x4(&m_xmf4x4World);
    m_localOBB->Transform(*m_worldOBB, worldMatrix);
}

void Player::Set_Collider_OBB_Center(const XMFLOAT3& newWorldCenter)
{
    const XMFLOAT3& offset = m_localOBB->Center;

    XMFLOAT3 newPos = {
        newWorldCenter.x - offset.x,
        newWorldCenter.y - offset.y,
        newWorldCenter.z - offset.z
    };

    SetPosition(newPos);
    UpdateWorldOBB(); 
}

void Player::InitAnimationController(const std::string& filepath, int animCount, int rootIdx, const std::unordered_set<int>& onceTracks)
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

    m_pSkinnedAnimationController->m_pAnimationTracks[AnimationRegistry::GetPlayerAnimationTrack(State::Idle)].m_fWeight = 1.0f;
}