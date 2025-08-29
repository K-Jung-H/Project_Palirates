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
    { -0.707f,  0.707f, TRACK_RUN_FORWARD_LEFT },  
    {  0.000f,  1.000f, TRACK_RUN_FORWARD },       
    {  0.707f,  0.707f, TRACK_RUN_FORWARD_RIGHT },
    { -0.707f, -0.707f, TRACK_RUN_BACKWARD_LEFT }, 
    {  0.000f, -1.000f, TRACK_RUN_BACKWARD },    
    {  0.707f, -0.707f, TRACK_RUN_BACKWARD_RIGHT },
    { -1.000f,  0.000f, TRACK_RUN_LEFT },         
    {  1.000f,  0.000f, TRACK_RUN_RIGHT }         
};

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
       // attack1 = true;
        if (!attack_toggle)
        {
            attack1 = true;
            attack2 = false;
        }
        else
        {
            attack1 = false;
            attack2 = true;
        }
        attack_toggle = !attack_toggle;
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

AnimationTrack GetRunTrackFromInput(uint32_t inputFlags)
{
    bool w = (inputFlags & INPUT_W) != 0;
    bool s = (inputFlags & INPUT_S) != 0;
    bool a = (inputFlags & INPUT_A) != 0;
    bool d = (inputFlags & INPUT_D) != 0;

    if (w && a) return TRACK_RUN_FORWARD_LEFT;
    if (w && d) return TRACK_RUN_FORWARD_RIGHT;
    if (w)      return TRACK_RUN_FORWARD;

    if (s && a) return TRACK_RUN_BACKWARD_LEFT;
    if (s && d) return TRACK_RUN_BACKWARD_RIGHT;
    if (s)      return TRACK_RUN_BACKWARD;

    if (a)      return TRACK_RUN_LEFT;
    if (d)      return TRACK_RUN_RIGHT;

    return TRACK_IDLE;
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

    { VK_LBUTTON, { Key_Value::Attack1_Key_Down, Key_Value::Attack1_Key_Up } },
    { VK_OEM_PERIOD, { Key_Value::Attack2_Key_Down, Key_Value::Attack2_Key_Up } },
    { VK_RBUTTON, { Key_Value::Attack3_Key_Down, Key_Value::Attack3_Key_Up } }
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
        break;

    case State::Attack_Normal:
        break;

    default:
        break;
    }
}

void StateMachine::changeState(State newState, Key_Value key_event)
{
    exitState(currentState, key_event);
    currentState = newState;
    enterState(currentState, key_event);
    lastStateChange = static_cast<int>(newState);
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

float StateMachine::GetDistance(const XMFLOAT3& a, const XMFLOAT3& b)
{
    XMVECTOR vecA = XMLoadFloat3(&a);
    XMVECTOR vecB = XMLoadFloat3(&b);
    return XMVectorGetX(XMVector3Length(vecB - vecA));
}

bool StateMachine::IsInState(std::initializer_list<State> states)
{
    State current = Get_State();
    for (State s : states)
    {
        if (current == s)
            return true;
    }
    return false;
}

PlayerStateMachine::PlayerStateMachine(CPlayer* owner)
    : StateMachine(State::Idle), m_pOwner(owner) {
}

void PlayerStateMachine::update(float Elapsed_time)
{
    blendSpeed = 5.0f * Elapsed_time;

    if (m_pOwner->bIsInvincible) {
        if (currentState != State::Knock_Down && currentState != State::Get_Up) {
            m_pOwner->invincibleTimeRemaining += Elapsed_time;
            if (m_pOwner->invincibleTimeRemaining >= m_pOwner->invincibleDuration) {
                m_pOwner->invincibleTimeRemaining = 0.0f;
                m_pOwner->bIsInvincible = false;
            }
        }
    }
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

    //std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);

    //float moveX = m_pOwner->GetMoveX();
    //float moveZ = m_pOwner->GetMoveZ();

    //if (key_state.dive && Get_State() != State::Dive) {
    //    m_pOwner->SetStateElapsedTime(0.0f);
    //    if (key_state.right)
    //        m_pOwner->SetLookDirection(m_pOwner->GetRight());
    //    if (key_state.left) {
    //        XMVECTOR rightVec = XMLoadFloat3(&m_pOwner->GetRight());
    //        XMVECTOR leftVec = -rightVec;

    //        XMFLOAT3 left;
    //        XMStoreFloat3(&left, leftVec);
    //        m_pOwner->SetLookDirection(left);

    //    }
    //    changeState(State::Dive, Key_Value::None);
    //}
    //if (key_state.attack1 && Get_State() != State::Attack1) {
    //    m_pOwner->SetStateElapsedTime(0.0f);
    //    changeState(State::Attack1, Key_Value::None);
    //}
    //if (key_state.attack2 && Get_State() != State::Attack2) {
    //    m_pOwner->SetStateElapsedTime(0.0f);
    //    changeState(State::Attack2, Key_Value::None);
    //}
    //if (key_state.attack3 && Get_State() != State::Attack3) {
    //    m_pOwner->SetStateElapsedTime(0.0f);
    //    changeState(State::Attack3, Key_Value::None);
    //}
    switch (Get_State()) {
    case State::Run: {
        std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);
        m_pOwner->targetWeights[GetRunTrackFromInput(m_pOwner->current_keyboard_inputFlags)] = 1.0f;
    }
                   break;
    }
    //switch (Get_State()) {
    ///*case State::Idle:
    //    if (moveX == 0.0f && moveZ == 0.0f) {
    //        m_pOwner->targetWeights[TRACK_IDLE] = 1.0f;
    //    }
    //    else {
    //        changeState(State::Run, Key_Value::None);
    //    }
    //    break;*/

    //case State::Run:
    //    //if (moveX == 0.0f && moveZ == 0.0f) {
    //    //    changeState(State::Idle, Key_Value::None);
    //    //}
    //    //else {
    //    //    float length = sqrtf(moveX * moveX + moveZ * moveZ);
    //    //    float normX = moveX / length;
    //    //    float normZ = moveZ / length;

    //    //    int bestIndex = -1, secondIndex = -1;
    //    //    float bestDot = -1.0f, secondDot = -1.0f;

    //    //    for (int i = 0; i < 8; i++) {
    //    //        float dot = normX * directions[i].x + normZ * directions[i].z;
    //    //        if (dot > bestDot) {
    //    //            secondDot = bestDot;
    //    //            secondIndex = bestIndex;
    //    //            bestDot = dot;
    //    //            bestIndex = i;
    //    //        }
    //    //        else if (dot > secondDot) {
    //    //            secondDot = dot;
    //    //            secondIndex = i;
    //    //        }
    //    //    }

    //    //    float totalDot = bestDot + secondDot;
    //    //    float weight1 = bestDot / totalDot;
    //    //    float weight2 = secondDot / totalDot;

    //    //    m_pOwner->targetWeights[directions[bestIndex].track] = weight1;
    //    //    m_pOwner->targetWeights[directions[secondIndex].track] = weight2;
    //    //}
    //   // m_pOwner->targetWeights[TRACK_RUN_FORWARD] = 1.0f;
    //    break;
    //case State::Dive:

    //    if (animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished) {
    //        changeState(State::Idle, Key_Value::None);
    //    }
    //    else {
    //        m_pOwner->targetWeights[TRACK_DIVEROLL_FORWARD] = 1.0f;
    //        RootMotionMove(30.0f);
    //    }
    //    break;
    //case State::Knock_Down:
    //    m_pOwner->targetWeights[TRACK_KNOCK_DOWN] = 1.0f;
    //    RootMotionMove(30.0f, true); {
    //        constexpr float respawntime = 5.0f;
    //        static float time = 0.0;
    //        time += Elapsed_time;
    //        if (time > respawntime) {
    //            time = 0.0f;
    //            changeState(State::Get_Up, Key_Value::None);
    //        }
    //    }
    //    break;
    //case State::Get_Up:
    //    if (animController->m_pAnimationTracks[TRACK_GET_UP].m_bFinished) {
    //        //m_pOwner->currentHP = 50.0f;
    //        changeState(State::Idle, Key_Value::None);
    //    }
    //    m_pOwner->targetWeights[TRACK_GET_UP] = 1.0f;
    //    RootMotionMove(10.0f);
    //    break;
    //case State::Attack1:
    //    if (animController->m_pAnimationTracks[TRACK_ATTACK1].m_bFinished) {
    //        changeState(State::Idle, Key_Value::None);
    //    }
    //    m_pOwner->targetWeights[TRACK_ATTACK1] = 1.0f;
    //    RootMotionMove(30.0f);
    //    break;
    //case State::Attack2:
    //    if (animController->m_pAnimationTracks[TRACK_ATTACK2].m_bFinished) {
    //        changeState(State::Idle, Key_Value::None);
    //    }
    //    m_pOwner->targetWeights[TRACK_ATTACK2] = 1.0f;
    //    RootMotionMove(30.0f);
    //    break;
    //case State::Attack3:
    //    if (animController->m_pAnimationTracks[TRACK_ATTACK3].m_bFinished) {
    //        changeState(State::Idle, Key_Value::None);
    //    }
    //    m_pOwner->targetWeights[TRACK_ATTACK3] = 1.0f;
    //    RootMotionMove(30.0f);
    //    break;
    //case State::Get_Hit_F2:
    //    if (animController->m_pAnimationTracks[TRACK_GET_HIT_F2].m_bFinished) {
    //        changeState(State::Idle, Key_Value::None);
    //    }
    //    m_pOwner->targetWeights[TRACK_GET_HIT_F2] = 1.0f;
    //    RootMotionMove(30.0f, true);
    //    break;
    //case State::Select_Idle:
    //    m_pOwner->targetWeights[TRACK_SELECT_IDLE] = 1.0f;
    //    //RootMotionMove(30.0f, true);
    //    break;
    //}

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

    if (IsInState({ State::Attack1, State::Attack2, State::Attack3 })) {
       /* if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOn();
        m_pOwner->Trail_Start();
        m_pOwner->GetTrailObj()->Set_Active(true);
        m_pOwner->GetTrailObj()->GetTrailMesh()->ResetTrail();
        m_pOwner->bTrailOn();
        std::cout << "Trail On" << "\n";*/

        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOn();
        }
        for (auto& t_obj : m_pOwner->GetTrailObj()) {
            t_obj->Set_Active(true);
            t_obj->GetTrailMesh()->ResetTrail();
        }
        m_pOwner->Trail_Start();
        m_pOwner->bTrailOn();
        std::cout << "Trail On" << "\n";
    }

    if (IsInState({ State::Get_Hit_F2 })) {
        m_pOwner->bIsInvincible = true;
        //m_pOwner->SetOutlineColor(1);
        m_pOwner->Set_Color_Blending(GetColorById(Client_ID));
       // m_pOwner->currentHP -= 30.0f;
        //if (m_pOwner->currentHP < 0) {
        //    //m_pOwner->currentHP = 0.0f;
        //    changeState(State::Knock_Down, Key_Value::None);
        //    //m_pOwner->currentHP = 100.0f;
        //}
        onGetHitEffect(true);
        for (int i = 0; i < n_Ani; i++)
        {
            if (i == TRACK_GET_HIT_F2) {
                animController->SetTrackWeight(i, 1.0f);
            }
            else
                animController->SetTrackWeight(i, 0.0f);
        }
    }

    switch (state)
    {
    case State::Idle:
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }
        break;
    case State::Select_Idle:
        if (m_pOwner != nullptr) {
           /* if (m_pOwner->Weapon_ptr != nullptr)
                m_pOwner->Weapon_ptr->Set_Active(false);*/
            for (auto& w : m_pOwner->Weapon_ptr) {
                w->Set_Active(false);
            }
        }
        break;
    case State::Knock_Down:
        m_pOwner->bIsInvincible = true;
        break;
    case State::Get_Up:
		//m_pOwner->currentHP = 50.0f;
        break;
        
    default:
        break;
    }

    std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);

    m_pOwner->targetWeights[GetPlayerAnimationTrack(state)] = 1.0f;
}

void PlayerStateMachine::exitState(State state, Key_Value key_event)
{

    if (GetRootMotionStates().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }

        ResetTrackForState(state, false);

    }

    if (IsInState({ State::Get_Hit_F2 })) {
        //m_pOwner->SetOutlineColor(0);
    }

    if (IsInState({ State::Attack1, State::Attack2, State::Attack3 })) {
        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOff();
        }
        for (auto& t_obj : m_pOwner->GetTrailObj()) {
            t_obj->Set_Active(false);
        }
        m_pOwner->Trail_End();
        m_pOwner->bTrailOff();
        /*if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOff();
        m_pOwner->Trail_End();
        m_pOwner->GetTrailObj()->Set_Active(false);
        m_pOwner->bTrailOff();*/
        std::cout << "Trail Off" << "\n";
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
        onUpdateUI(true);
        m_pOwner->bIsInvincible = false;
        break;
    case State::Attack1:
        key_state.attack1 = false;
        break;
    case State::Attack2:
        key_state.attack2 = false;
        break;
    case State::Attack3:
        key_state.attack3 = false;
        break;
    case State::Select_Idle: 
        if (m_pOwner != nullptr) {
            //m_pOwner->Weapon_ptr->Set_Active(true);
            for (auto& w : m_pOwner->Weapon_ptr) {
                w->Set_Active(true);
            }
        }
    
        break;

    default:
        break;
    }
}

void PlayerStateMachine::RootMotionMove(float scaleFactor, bool bUseNegative) {

    //XMFLOAT3 vec = animController->HipsPosition;
    //XMFLOAT3 vec2 = animController->m_xmf3PrevHipsPosition;

    //XMFLOAT3 shift;
    //shift.x = vec.x - vec2.x;
    //shift.y = vec.y - vec2.y;
    //shift.z = vec.z - vec2.z;

    //animController->m_xmf3PrevHipsPosition = vec;
    //XMFLOAT3 scaleShift = { shift.x * scaleFactor, shift.y * scaleFactor, shift.z * scaleFactor };

    //XMFLOAT3 moveDirection = m_pOwner->GetLook();
    //XMFLOAT3 finalMove = {
    //    moveDirection.x * scaleShift.z,
    //    moveDirection.y * scaleShift.z,
    //    moveDirection.z * scaleShift.z
    //};

    //if (bUseNegative) {
    //    finalMove.x = -1 * finalMove.x;
    //    finalMove.z = -1 * finalMove.z;
    //}

    //if (scaleShift.z > 0.001f) {
    //    m_pOwner->Move(finalMove, false);
    //}
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

       /* float target = m_pOwner->targetWeights[i];
        if (target == 0.0f) {
            animController->SetTrackWeight(i, 0.0f);
        }
        else  animController->SetTrackWeight(i, 1.0f);*/
    }
}

////////////////////////////////////////////////////////////

MonsterStateMachine::MonsterStateMachine(CMonsterObject* owner)
    : StateMachine(State::Idle), m_pOwner(owner) {

}

void MonsterStateMachine::update(float Elapsed_time)
{
    OnPrepareUpdate(6.0f, Elapsed_time);

    //if (stateElapsedTime >= stateChangeTime) {
    //    switch (Get_State()) {
    //    case State::Idle:
    //        changeState(State::Run, Key_Value::None);
    //        break;
    //    case State::Run:
    //        changeState(State::Idle, Key_Value::None);
    //        break;
    //    case State::Dive:
    //        break;
    //    }

    //    stateElapsedTime = 0.0f;
    //    stateChangeTime = 1.0f + static_cast<float>(rand() % 10);
    //}

    //switch (Get_State()) {
    //case State::Idle:
    //   // m_pOwner->targetWeights[TRACK_IDLE] = 1.0f;
    //    m_pOwner->targetWeights[0] = 1.0f;
    //    break;
    //case State::Run:
    //    m_pOwner->targetWeights[6] = 1.0f;

    //    RootMotionMove(10.0f);

    //    break;
    //case State::Attack2:

    //    break;
    //}

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
     //   m_pOwner->prevWeights[TRACK_IDLE] = 1.0f;
        m_pOwner->prevWeights[0] = 1.0f;
        isFirstUpdate = false;
    }
    else {
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = animController->m_pAnimationTracks[i].m_fWeight;
        }
    }

   // std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);
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
    if (IsInState({ State::Get_Hit })) {
        m_pOwner->SetOutlineColor(1);
       /* m_pOwner->currentHP -= 30.0f;
        if (m_pOwner->currentHP < 0) {
            m_pOwner->currentHP = 0.0f;
            changeState(State::Knock_Down, Key_Value::None);
        }*/
    }


    std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);
    int currTrack = GetMonsterAnimationTrack(m_pOwner->mType, state);
    if (m_pOwner->mType == Monster_Type::Anubis) {
		cout << currTrack << "\n";
    }
    m_pOwner->targetWeights[currTrack] = 1.0f;
    if (animController != nullptr) {
        animController->m_pAnimationTracks[currTrack].m_bFinished = false;
        animController->m_pAnimationTracks[currTrack].m_fPosition = 0.0f;
    }
}

void MonsterStateMachine::exitState(State state, Key_Value key_event)
{
    if (IsInState({ State::Get_Hit })) {
        m_pOwner->SetOutlineColor(0);
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
        m_pOwner->GetSkinnedAnimationController()->SetTrackWeight(i, newWeight);
    }
}

void MonsterStateMachine::RotateLookToTarget(const XMFLOAT3& targetPos, float fDeltaTime, float fLerpSpeed, float fTriggerDistance)
{
    const XMFLOAT3& selfPosF3 = m_pOwner->GetPosition();

    float distance = GetDistance(selfPosF3, targetPos);
    if (distance > fTriggerDistance) {
        if (TargetSet) ChangeTargetSet();
        return;
    }
   
    if (!TargetSet) ChangeTargetSet();
    XMVECTOR selfPos = XMLoadFloat3(&selfPosF3);
    XMVECTOR target = XMLoadFloat3(&targetPos);
    XMVECTOR toTarget = target - selfPos;

    toTarget = XMVectorSetY(toTarget, 0.0f);
    toTarget = XMVector3Normalize(toTarget);

    XMVECTOR currentLook = XMLoadFloat3(&m_pOwner->GetLook());
    XMVECTOR newLook = XMVector3Normalize(XMVectorLerp(currentLook, toTarget, fLerpSpeed * fDeltaTime));

    newLook = XMVectorSetY(newLook, 0.0f);

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
                //changeState(State::Run, Key_Value::None);
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
    case State::Knock_Down:
        m_pOwner->targetWeights[TRACK_FISHMAN_DEAD] = 1.0f;
        RootMotionMove(10.0f);
        break;
    }

    SetWeight();
}

void FishManStateMachine::enterState(State state, Key_Value key_event)
{
    MonsterStateMachine::enterState(state, key_event);

    if (GetFishManRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = false;
        }

        ResetTrackForState(state, true);
    }

    if (IsInState({ State::Attack1, State::Attack2 })) {
        /*if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOn();*/
        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOn();
        }
    }

    switch (state)
    {
    default:
        break;
    }
}

void FishManStateMachine::exitState(State state, Key_Value key_event)
{
    MonsterStateMachine::exitState(state, key_event);

    if (GetFishManRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }

        ResetTrackForState(state, false);
    }

    if (IsInState({ State::Attack1, State::Attack2 })) {
        /*if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOff();*/
        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOff();
        }
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
                //changeState(State::Run, Key_Value::None);
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
            std::uniform_int_distribution<int> dist(0, 1);
            State attackState;

            switch (dist(m_Rng)) {
            case 0: attackState = State::Attack1; break;
            case 1: attackState = State::Attack2; break;
           // case 2: attackState = State::Attack3; break;
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
       /* const float forceEndTime = 5.0f;
        Skill1_ElapsedTime += Elapsed_time;
        if (Skill1_ElapsedTime > forceEndTime) {
            changeState(State::Idle, Key_Value::None);
            Skill1_ElapsedTime = 0.0f;
        }
        else {
            if (animController->m_pAnimationTracks[TRACK_ANUBIS_SKILL].m_fPosition > 1.3f) {
                animController->m_pAnimationTracks[TRACK_ANUBIS_SKILL].m_fPosition = 1.3f;
            }
        }*/
        Skill1_ElapsedTime += Elapsed_time;
        if (animController->m_pAnimationTracks[TRACK_ANUBIS_SKILL].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
            //Skill1_ElapsedTime = 0.0f;
        }
        m_pOwner->targetWeights[TRACK_ANUBIS_SKILL] = 1.0f;
        RootMotionMove(0.0f);
        break;
    }

    SetWeight();
}

void AnubisStateMachine::enterState(State state, Key_Value key_event)
{
    MonsterStateMachine::enterState(state, key_event);

    if (GetAnubisRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = false;
        }

        ResetTrackForState(state, true);
    }
    if (IsInState({ State::Attack1, State::Attack2 })) {
        /*if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOn();*/
        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOn();
        }
    }
    if (IsInState({ State::Attack3 })) {
        Skill1_ElapsedTime = 0.0f;
    }
}

void AnubisStateMachine::exitState(State state, Key_Value key_event)
{
    MonsterStateMachine::exitState(state, key_event);

    if (GetAnubisRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }

        ResetTrackForState(state, false);
    }
    if (IsInState({ State::Attack1, State::Attack2 })) {
        /*if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOff();*/
        for (auto w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOff();
        }
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

    if (!GetTargetSet()) {
        if (Get_State() == State::Run) {
            changeState(State::Idle, Key_Value::None);
        }
       /* if (stateElapsedTime >= stateChangeTime) {
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
        }*/
    }

    switch (Get_State()) {
    case State::Idle:
        if (TargetSet)  changeState(State::Run, Key_Value::None);
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
        m_pOwner->targetWeights[TRACK_DRAGON_IDLE] = 1.0f;
      
        break;
    case State::Run:
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
        m_pOwner->targetWeights[TRACK_DRAGON_RUN] = 1.0f;

        RootMotionMove(10.0f);

        break;
    case State::Attack2:
        if (!m_pOwner->Test_Mode)
            RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 150.0f);
        m_pOwner->targetWeights[TRACK_DRAGON_BREATHE] = 1.0f;
        RootMotionMove(0.0f);
        break;
    case State::Attack3:
        if (!m_pOwner->Test_Mode)
            RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 150.0f);
        m_pOwner->targetWeights[TRACK_DRAGON_FLY_BREATHE] = 1.0f;
        RootMotionMove(0.0f);
        break;
    }

    SetWeight();
}

void DragonStateMachine::enterState(State state, Key_Value key_event)
{
    MonsterStateMachine::enterState(state, key_event);

    if (GetDragonRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = false;
        }

        ResetTrackForState(state, true);
    }
    if (IsInState({ State::Attack1, State::Attack2, State::Attack3 })) {
       /* if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOn();*/
        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOn();
        }
    }
}

void DragonStateMachine::exitState(State state, Key_Value key_event)
{
    MonsterStateMachine::exitState(state, key_event);

    if (GetDragonRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }

        ResetTrackForState(state, false);
    }
    if (IsInState({ State::Attack1, State::Attack2, State::Attack3 })) {
        /*if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOff();*/
        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOff();
        }
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

void GargoyleStateMachine::update(float Elapsed_time)
{
    OnPrepareUpdate(6.0f, Elapsed_time);

    if (!GetTargetSet()) {
        if (stateElapsedTime >= stateChangeTime) {
            switch (Get_State()) {
            case State::Idle:
                //changeState(State::Run, Key_Value::None);
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
        m_pOwner->targetWeights[TRACK_GARGOYLE_IDLE] = 1.0f;
        break;
    case State::Run:
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
        m_pOwner->targetWeights[TRACK_GARGOYLE_WALK] = 1.0f;
        RootMotionMove(10.0f);
        {
            std::uniform_int_distribution<int> dist(0, 1);
            State attackState = (dist(m_Rng) == 0) ? State::Attack1 : State::Attack2;
            ChangeIfNear(attackState, 20.0f);
        }
        break;
    case State::Get_Hit:
        if (animController->m_pAnimationTracks[TRACK_GARGOYLE_GET_HIT].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_GARGOYLE_GET_HIT] = 1.0f;
        RootMotionMove(0.0f, true);
        break;
    case State::Attack1:
        if (animController->m_pAnimationTracks[TRACK_GARGOYLE_ATTACK1].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_GARGOYLE_ATTACK1] = 1.0f;
        RootMotionMove(20.0f);
        break;
    case State::Attack2:
        if (animController->m_pAnimationTracks[TRACK_GARGOYLE_SKILL_1].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_GARGOYLE_SKILL_1] = 1.0f;
        RootMotionMove(20.0f);
        break;
    case State::Knock_Down:
        m_pOwner->targetWeights[TRACK_GARGOYLE_DEAD] = 1.0f;
        RootMotionMove(10.0f);
        break;
    }

    SetWeight();
}

void GargoyleStateMachine::enterState(State state, Key_Value key_event)
{
    MonsterStateMachine::enterState(state, key_event);

    if (GetGargoyleRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = false;
        }

        ResetTrackForState(state, true);
    }
    if (IsInState({ State::Attack1/*, State::Attack2*/ })) {
        /*if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOn();*/
        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOn();
        }
    }

    //const auto& map = GetGargoyleRootMotionStateToTrackMap();
    //auto it = map.find(state);
    //if (it != map.end()) {
    //    int trackIndex = it->second;  
    //    //m_pOwner->targetWeights[trackIndex] = 1.0f;

    //    if (m_pOwner != nullptr) {
    //        m_pOwner->bIsControllable = false;
    //    }
    //    ResetTrackForState(state, true);
    //}

    //if (IsInState({ State::Attack1/*, State::Attack2, State::Attack3*/ })) {
    //    /* if (m_pOwner->Weapon_ptr != nullptr)
    //         m_pOwner->Weapon_ptr->bUpdateOBBOn();*/
    //    for (auto& w : m_pOwner->Weapon_ptr) {
    //        w->bUpdateOBBOn();
    //    }
    //}
}

void GargoyleStateMachine::exitState(State state, Key_Value key_event)
{
    MonsterStateMachine::exitState(state, key_event);

    if (GetGargoyleRootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }

        ResetTrackForState(state, false);
    }
    if (IsInState({ State::Attack1/*, State::Attack2, State::Attack3*/ })) {
        /*if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOff();*/
        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOff();
        }
    }
}

void GargoyleStateMachine::ResetTrackForState(State state, bool posReset)
{
    auto it = GetGargoyleRootMotionStateToTrackMap().find(state);
    if (it != GetGargoyleRootMotionStateToTrackMap().end()) {
        int track = it->second;
        if (animController != nullptr) {
            animController->m_pAnimationTracks[track].m_bFinished = false;
            if (posReset)
                animController->m_pAnimationTracks[track].m_fPosition = 0.0f;
        }
    }
}

////////////////////////////////////////////////////////

void Creature1StateMachine::update(float Elapsed_time)
{
    OnPrepareUpdate(6.0f, Elapsed_time);

    if (!GetTargetSet()) {
        if (stateElapsedTime >= stateChangeTime) {
            switch (Get_State()) {
            case State::Idle:
                //changeState(State::Run, Key_Value::None);
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
        m_pOwner->targetWeights[TRACK_CREATURE1_IDLE] = 1.0f;
        break;
    case State::Run:
        RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
        m_pOwner->targetWeights[TRACK_CREATURE1_WALK] = 1.0f;
        RootMotionMove(10.0f);
        {
            std::uniform_int_distribution<int> dist(0, 1);
            State attackState = (dist(m_Rng) == 0) ? State::Attack1 : State::Attack2;
            ChangeIfNear(attackState, 20.0f);
        }
        break;
    case State::Get_Hit:
        if (animController->m_pAnimationTracks[TRACK_CREATURE1_GET_HIT].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_CREATURE1_GET_HIT] = 1.0f;
        RootMotionMove(0.0f, true);
        break;
    case State::Attack1:
        if (animController->m_pAnimationTracks[TRACK_CREATURE1_ATTACK1].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_CREATURE1_ATTACK1] = 1.0f;
        RootMotionMove(20.0f);
        break;
    case State::Attack2:
        if (animController->m_pAnimationTracks[TRACK_CREATURE1_ATTACK2].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_CREATURE1_ATTACK2] = 1.0f;
        RootMotionMove(20.0f);
        break;
    case State::Knock_Down:
        m_pOwner->targetWeights[TRACK_CREATURE1_DEAD] = 1.0f;
        RootMotionMove(10.0f);
        break;
    }

    SetWeight();
}

void Creature1StateMachine::enterState(State state, Key_Value key_event)
{
    MonsterStateMachine::enterState(state, key_event);

    if (GetCreature1RootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = false;
        }

        ResetTrackForState(state, true);
    }

    if (IsInState({ State::Attack1, State::Attack2 })) {
        /*if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOn();*/
        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOn();
        }
    }

    switch (state)
    {
    default:
        break;
    }
}

void Creature1StateMachine::exitState(State state, Key_Value key_event)
{
    MonsterStateMachine::exitState(state, key_event);

    if (GetCreature1RootMotionStateToTrackMap().contains(state)) {
        if (m_pOwner != nullptr) {
            m_pOwner->bIsControllable = true;
        }

        ResetTrackForState(state, false);
    }

    if (IsInState({ State::Attack1, State::Attack2 })) {
        /*if (m_pOwner->Weapon_ptr != nullptr)
            m_pOwner->Weapon_ptr->bUpdateOBBOff();*/
        for (auto& w : m_pOwner->Weapon_ptr) {
            w->bUpdateOBBOff();
        }
    }
}

void Creature1StateMachine::ResetTrackForState(State state, bool posReset)
{
    auto it = GetCreature1RootMotionStateToTrackMap().find(state);
    if (it != GetCreature1RootMotionStateToTrackMap().end()) {
        int track = it->second;
        if (animController != nullptr) {
            animController->m_pAnimationTracks[track].m_bFinished = false;
            if (posReset)
                animController->m_pAnimationTracks[track].m_fPosition = 0.0f;
        }
    }
}

