#include "stdafx.h"
#include "Object_StateMachine.h"
#include "Player.h"

std::map<State, std::wstring> stateToStringMap = {
    {State::Idle, L"Idle"},
    {State::Run, L"Run"},
    {State::Knock_Down, L"Knock Down"},
    {State::Get_Up, L"Get Up"},
    {State::Dive, L"Dive"},
    {State::Jump, L"Jump"},
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

    case Key_Value::None:
    case Key_Value::ETC:
    default:
        break;
    }
}

bool Key_State::check_move()
{
    // 서로 반대되는 키를 누른 경우는 움직임 취급 x
    return (left != right) || (forward != back);
}

//========================================================
// 상태 머신

void StateMachine::start()
{
    enterState(currentState, Key_Value::None);
}

void StateMachine::update(float Elapsed_time)
{
    if (!m_pOwner || !m_pOwner->GetSkinnedAnimationController()) return;

    auto* animController = m_pOwner->GetSkinnedAnimationController();
    int n_Ani = m_pOwner->n_Animation;

   // float blendSpeed = 8.0f * Elapsed_time; // 보간 속도 (Elapsed_time을 곱해 시간 기반 변환)
    float blendSpeed = (currentState == State::Dive && nextState == State::Idle) ? 4.0f * Elapsed_time : 8.0f * Elapsed_time;


    // 최초 실행 여부를 체크하는 플래그
    static bool isFirstUpdate = true;

    if (isFirstUpdate) {
        // 최초 실행 시에는 모든 애니메이션의 가중치를 0으로 초기화하고 idle만 1로 설정
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = 0.0f;
            animController->SetTrackWeight(i, 0.0f);
        }
        m_pOwner->prevWeights[TRACK_IDLE] = 1.0f;
        animController->SetTrackWeight(TRACK_IDLE, 1.0f);
    }
    else {
        // 이후부터는 기존 트랙 가중치 저장
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = animController->m_pAnimationTracks[i].m_fWeight;
        }
    }

    std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);
    // 목표 가중치 초기화

    float moveX = m_pOwner->GetMoveX();
    float moveZ = m_pOwner->GetMoveZ();

    if (key_state.dive && Get_State() != State::Dive) {
        nextState = State::Idle;
        m_pOwner->SetStateElapsedTime(0.0f);
        animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
        changeState(State::Dive, Key_Value::None);
    }

    switch (Get_State()) {
    case State::Idle:
        if (moveX == 0.0f && moveZ == 0.0f) {
            m_pOwner->targetWeights[TRACK_IDLE] = 1.0f;
        }
        else {
            if (Get_State() != State::Run) {
                changeState(State::Run, Key_Value::None);
            }
        }
        break;

    case State::Run:
        if (moveX == 0.0f && moveZ == 0.0f) {
            changeState(State::Idle, Key_Value::None);
        }
        else {
            // 방향 벡터 정규화
            float length = sqrtf(moveX * moveX + moveZ * moveZ);
            float normX = moveX / length;
            float normZ = moveZ / length;

            // 가장 가까운 방향 2개 찾기
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

            // 두 개의 방향을 보간하여 목표 가중치 설정
            float totalDot = bestDot + secondDot;
            float weight1 = bestDot / totalDot;
            float weight2 = secondDot / totalDot;

            m_pOwner->targetWeights[directions[bestIndex].track] = weight1;
            m_pOwner->targetWeights[directions[secondIndex].track] = weight2;
        }
        break;
    case State::Dive:
        m_pOwner->targetWeights[TRACK_DIVEROLL_FORWARD] = 1.0f;
        
    	float fFixedSpeed = 300.0f; 

    	// Dive 상태일 때 무조건 전방 이동
        m_pOwner->Move(DIR_FORWARD, fFixedSpeed * Elapsed_time, false);

        if (animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished) {
            key_state.dive = false;
            animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished = false;
            animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
            m_pOwner->targetWeights[TRACK_IDLE] = 1.0f;
            m_pOwner->targetWeights[TRACK_DIVEROLL_FORWARD] = 0.0f;  
            changeState(State::Idle, Key_Value::None);
        }
       
        break;
    }
    
    // 가중치 부드럽게 변환 
    for (int i = 0; i < n_Ani; i++)
    {
        float newWeight = m_pOwner->prevWeights[i] + (m_pOwner->targetWeights[i] - m_pOwner->prevWeights[i]) * blendSpeed;
        animController->SetTrackWeight(i, newWeight);
    }

    isFirstUpdate = false;

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
    { 0x57,     { Key_Value::Forward_Key_Down, Key_Value::Forward_Key_Up } },  // W 키

    { VK_DOWN,  { Key_Value::Back_Key_Down, Key_Value::Back_Key_Up } },
    { 0x53,     { Key_Value::Back_Key_Down, Key_Value::Back_Key_Up } },  // S 키

    { VK_LEFT,  { Key_Value::Left_Key_Down, Key_Value::Left_Key_Up } },
    { 0x41,     { Key_Value::Left_Key_Down, Key_Value::Left_Key_Up } },  // A 키

    { VK_RIGHT, { Key_Value::Right_Key_Down, Key_Value::Right_Key_Up } },
    { 0x44,     { Key_Value::Right_Key_Down, Key_Value::Right_Key_Up } },  // D 키

    { VK_SPACE, { Key_Value::Jump_Key_Down, Key_Value::Jump_Key_Up } },
    { VK_SHIFT, { Key_Value::Dive_Key_Down, Key_Value::Dive_Key_Up } }
    };

    for (const auto& [key, keyPair] : keyMappings)
    {
        key_state.update(pKeysBuffer[key] & 0xF0 ? keyPair.first : keyPair.second);
    }

    // 이따구로 하면 안될 듯? x z 변수로 컨트롤 해야됨
    /*if (!key_state.forward && !key_state.back && !key_state.left && !key_state.right) {
        DebugOutput("Change Idle\n");
        changeState(State::Idle, Key_Value::None);
    }

    if (key_state.dive)
    {
        DebugOutput("Idle->Dive\n");
        changeState(State::Dive, Key_Value::Dive_Key_Down);
    }*/

    switch (currentState)
    {
    // 이딴식으로 하면 안됨
    case State::Idle:
        /*if (key_state.forward)
        {
            DebugOutput("Idle->Run_forward\n");
            changeState(State::Run_Forawrd, Key_Value::Forward_Key_Down);
        }
        else if (key_state.back)
        {
            DebugOutput("Idle->Run_backward\n");
            changeState(State::Run_Backawrd, Key_Value::Back_Key_Down);
        }
        else if (key_state.left)
        {
            DebugOutput("Idle->Run_left\n");
            changeState(State::Run_Left, Key_Value::Left_Key_Down);
        }
        else if (key_state.right)
        {
            DebugOutput("Idle->Run_right\n");
            changeState(State::Run_Right, Key_Value::Right_Key_Down);
        }
        else *//*if (key_state.dive)
        {
            DebugOutput("Idle->Dive\n");
            changeState(State::Dive, Key_Value::Dive_Key_Down);
        }*/
        break;

    case State::Run:
       /* if (!key_state.forward)
        {
            DebugOutput("Run_forward->Idle\n");
            changeState(State::Idle, Key_Value::Forward_Key_Up);
        }*/
        break;
   /* case State::Run_Forawrd:
        if (!key_state.forward)
        {
            DebugOutput("Run_forward->Idle\n");
            changeState(State::Idle, Key_Value::Forward_Key_Up);
        }
        break;

    case State::Run_Backawrd:
        if (!key_state.back)
        {
            DebugOutput("Run_backward->Idle\n");
            changeState(State::Idle, Key_Value::Back_Key_Up);
        }
        break;

    case State::Run_Left:
        if (!key_state.left)
        {
            DebugOutput("Run_left->Idle\n");
            changeState(State::Idle, Key_Value::Left_Key_Up);
        }
        break;

    case State::Run_Right:
        if (!key_state.right)
        {
            DebugOutput("Run_right->Idle\n");
            changeState(State::Idle, Key_Value::Right_Key_Up);
        }
        break;*/
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
    if (!m_pOwner || !m_pOwner->GetSkinnedAnimationController()) return;

    auto* animController = m_pOwner->GetSkinnedAnimationController();

    int n_Ani = m_pOwner->n_Animation;

    /*for (int i = 0; i < n_Ani; ++i) {
         animController->SetTrackEnable(i, false);
     }*/
     /*animController->SetTrackEnable(GetStateKey(Get_State()), true);
     animController->SetTrackEnable(GetStateKey(Get_LastState()), true);*/

    switch (state)
    {
    case State::Idle:
       /* for (int i = 0; i < n_Ani; ++i) {
            animController->SetTrackEnable(i, false);
        }

        animController->SetTrackEnable(TRACK_IDLE, true);
        if (lastState == State::Run_Forawrd)
        {
            animController->SetTrackEnable(TRACK_RUN_FORWARD, true);
        }
        else if (lastState == State::Run_Backawrd) 
        {
            animController->SetTrackEnable(TRACK_RUN_BACKWARD, true);
        }
        else if (lastState == State::Run_Left)
        {
            animController->SetTrackEnable(TRACK_RUN_LEFT, true);
        }
        else if (lastState == State::Run_Right)
        {
            animController->SetTrackEnable(TRACK_RUN_RIGHT, true);
        }
        else if (lastState == State::Dive)
        {
            animController->SetTrackEnable(TRACK_DIVEROLL_FORWARD, true);
        }*/
        
        break;
    case State::Run:
        break;
    case State::Dive:
        /*animController->SetTrackEnable(TRACK_IDLE, true);
        for (int i = 1; i < n_Ani; ++i) {
            animController->SetTrackEnable(i, false);
        }
        animController->SetTrackEnable(TRACK_DIVEROLL_FORWARD, true);*/
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
    if (!m_pOwner || !m_pOwner->GetSkinnedAnimationController()) return;

    auto* animController = m_pOwner->GetSkinnedAnimationController();

    int n_Ani = m_pOwner->n_Animation;

    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Dive:
        key_state.dive = false;
        animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished = false;
        animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_fPosition = 0.0f;
        DebugOutput("Dive->Idle\n");
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;
        
    default:
        break;
    }
}
