#include "stdafx.h"
#include "Player.h"
#include "AnimationRegistry.h"
#include "PlayerStateMachine.h"
#include "Scene.h"

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
        //TRACK_IDLE,
        TRACK_RUN_FORWARD_LEFT,
        TRACK_RUN_FORWARD,
        TRACK_RUN_FORWARD_RIGHT,
        TRACK_RUN_BACKWARD_LEFT,
        TRACK_RUN_BACKWARD,
        TRACK_RUN_BACKWARD_RIGHT,
        TRACK_RUN_LEFT,
        TRACK_RUN_RIGHT,
        TRACK_DIVEROLL_FORWARD,
        TRACK_KNOCK_DOWN,
        TRACK_GET_UP,
        TRACK_ATTACK1,
        TRACK_ATTACK2,
        TRACK_ATTACK3,
        TRACK_GET_HIT_F2
    };

    std::unordered_set<int> OnceType = {
        TRACK_ATTACK1,
        TRACK_ATTACK2,
        TRACK_ATTACK3,
        TRACK_KNOCK_DOWN,
        TRACK_GET_UP,
		TRACK_GET_HIT_F2,
        TRACK_DIVEROLL_FORWARD
    };

    for (int i = 0; i < n_Animation; ++i) {
        if (OnceType.find(i) != OnceType.end()) {
            m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
        }
    }

    InitAnimationController("Model/Captain_v17.bin", 17, 2, OnceType);
   
    m_StateMachine = std::make_unique<PlayerStateMachine>(this);
    InitStateMachine();
    m_fScale = 10.0f;
    //SetScale(m_fScale, m_fScale, m_fScale);
 /*   auto body = std::make_shared<BoundingOrientedBox>(
        XMFLOAT3(0.0f, 8.0f, 0.0f),
        XMFLOAT3(4.0f, 8.0f, 4.0f),
        XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    Set_Collider_OBB(body);*/
}

enum class RunDir : int { N = 0, NE = 1, E = 2, SE = 3, S = 4, SW = 5, W = 6, NW = 7 };

inline bool BuildMoveVector(uint32_t keyState, XMFLOAT2& out) {
    int vx = 0, vz = 0; // x: 오른쪽(+), z: 앞(+)
    if (keyState & INPUT_D) vx += 1;
    if (keyState & INPUT_A) vx -= 1;
    if (keyState & INPUT_W) vz += 1;
    if (keyState & INPUT_S) vz -= 1;
    out = XMFLOAT2((float)vx, (float)vz);
    return (vx != 0 || vz != 0);
}

inline RunDir Quantize8Way(const XMFLOAT2& v) {
    float len2 = v.x * v.x + v.y * v.y;
    if (len2 <= 1e-6f) return RunDir::N; 

    float ang = XMConvertToDegrees(atan2f(v.x, v.y)); 
 
    if (ang < 0.0f) ang += 360.0f;

    int sector = int(std::round(ang / 45.0f)) % 8; 
    return static_cast<RunDir>(sector);
}

static const int kRunTrackByDir[8] = {
    TRACK_RUN_FORWARD,  // N  (0)
    TRACK_RUN_FORWARD_RIGHT, // NE (1)
    TRACK_RUN_RIGHT,  // E  (2)
    TRACK_RUN_BACKWARD_RIGHT, // SE (3)
    TRACK_RUN_BACKWARD,  // S  (4)
    TRACK_RUN_BACKWARD_LEFT, // SW (5)
    TRACK_RUN_LEFT,  // W  (6)
    TRACK_RUN_FORWARD_LEFT  // NW (7)
};

void Player::key_input(uint32_t keyState)
{
   // cout << keyState << "\n";


    if (keyState == INPUT_NONE)
    {
       // cout << "key none" << "\n";
        if (GetStateMachine()->GetCurrentState()->GetStateEnum() == State::Run) {
            cout << "change normal" << "\n";
            GetStateMachine()->ChangeState(std::make_unique<PlayerNormalState>());
        }
    }

    constexpr uint32_t MOVE_MASK = (INPUT_W | INPUT_A | INPUT_S | INPUT_D);
    uint32_t moveMask = keyState & MOVE_MASK;

    if (moveMask) {
        XMFLOAT2 mv;
        if (BuildMoveVector(keyState, mv)) {
            RunDir dir = Quantize8Way(mv);
            int track = kRunTrackByDir[(int)dir];
           //cout << track << "\n";
            auto sm = GetStateMachine();
            if (sm->GetCurrentState()->GetStateEnum() != State::Run) {
                sm->ChangeState(std::make_unique<PlayerRunState>());
                SetRunDirectionTrack(track);
                sm->lastMoveMask = moveMask;
            }
            else {
                if (sm->lastMoveMask != moveMask) {           
                    SetRunDirectionTrack(track);
                    sm->lastMoveMask = moveMask;
                }
            }
            return;
        }
    }
    else {
        GetStateMachine()->lastMoveMask = 0; // 입력 끊김 → 캐시 리셋
    }

    if (keyState & INPUT_Q)
    {
        motion_blur = !motion_blur;
    }

    if (keyState & INPUT_SHIFT)
    {
        cout << "input" << "\n";
       // if (GetStateMachine()->GetCurrentState() == std::make_unique<PlayerDiveState>)
        GetStateMachine()->ChangeState(std::make_unique<PlayerDiveState>());
    }

    if (keyState & INPUT_MOUSE_LEFT)
    {
        cout << "input" << "\n";
        // if (GetStateMachine()->GetCurrentState() == std::make_unique<PlayerDiveState>)
        GetStateMachine()->ChangeState(std::make_unique<PlayerAttack1State>());
    }

    if (keyState & INPUT_MOUSE_RIGHT)
    {
        cout << "input" << "\n";
        // if (GetStateMachine()->GetCurrentState() == std::make_unique<PlayerDiveState>)
        GetStateMachine()->ChangeState(std::make_unique<PlayerAttack3State>());
    }
}

void Player::animate(float Elapsedtime)
{
   
}

void Player::update(float deltaTime)
{
    if (m_StateMachine) 
    {
        // m_StateMachine->OnPrepareUpdate(deltaTime);
        m_StateMachine->update(deltaTime);
        //m_StateMachine->SetWeight(deltaTime);
    }
    //auto con = GetSkinnedAnimationController();
    //if (con) {

    //    con->AdvanceTime2(deltaTime, this);
    // //   if (con->m_pAnimationTracks[TRACK_GET_HIT_F2].m_fWeight >= 0.5)
    //  //      std::cout << con->m_pAnimationTracks[TRACK_GET_HIT_F2].m_fPosition << std::endl;
    //}

    if (bIsInvincible) {
        if (!bDead) {
            invincibleTimeRemaining += deltaTime;
            if (invincibleTimeRemaining >= invincibleDuration) {
                bIsInvincible = false;
                invincibleTimeRemaining = 0.0f;
            }
        }
    }
}

void Player::update_collision(float deltaTime, std::vector<BoundingOrientedBox> obblist) {
    if (m_pSkinnedAnimationController) {
        m_pSkinnedAnimationController->AdvanceTime2(deltaTime, this, &obblist);
    }
}

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
        m_pSkinnedAnimationController->SetTrackWeight(i, 0.0f);
        if (onceTracks.find(i) != onceTracks.end()) {
            m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
        }
    }

    m_pSkinnedAnimationController->m_pAnimationTracks[AnimationRegistry::GetPlayerAnimationTrack(State::Idle)].m_fWeight = 1.0f;
    //m_pSkinnedAnimationController->m_pAnimationTracks[AnimationRegistry::GetPlayerAnimationTrack(State::Attack2)].m_fWeight = 1.0f;
}

void Player::InitStateMachine() {
    if (!m_StateMachine) return;

    m_StateMachine->animController = m_pSkinnedAnimationController;
    m_StateMachine->n_Ani = n_Animation;

    if (auto state = m_StateMachine->GetCurrentState())
        state->Enter(this, m_StateMachine.get());
}

void Player::HitDamage(float damage) {
    if (hp - damage < 0.0f)
        hp = 0.0f;
    else hp -= damage;
}

int Player::PlayAnimation(State state) {
    int track = AnimationRegistry::GetPlayerAnimationTrack(state);
    currStateTrackIdx = track;
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

void Player::SetRunDirectionTrack(int track)
{
    for (int i = 0; i < m_pSkinnedAnimationController->m_nAnimationTracks; ++i) {
        m_pSkinnedAnimationController->m_pAnimationTracks[i].m_fWeight = 0.0f;
    }
    m_pSkinnedAnimationController->SetTrackWeight(track, 1.0f);
    if (track >= 0 && track < n_Animation) {
        for (int i = 0; i < n_Animation; ++i) {
            targetWeights[i] = 0.0f;
        }
        targetWeights[track] = 1.0f;
    }
    currStateTrackIdx = track;
    cout << track << "\n";
}