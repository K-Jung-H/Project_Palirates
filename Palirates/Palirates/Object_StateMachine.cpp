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
    { VK_SHIFT, { Key_Value::Dive_Key_Down, Key_Value::Dive_Key_Up } }
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
    float blendSpeed = 6.0f * Elapsed_time;

    if (isFirstUpdate) {
        animController = m_pOwner->GetSkinnedAnimationController();
        n_Ani = m_pOwner->n_Animation;
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = 0.0f;
            animController->SetTrackWeight(i, 0.0f);
        }
        m_pOwner->prevWeights[TRACK_IDLE] = 1.0f;
        animController->SetTrackWeight(TRACK_IDLE, 1.0f);
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
       // animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
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

            float fFixedSpeed = 300.0f;


            std::wostringstream oss;
            XMFLOAT3 vec = animController->HipsPosition;
            XMFLOAT3 vec2 = animController->m_xmf3PrevHipsPosition;

            XMFLOAT3 shift;
            shift.x = vec.x - vec2.x;
            shift.y = vec.y - vec2.y;
            shift.z = vec.z - vec2.z;

            animController->m_xmf3PrevHipsPosition = animController->HipsPosition;

            float scaleFactor = 30.0f;
            XMFLOAT3 scaleShift = { shift.x * scaleFactor, shift.y, shift.z * scaleFactor };

            oss << L"XMFLOAT3: ("
                << scaleShift.x << L", "
                << scaleShift.y << L", "
                << scaleShift.z << L")\n";
            //OutputDebugStringW(oss.str().c_str());

            XMFLOAT3 moveDirection = m_pOwner->GetLook(); 
            XMFLOAT3 finalMove = {
                moveDirection.x * scaleShift.z,
                moveDirection.y * scaleShift.z,
                moveDirection.z * scaleShift.z
            };

            if (scaleShift.z > 0.001f) {
                m_pOwner->Move(finalMove, false);
            }
        }
        break;
    case State::Knock_Down:
       /* if (animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }*/
        m_pOwner->targetWeights[TRACK_KNOCK_DOWN] = 1.0f;
        break;
    case State::Get_Up:
        if (animController->m_pAnimationTracks[TRACK_GET_UP].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_GET_UP] = 1.0f;
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

void PlayerStateMachine::enterState(State state, Key_Value key_event)
{

    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Dive:
        animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished = false;
        animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
        break;
    case State::Knock_Down:
        animController->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_bFinished = false;
        animController->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
        break;
    case State::Get_Up:
        animController->m_pAnimationTracks[TRACK_GET_UP].m_bFinished = false;
        animController->m_pAnimationTracks[TRACK_GET_UP].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;

    default:
        break;
    }
}

void PlayerStateMachine::exitState(State state, Key_Value key_event)
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

////////////////////////////////////////////////////////////////////////////

MultiPlayerStateMachine::MultiPlayerStateMachine(std::shared_ptr<CTerrainPlayer> owner)
    : StateMachine(State::Idle), m_pOwner(owner) {
}

void MultiPlayerStateMachine::update(float Elapsed_time)
{
    float blendSpeed = 6.0f * Elapsed_time;

    if (isFirstUpdate) {
        animController = m_pOwner->GetSkinnedAnimationController();
        n_Ani = m_pOwner->n_Animation;
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = 0.0f;
            animController->SetTrackWeight(i, 0.0f);
        }
        m_pOwner->prevWeights[TRACK_IDLE] = 1.0f;
        animController->SetTrackWeight(TRACK_IDLE, 1.0f);
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

            float fFixedSpeed = 300.0f;


            std::wostringstream oss;
            XMFLOAT3 vec = animController->HipsPosition;
            XMFLOAT3 vec2 = animController->m_xmf3PrevHipsPosition;

            XMFLOAT3 shift;
            shift.x = vec.x - vec2.x;
            shift.y = vec.y - vec2.y;
            shift.z = vec.z - vec2.z;

            animController->m_xmf3PrevHipsPosition = animController->HipsPosition;

            float scaleFactor = 20.0f;
            XMFLOAT3 scaleShift = { shift.x * scaleFactor, shift.y, shift.z * scaleFactor };

            oss << L"XMFLOAT3: ("
                << scaleShift.x << L", "
                << scaleShift.y << L", "
                << scaleShift.z << L")\n";
            //OutputDebugStringW(oss.str().c_str());

            XMFLOAT3 moveDirection = m_pOwner->GetLook();
            XMFLOAT3 finalMove = {
                moveDirection.x * scaleShift.z,
                moveDirection.y * scaleShift.z,
                moveDirection.z * scaleShift.z
            };

            if (scaleShift.z > 0.001f) {
              //  m_pOwner->Move(finalMove, false);
                //m_pOwner->Move(finalMove);
            }
        }
        break;
    case State::Knock_Down:
         /*if (animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished) {
             changeState(State::Idle, Key_Value::None);
         }*/
        m_pOwner->targetWeights[TRACK_KNOCK_DOWN] = 1.0f;
        break;
    case State::Get_Up:
        if (animController->m_pAnimationTracks[TRACK_GET_UP].m_bFinished) {
            changeState(State::Idle, Key_Value::None);
        }
        m_pOwner->targetWeights[TRACK_GET_UP] = 1.0f;
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
        animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
        break;
    case State::Knock_Down:
        animController->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_bFinished = false;
        animController->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
        break;
    case State::Get_Up:
        animController->m_pAnimationTracks[TRACK_GET_UP].m_bFinished = false;
        animController->m_pAnimationTracks[TRACK_GET_UP].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
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

//#define ADD
void MonsterStateMachine::update(float Elapsed_time)
{
    float blendSpeed = 6.0f * Elapsed_time;

    stateElapsedTime += Elapsed_time;

    if (isFirstUpdate) {
        animController = m_pOwner->GetSkinnedAnimationController();
        n_Ani = m_pOwner->n_Animation;
        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = 0.0f;
            animController->SetTrackWeight(i, 0.0f);
        }
        m_pOwner->prevWeights[TRACK_IDLE] = 1.0f;
        /*  if (m_pOwner->test_num == 1)
              animController->SetTrackWeight(TRACK_IDLE, 1.0f);
          else if (m_pOwner->test_num == 2)
              animController->SetTrackWeight(3, 1.0f);
          else if (m_pOwner->test_num == 3)
              animController->SetTrackWeight(4, 1.0f);*/
              //animController->SetTrackWeight(TRACK_IDLE, 1.0f);
        isFirstUpdate = false;
    }
    else {

        for (int i = 0; i < n_Ani; i++) {
            m_pOwner->prevWeights[i] = animController->m_pAnimationTracks[i].m_fWeight;
        }
    }

    std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);

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
        m_pOwner->targetWeights[3] = 1.0f;

        XMFLOAT3 vec = animController->HipsPosition;
        XMFLOAT3 vec2 = animController->m_xmf3PrevHipsPosition;

        XMFLOAT3 shift;
        shift.x = vec.x - vec2.x;
        shift.y = vec.y - vec2.y;
        shift.z = vec.z - vec2.z;

        animController->m_xmf3PrevHipsPosition = animController->HipsPosition;
        {
            float scaleFactor = 20.0f;

            XMFLOAT3 scaleShift = { shift.x * scaleFactor, shift.y, shift.z * scaleFactor };


            XMFLOAT3 moveDirection = m_pOwner->GetLook(); 
            XMFLOAT3 finalMove = {
                moveDirection.x * scaleShift.z,
                moveDirection.y * scaleShift.z,
                moveDirection.z * scaleShift.z
            };

            if (scaleShift.z > 0.001f) {
                m_pOwner->Move(finalMove);
            }

        }
        break;
    case State::Dive:

        break;
    }
    /* if (m_pOwner->test_num == 1) {
         XMFLOAT3 vec = animController->HipsPosition;
         string message3 = "blendedTransform :" + std::to_string((int)(vec.x)) + ", " + std::to_string((int)(vec.y)) + ", " + std::to_string((int)(vec.z)) + "\n";
         DebugOutput(message3);
     }*/

    //string message2 = "M" + std::to_string(m_pOwner->test_num) + " weight: ";
    for (int i = 0; i < n_Ani; i++)
    {

        float prev = m_pOwner->prevWeights[i];
        float target = m_pOwner->targetWeights[i];

        if (fabs(prev - target) < 0.0001f)
            continue;

        float newWeight = prev + (target - prev) * blendSpeed;
        animController->SetTrackWeight(i, newWeight);

       // message2 += std::to_string(newWeight) + " ";
    }
   // message2 += "\n";
    // DebugOutput(message2);

    string message;
#ifdef ADD
    if (m_pOwner->test_num == 1) {
        message = "M1 add: " + std::to_string((int)m_pOwner) + ",M1 ST add: " + std::to_string(m_pOwner->test_num) + ",M1 AC add " + std::to_string((int)animController) + " "
            + std::to_string((int)(animController->m_pAnimationSets)) + '\n';
    }
    else if (m_pOwner->test_num == 2)
        message = "M2 add: " + std::to_string((int)m_pOwner) + ",M2 ST add: " + std::to_string(m_pOwner->test_num) + ",M2 AC add " + std::to_string((int)animController) + " "
        + std::to_string((int)(animController->m_pAnimationSets)) + '\n';
    else if (m_pOwner->test_num == 3)
        message = "M3 add: " + std::to_string((int)m_pOwner) + ",M3 ST add: " + std::to_string(m_pOwner->test_num) + ",M3 AC add " + std::to_string((int)animController) + " "
        + std::to_string((int)(animController->m_pAnimationSets)) + '\n';
    DebugOutput(message);
#else

#endif

    //doAction(currentState, Elapsed_time);
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