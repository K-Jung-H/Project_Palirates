#include "stdafx.h"
#include "Object_StateMachine.h"
#include "Player.h"
#include "Object.h"

std::map<State, std::wstring> stateToStringMap = {
    {State::Idle, L"Idle"},
    {State::Run, L"Run"},
    {State::Knock_Down, L"Knock Down"},
    {State::Get_Up, L"Get Up"},
    {State::Dive, L"Dive"},
    {State::Jump, L"Jump"},
    {State::Attack1, L"Attack"},
    {State::Attack2, L"Attack"},
    {State::Attack3, L"Attack"},
    {State::Attack_Normal, L"Attack Normal"},
    {State::ETC, L"ETC"}
};

struct Direction {
    float x, z;
    AnimationTrack track;
};

Direction directions[8] = {
    { -0.707f,  0.707f, TRACK_RUN_FORWARD_LEFT },  // Left-Front
    {  0.000f,  1.000f, TRACK_RUN_FORWARD },       // Front
    {  0.707f,  0.707f, TRACK_RUN_FORWARD_RIGHT }, // Right-Front
    { -0.707f, -0.707f, TRACK_RUN_BACKWARD_LEFT }, // Left-Back
    {  0.000f, -1.000f, TRACK_RUN_BACKWARD },      // Back
    {  0.707f, -0.707f, TRACK_RUN_BACKWARD_RIGHT },// Right-Back
    { -1.000f,  0.000f, TRACK_RUN_LEFT },          // Left
    {  1.000f,  0.000f, TRACK_RUN_RIGHT }          // Right
};


//StateMachine::StateMachine(CPlayer* owner)
//    : m_pOwner(owner), currentState(State::Idle) {
//
//    if (m_pOwner) {
//        animController = m_pOwner->GetSkinnedAnimationController();
//        n_Ani = m_pOwner->n_Animation; 
//    }
//}

void Key_State::update(Key_Value key_state)
{
    switch (key_state)
    {
    case Key_Value::Forward_Key_Down:
        forward = true;
        break;
    case Key_Value::Forward_Key_Up:
        forward = false;
        break;
    case Key_Value::Back_Key_Down:
        back = true;
        break;
    case Key_Value::Back_Key_Up:
        back = false;
        break;
    case Key_Value::Left_Key_Down:
        left = true;
        break;
    case Key_Value::Left_Key_Up:
        left = false;
        break;
    case Key_Value::Right_Key_Down:
        right = true;
        break;
    case Key_Value::Right_Key_Up:
        right = false;
        break;

    case Key_Value::Jump_Key_Down:
        break;
    case Key_Value::Jump_Key_Up:
        break;

    case Key_Value::Dive_Key_Down:
        dive = true;
        break;
    case Key_Value::Dive_Key_Up:
        break;
    case Key_Value::Attack1_Key_Down:
        attack1 = true;
        break;
    case Key_Value::Attack1_Key_Up:
        break;
    case Key_Value::Attack2_Key_Down:
        attack2 = true;
        break;
    case Key_Value::Attack2_Key_Up:
        break;
    case Key_Value::Attack3_Key_Down:
        attack3 = true;
        break;
    case Key_Value::Attack3_Key_Up:
        break;
    case Key_Value::None:
    case Key_Value::ETC:
    default:
        break;
    }
}

bool Key_State::check_move()
{
    return (left != right) || (forward != back);
}

//========================================================

void StateMachine::start()
{
    enterState(currentState, Key_Value::None);
}

void StateMachine::update(float Elapsed_time)
{

    doAction(currentState, Elapsed_time);
}

void StateMachine::handleEvent(UCHAR* pKeysBuffer)
{
    if (this == nullptr)
    {
        DebugOutput("this == nullptr\n");

        return;
    }

    std::unordered_map<int, std::pair<Key_Value, Key_Value>> keyMappings = {
    { VK_UP,    { Key_Value::Forward_Key_Down, Key_Value::Forward_Key_Up } },
    { 0x57,     { Key_Value::Forward_Key_Down, Key_Value::Forward_Key_Up } },

    { VK_DOWN,  { Key_Value::Back_Key_Down, Key_Value::Back_Key_Up } },
    { 0x53,     { Key_Value::Back_Key_Down, Key_Value::Back_Key_Up } },

    { VK_LEFT,  { Key_Value::Left_Key_Down, Key_Value::Left_Key_Up } },
    { 0x41,     { Key_Value::Left_Key_Down, Key_Value::Left_Key_Up } },

    { VK_RIGHT, { Key_Value::Right_Key_Down, Key_Value::Right_Key_Up } },
    { 0x44,     { Key_Value::Right_Key_Down, Key_Value::Right_Key_Up } },

    { VK_SPACE, { Key_Value::Jump_Key_Down, Key_Value::Jump_Key_Up } },
    { VK_SHIFT, { Key_Value::Dive_Key_Down, Key_Value::Dive_Key_Up } },

    { VK_OEM_COMMA, { Key_Value::Attack1_Key_Down, Key_Value::Attack1_Key_Up } },
    { VK_OEM_PERIOD, { Key_Value::Attack2_Key_Down, Key_Value::Attack2_Key_Up } },
    { VK_OEM_2, { Key_Value::Attack3_Key_Down, Key_Value::Attack3_Key_Up } }
    };

    for (const auto& [key, keyPair] : keyMappings)
    {
        key_state.update(pKeysBuffer[key] & 0xF0 ? keyPair.first : keyPair.second);
    }

    switch (currentState)
    {
    case State::Idle:

        break;

    case State::Run:
        break;
    case State::Jump:
        // 점프 상태에서 키 입력 처리
        break;

    case State::Attack_Normal:
        // 공격 상태에서 키 입력 처리
        break;

    default:
        break;
    }
}

void StateMachine::changeState(State newState, Key_Value key_event)
{
    lastState = currentState;
    exitState(currentState, key_event);
    currentState = newState;
    enterState(currentState, key_event);
}

void StateMachine::enterState(State state, Key_Value key_event)
{

    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Dive:
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;

    default:
        break;
    }
}

void StateMachine::exitState(State state, Key_Value key_event)
{

    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Dive:
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;

    default:
        break;
    }
}

PlayerStateMachine::PlayerStateMachine(CPlayer* owner)
    : StateMachine(State::Idle), m_pOwner(owner) {
}

void PlayerStateMachine::update(float Elapsed_time)
{
    blendSpeed = 6.0f * Elapsed_time;

    if (isFirstUpdate) {
        animController = m_pOwner->GetSkinnedAnimationController();
        n_Ani = m_pOwner->n_Animation;
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = 0.0f;
            animController->SetTrackWeight(i, 0.0f);
        }
        m_pOwner->prevWeights[TRACK_IDLE] = 1.0f;
        animController->SetTrackWeight(TRACK_IDLE, 1.0f);
        isFirstUpdate = false;
    }
    else {
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = animController->m_pAnimationTracks[i].m_fWeight;
        }
    }

    std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);

    float moveX = m_pOwner->GetMoveX();
    float moveZ = m_pOwner->GetMoveZ();

    if (key_state.dive && Get_State() != State::Dive) {
        m_pOwner->SetStateElapsedTime(0.0f);
        if (key_state.right)
            m_pOwner->SetLookDirection(m_pOwner->GetRight());
        if (key_state.left) {
            XMVECTOR rightVec = XMLoadFloat3(&m_pOwner->GetRight());
            XMVECTOR leftVec = -rightVec;

            XMFLOAT3 left;
            XMStoreFloat3(&left, leftVec);
            m_pOwner->SetLookDirection(left);

        }
        changeState(State::Dive, Key_Value::None);
    }
    if (key_state.attack1 && Get_State() != State::Attack1) {
        m_pOwner->SetStateElapsedTime(0.0f);
        changeState(State::Attack1, Key_Value::None);
        m_pOwner->bTrailOn();
        m_pOwner->Weapon_ptr->bUpdateOBBOn();
    }
    if (key_state.attack2 && Get_State() != State::Attack2) {
        m_pOwner->SetStateElapsedTime(0.0f);
        changeState(State::Attack2, Key_Value::None);
        m_pOwner->bTrailOn();
        m_pOwner->Weapon_ptr->bUpdateOBBOn();
    }
    if (key_state.attack3 && Get_State() != State::Attack3) {
        m_pOwner->SetStateElapsedTime(0.0f);
        changeState(State::Attack3, Key_Value::None);
        m_pOwner->bTrailOn();
        m_pOwner->Weapon_ptr->bUpdateOBBOn();
    }

    switch (Get_State()) {
    case State::Idle:
        if (moveX == 0.0f && moveZ == 0.0f) {
            m_pOwner->targetWeights[TRACK_IDLE] = 1.0f;
        }
        else {
            changeState(State::Run, Key_Value::None);
        }
        break;

    case State::Run:
        if (moveX == 0.0f && moveZ == 0.0f) {
            changeState(State::Idle, Key_Value::None);
        }
        else {
            float length = sqrtf(moveX * moveX + moveZ * moveZ);
            float normX = moveX / length;
            float normZ = moveZ / length;

            int bestIndex = -1, secondIndex = -1;
            float bestDot = -1.0f, secondDot = -1.0f;

            for (int i = 0; i < 8; i++) {
                float dot = normX * directions[i].x + normZ * directions[i].z;
                if (dot > bestDot) {
                    secondDot = bestDot;
                    secondIndex = bestIndex;
                    bestDot = dot;
                    bestIndex = i;
                }
                else if (dot > secondDot) {
                    secondDot = dot;
                    secondIndex = i;
                }
            }

            float totalDot = bestDot + secondDot;
            float weight1 = bestDot / totalDot;
            float weight2 = secondDot / totalDot;

            m_pOwner->targetWeights[directions[bestIndex].track] = weight1;
            m_pOwner->targetWeights[directions[secondIndex].track] = weight2;
        }
        break;
    case State::Dive:

        if (animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        else {
            m_pOwner->targetWeights[TRACK_DIVEROLL_FORWARD] = 1.0f;
            RootMotionMove(30.0f);
        }
        break;
    case State::Knock_Down:
        m_pOwner->targetWeights[TRACK_KNOCK_DOWN] = 1.0f;
        RootMotionMove(30.0f, true);
        break;
    case State::Get_Up:
        if (animController->m_pAnimationTracks[TRACK_GET_UP].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_GET_UP] = 1.0f;
        RootMotionMove(10.0f);
        break;
    case State::Attack1:
        if (animController->m_pAnimationTracks[TRACK_ATTACK1].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_ATTACK1] = 1.0f;
        RootMotionMove(30.0f);
        break;
    case State::Attack2:
        if (animController->m_pAnimationTracks[TRACK_ATTACK2].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_ATTACK2] = 1.0f;
        RootMotionMove(30.0f);
        break;
    case State::Attack3:
        if (animController->m_pAnimationTracks[TRACK_ATTACK3].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_ATTACK3] = 1.0f;
        RootMotionMove(30.0f);
        break;
    case State::Get_Hit_F2:
        if (animController->m_pAnimationTracks[TRACK_GET_HIT_F2].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_GET_HIT_F2] = 1.0f;
        RootMotionMove(30.0f, true);
        break;
    }

    SetWeight();
}


void PlayerStateMachine::enterState(State state, Key_Value key_event)
{
    if (GetRootMotionStates().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = false;
            m_pOwner->SetStateElapsedTime(0.0f);
        }

        ResetTrackForState(state, true);
    }

    switch (state)
    {
    case State::Idle:
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }
        break;
    default:
        break;
    }
}

void PlayerStateMachine::exitState(State state, Key_Value key_event)
{

    if (GetRootMotionStates().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }

        ResetTrackForState(state, false);

    }

    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Dive:
        key_state.dive = false;
        break;
    case State::Knock_Down:
        break;
    case State::Get_Up:
        break;
    case State::Attack1:
        key_state.attack1 = false;
        m_pOwner->bTrailOff();
        m_pOwner->GetTrailObj()->Set_Active(false);
        m_pOwner->Weapon_ptr->bUpdateOBBOff();
        break;
    case State::Attack2:
        key_state.attack2 = false;
        m_pOwner->bTrailOff();
        m_pOwner->GetTrailObj()->Set_Active(false);
        m_pOwner->Weapon_ptr->bUpdateOBBOff();
        break;
    case State::Attack3:
        key_state.attack3 = false;
        m_pOwner->bTrailOff();
        m_pOwner->GetTrailObj()->Set_Active(false);
        m_pOwner->Weapon_ptr->bUpdateOBBOff();
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;

    default:
        break;
    }
}

void PlayerStateMachine::RootMotionMove(float scaleFactor, bool bUseNegative) {

    XMFLOAT3 vec = animController->HipsPosition;
    XMFLOAT3 vec2 = animController->m_xmf3PrevHipsPosition;

    XMFLOAT3 shift;
    shift.x = vec.x - vec2.x;
    shift.y = vec.y - vec2.y;
    shift.z = vec.z - vec2.z;

    animController->m_xmf3PrevHipsPosition = vec;
    XMFLOAT3 scaleShift = { shift.x * scaleFactor, shift.y * scaleFactor, shift.z * scaleFactor };

    XMFLOAT3 moveDirection = m_pOwner->GetLook();
    XMFLOAT3 finalMove = {
        moveDirection.x * scaleShift.z,
        moveDirection.y * scaleShift.z,
        moveDirection.z * scaleShift.z
    };

    if (bUseNegative) {
        finalMove.x = -1 * finalMove.x;
        finalMove.z = -1 * finalMove.z;
    }

    if (scaleShift.z > 0.001f) {
        m_pOwner->Move(finalMove, false);
    }
}

void PlayerStateMachine::ResetTrackForState(State state, bool posReset)
{
    auto it = GetRootMotionStateToTrackMap().find(state);
    if (it != GetRootMotionStateToTrackMap().end()) {
        int track = it->second;
        if (animController != nullptr) {
            animController->m_pAnimationTracks[track].m_bFinished = false;
            if (posReset)
                animController->m_pAnimationTracks[track].m_fPosition = 0.0f;
        }
    }
}

void PlayerStateMachine::SetWeight()
{
    for (int i = 0; i < n_Ani; i++)
    {
        float prev = m_pOwner->prevWeights[i];
        float target = m_pOwner->targetWeights[i];

        if (fabs(prev - target) == 0.0f)
            continue;

        float newWeight = prev + (target - prev) * blendSpeed;
        animController->SetTrackWeight(i, newWeight);
    }
}

////////////////////////////////////////////////////////////////////////////

MultiPlayerStateMachine::MultiPlayerStateMachine(std::shared_ptr<CTerrainPlayer> owner)
    : StateMachine(State::Idle), m_pOwner(owner) {
}

void MultiPlayerStateMachine::update(float Elapsed_time)
{
    float blendSpeed = 6.0f * Elapsed_time;

    switch (Get_State()) {
    case State::Idle:

        break;

    case State::Run:
        break;
    case State::Dive:

        break;
    case State::Knock_Down:
        break;
    case State::Get_Up:
        break;
    }

    for (int i = 0; i < n_Ani; i++)
    {
        float prev = m_pOwner->prevWeights[i];
        float target = m_pOwner->targetWeights[i];

        if (fabs(prev - target) < 0.0001f)
            continue;

        float newWeight = prev + (target - prev) * blendSpeed;
        animController->SetTrackWeight(i, newWeight);
    }

    isFirstUpdate = false;

    //doAction(currentState, Elapsed_time);
}

void MultiPlayerStateMachine::server_update(XMFLOAT3 pos, XMFLOAT3 look, State state, float ble_pos1, float ble_pos2)
{

}

void MultiPlayerStateMachine::enterState(State state, Key_Value key_event)
{

    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Dive:
        animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished = false;
        animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_fPosition = 0;
        break;
    case State::Knock_Down:
        animController->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_bFinished = false;
        animController->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_fPosition = 0;
        break;
    case State::Get_Up:
        animController->m_pAnimationTracks[TRACK_GET_UP].m_bFinished = false;
        animController->m_pAnimationTracks[TRACK_GET_UP].m_fPosition = 0;
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;

    default:
        break;
    }
}

void MultiPlayerStateMachine::exitState(State state, Key_Value key_event)
{

    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Dive:
        key_state.dive = false;
        animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished = false;
        DebugOutput("Dive->Idle\n");
        break;
    case State::Knock_Down:
        animController->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_bFinished = false;
        break;
    case State::Get_Up:
        animController->m_pAnimationTracks[TRACK_GET_UP].m_bFinished = false;
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;

    default:
        break;
    }
}

////////////////////////////////////////////////////////////

MonsterStateMachine::MonsterStateMachine(CMonsterObject* owner)
    : StateMachine(State::Idle), m_pOwner(owner) {

}

void MonsterStateMachine::update(float Elapsed_time)
{
    OnPrepareUpdate(6.0f, Elapsed_time);

    if (stateElapsedTime >= stateChangeTime) {
        switch (Get_State()) {
        case State::Idle:
            changeState(State::Run, Key_Value::None);
            break;
        case State::Run:
            changeState(State::Idle, Key_Value::None);
            break;
        case State::Dive:
            break;
        }

        stateElapsedTime = 0.0f;
        stateChangeTime = 1.0f + static_cast<float>(rand() % 10);
    }

    switch (Get_State()) {
    case State::Idle:
        m_pOwner->targetWeights[TRACK_IDLE] = 1.0f;
        break;
    case State::Run:
        m_pOwner->targetWeights[6] = 1.0f;

        RootMotionMove(10.0f);

        break;
    case State::Attack2:

        break;
    }

    SetWeight();
}

void MonsterStateMachine::OnPrepareUpdate(float blendSpeedOffSet, float Elapsed_time)
{
    blendSpeed = 6.0f * Elapsed_time;

    stateElapsedTime += Elapsed_time;

    if (isFirstUpdate) {
        animController = m_pOwner->GetSkinnedAnimationController();
        n_Ani = m_pOwner->n_Animation;
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = 0.0f;
            animController->SetTrackWeight(i, 0.0f);
        }
        m_pOwner->prevWeights[TRACK_IDLE] = 1.0f;
        isFirstUpdate = false;
    }
    else {

        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = animController->m_pAnimationTracks[i].m_fWeight;
        }
    }

    std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);
}

void MonsterStateMachine::RootMotionMove(float scaleFactor, bool bUseNegative) {

    XMFLOAT3 vec = animController->HipsPosition;
    XMFLOAT3 vec2 = animController->m_xmf3PrevHipsPosition;

    XMFLOAT3 shift;
    shift.x = vec.x - vec2.x;
    shift.y = vec.y - vec2.y;
    shift.z = vec.z - vec2.z;

    animController->m_xmf3PrevHipsPosition = vec;
    XMFLOAT3 scaleShift = { shift.x * scaleFactor, shift.y * scaleFactor, shift.z * scaleFactor };

    XMFLOAT3 moveDirection = m_pOwner->GetLook();
    XMFLOAT3 finalMove = {
        moveDirection.x * scaleShift.z,
        moveDirection.y * scaleShift.z,
        moveDirection.z * scaleShift.z
    };

    if (bUseNegative) {
        finalMove.x = -1 * finalMove.x;
        finalMove.z = -1 * finalMove.z;
    }

    if (scaleShift.z > 0.001f) {
        m_pOwner->Move(finalMove);
    }
}

void MonsterStateMachine::enterState(State state, Key_Value key_event)
{

    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Dive:
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;

    default:
        break;
    }
}

void MonsterStateMachine::exitState(State state, Key_Value key_event)
{
    /* if (kAnubisRootMotionStateToTrackMap.contains(state)) {
         if (m_pOwner != nullptr) {
             m_pOwner->bIsControllable = true;
         }

         ResetTrackForState(state, false);
     }*/

    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Dive:
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;

    default:
        break;
    }
}

void MonsterStateMachine::SetWeight()
{
    for (int i = 0; i < n_Ani; i++)
    {
        float prev = m_pOwner->prevWeights[i];
        float target = m_pOwner->targetWeights[i];

        if (fabs(prev - target) == 0.0f)
            continue;

        float newWeight = prev + (target - prev) * blendSpeed;
        animController->SetTrackWeight(i, newWeight);
    }
}

void MonsterStateMachine::RotateLookToTarget(const XMFLOAT3& targetPos, float fDeltaTime, float fLerpSpeed, float fTriggerDistance)
{
    const XMFLOAT3& selfPosF3 = m_pOwner->GetPosition();

    float distance = GetDistance(selfPosF3, targetPos);
    if (distance > fTriggerDistance) return;
    if (!TargetSet) ChangeTargetSet();

    XMVECTOR selfPos = XMLoadFloat3(&selfPosF3);
    XMVECTOR target = XMLoadFloat3(&targetPos);
    XMVECTOR toTarget = XMVector3Normalize(target - selfPos);

    XMVECTOR currentLook = XMLoadFloat3(&m_pOwner->GetLook());
    XMVECTOR newLook = XMVector3Normalize(XMVectorLerp(currentLook, toTarget, fLerpSpeed * fDeltaTime));

    XMFLOAT3 finalLook;
    XMStoreFloat3(&finalLook, newLook);
    m_pOwner->SetLookDirection(finalLook);
}

void MonsterStateMachine::ChangeIfNear(State nextState, float triggerDistance)
{
    float distance = GetDistance(m_pOwner->GetPosition(), m_TargetPosition);
    if (distance > triggerDistance) return;

    XMVECTOR selfPos = XMLoadFloat3(&m_pOwner->GetPosition());
    XMVECTOR targetPos = XMLoadFloat3(&m_TargetPosition);
    XMVECTOR dir = XMVector3Normalize(targetPos - selfPos);

    dir = XMVectorSetY(dir, 0.0f);
    dir = XMVector3Normalize(dir);

    XMFLOAT3 finalLook;
    XMStoreFloat3(&finalLook, dir);
    m_pOwner->SetLookDirection(finalLook);

    changeState(nextState, Key_Value::None);
}

////////////////////////////////////////////////////////

void FishManStateMachine::update(float Elapsed_time)
{
    OnPrepareUpdate(6.0f, Elapsed_time);

    if (!GetTargetSet()) {
        if (stateElapsedTime >= stateChangeTime) {
            switch (Get_State()) {
            case State::Idle:
                changeState(State::Run, Key_Value::None);
                break;
            case State::Run:
                changeState(State::Idle, Key_Value::None);
                break;
            }

            stateElapsedTime = 0.0f;
            stateChangeTime = 1.0f + static_cast<float>(rand() % 10);
        }
    }

    switch (Get_State()) {
    case State::Idle:
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
        if (TargetSet)  changeState(State::Run, Key_Value::None);
        m_pOwner->targetWeights[TRACK_FISHMAN_IDLE] = 1.0f;
        break;
    case State::Run:
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
        m_pOwner->targetWeights[TRACK_FISHMAN_WALK] = 1.0f;
        RootMotionMove(10.0f);
        {
            std::uniform_int_distribution<int> dist(0, 1);
            State attackState = (dist(m_Rng) == 0) ? State::Attack1 : State::Attack2;
            ChangeIfNear(attackState, 20.0f);
        }
        break;
    case State::Get_Hit:
        if (animController->m_pAnimationTracks[TRACK_FISHMAN_GET_HIT].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_FISHMAN_GET_HIT] = 1.0f;
        RootMotionMove(0.0f, true);
        break;
    case State::Attack1:
        if (animController->m_pAnimationTracks[TRACK_FISHMAN_ATTACK1].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_FISHMAN_ATTACK1] = 1.0f;
        RootMotionMove(20.0f);
        break;
    case State::Attack2:
        if (animController->m_pAnimationTracks[TRACK_FISHMAN_ATTACK2].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_FISHMAN_ATTACK2] = 1.0f;
        RootMotionMove(20.0f);
        break;
    }

    SetWeight();
}

void FishManStateMachine::enterState(State state, Key_Value key_event)
{
    if (GetFishManRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = false;
        }

        ResetTrackForState(state, true);
    }
    switch (state)
    {
    default:
        break;
    }
}

void FishManStateMachine::exitState(State state, Key_Value key_event)
{
    if (GetFishManRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }

        ResetTrackForState(state, false);
    }
}

void FishManStateMachine::ResetTrackForState(State state, bool posReset)
{
    auto it = GetFishManRootMotionStateToTrackMap().find(state);
    if (it != GetFishManRootMotionStateToTrackMap().end()) {
        int track = it->second;
        if (animController != nullptr) {
            animController->m_pAnimationTracks[track].m_bFinished = false;
            if (posReset)
                animController->m_pAnimationTracks[track].m_fPosition = 0.0f;
        }
    }
}

void AnubisStateMachine::update(float Elapsed_time)
{
    OnPrepareUpdate(6.0f, Elapsed_time);

    if (!GetTargetSet()) {
        if (stateElapsedTime >= stateChangeTime) {
            switch (Get_State()) {
            case State::Idle:
                changeState(State::Run, Key_Value::None);
                break;
            case State::Run:
                changeState(State::Idle, Key_Value::None);
                break;
            }
            stateElapsedTime = 0.0f;
            stateChangeTime = 1.0f + static_cast<float>(rand() % 10);
        }
    }

    switch (Get_State()) {
    case State::Idle:
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
        m_pOwner->targetWeights[TRACK_ANUBIS_IDLE] = 1.0f;
        if (TargetSet)  changeState(State::Run, Key_Value::None);
        break;
    case State::Run:
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
        m_pOwner->targetWeights[TRACK_ANUBIS_WALK] = 1.0f;
        RootMotionMove(10.0f);
        {
            std::uniform_int_distribution<int> dist(0, 2);
            State attackState;

            switch (dist(m_Rng)) {
            case 0: attackState = State::Attack1; break;
            case 1: attackState = State::Attack2; break;
            case 2: attackState = State::Attack3; break;
            }

            ChangeIfNear(attackState, 30.0f);
        }
        break;
    case State::Get_Hit:
        if (animController->m_pAnimationTracks[TRACK_ANUBIS_GET_HIT].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_ANUBIS_GET_HIT] = 1.0f;
        RootMotionMove(20.0f, true);
        break;
    case State::Attack1:
        if (animController->m_pAnimationTracks[TRACK_ANUBIS_ATTACK1].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_ANUBIS_ATTACK1] = 1.0f;
        RootMotionMove(30.0f);
        break;
    case State::Attack2:
        if (animController->m_pAnimationTracks[TRACK_ANUBIS_ATTACK2].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_ANUBIS_ATTACK2] = 1.0f;
        RootMotionMove(30.0f);
        break;
    case State::Attack3:
        if (animController->m_pAnimationTracks[TRACK_ANUBIS_SKILL].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_ANUBIS_SKILL] = 1.0f;
        RootMotionMove(10.0f);
        break;
    }

    SetWeight();
}

void AnubisStateMachine::enterState(State state, Key_Value key_event)
{
    if (GetAnubisRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = false;
        }

        ResetTrackForState(state, true);
    }
    switch (state)
    {
    case State::Attack1:
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = 0.0f;
            m_pOwner->targetWeights[i] = 0.0f;
            animController->m_pAnimationTracks[i].m_fPosition = 0.0f;
        }
        break;
    default:
        break;
    }
}

void AnubisStateMachine::exitState(State state, Key_Value key_event)
{
    if (GetAnubisRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }

        ResetTrackForState(state, false);
    }
}

void AnubisStateMachine::ResetTrackForState(State state, bool posReset)
{
    auto it = GetAnubisRootMotionStateToTrackMap().find(state);
    if (it != GetAnubisRootMotionStateToTrackMap().end()) {
        int track = it->second;
        if (animController != nullptr) {
            animController->m_pAnimationTracks[track].m_bFinished = false;
            if (posReset)
                animController->m_pAnimationTracks[track].m_fPosition = 0.0f;
        }
    }
}

void DragonStateMachine::update(float Elapsed_time)
{
    OnPrepareUpdate(6.0f, Elapsed_time);

    /*if (stateElapsedTime >= stateChangeTime) {
        switch (Get_State()) {
        case State::Idle:
            changeState(State::Run, Key_Value::None);
            break;
        case State::Run:
            changeState(State::Idle, Key_Value::None);
            break;
        case State::Dive:
            break;
        }

        stateElapsedTime = 0.0f;
        stateChangeTime = 1.0f + static_cast<float>(rand() % 10);
    }*/

    switch (Get_State()) {
    case State::Idle:
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
        // m_pOwner->targetWeights[TRACK_IDLE] = 1.0f;
        m_pOwner->targetWeights[0] = 1.0f;
        break;
    case State::Run:
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
        m_pOwner->targetWeights[3] = 1.0f;

        RootMotionMove(10.0f);

        break;
    case State::Attack2:
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 150.0f);
        m_pOwner->targetWeights[10] = 1.0f;
        RootMotionMove(0.0f);
        break;
    }

    SetWeight();
}

void DragonStateMachine::enterState(State state, Key_Value key_event)
{
    if (GetDragonRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = false;
        }

        ResetTrackForState(state, true);
    }
    switch (state)
    {
    case State::Attack1:
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = 0.0f;
            m_pOwner->targetWeights[i] = 0.0f;
            animController->m_pAnimationTracks[i].m_fPosition = 0.0f;
        }
        break;
    default:
        break;
    }
}

void DragonStateMachine::exitState(State state, Key_Value key_event)
{
    if (GetDragonRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }

        ResetTrackForState(state, false);
    }
}

void DragonStateMachine::ResetTrackForState(State state, bool posReset)
{
    auto it = GetDragonRootMotionStateToTrackMap().find(state);
    if (it != GetDragonRootMotionStateToTrackMap().end()) {
        int track = it->second;
        if (animController != nullptr) {
            animController->m_pAnimationTracks[track].m_bFinished = false;
            if (posReset)
                animController->m_pAnimationTracks[track].m_fPosition = 0.0f;
        }
    }
}