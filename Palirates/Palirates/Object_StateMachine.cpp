#include "stdafx.h"
#include "Object_StateMachine.h"
#include "Player.h"


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
    if (key_state.forward) AddToMoveZ(Elapsed_time);
    // 현재 상태에 따른 동작 or 애니메이션을 수행
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
    if (!key_state.forward && !key_state.back && !key_state.left && !key_state.right) {
        DebugOutput("Change Idle\n");
        changeState(State::Idle, Key_Value::None);
    }

    if (key_state.dive)
    {
        DebugOutput("Idle->Dive\n");
        changeState(State::Dive, Key_Value::Dive_Key_Down);
    }

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

    m_pOwner->SetStateElapsedTime(0.0f);

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
