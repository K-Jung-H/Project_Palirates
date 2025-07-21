#include "stdafx.h"
#include "Object_StateMachine.h"
#include "GameObject.h"
#include "State.h"

void MonsterStateMachine::update(float deltaTime)
{
    if (currentState) {
        currentState->Update(m_pOwner, deltaTime, this);
    }
}

void MonsterStateMachine::ChangeState(std::unique_ptr<MonsterState> newState)
{
    if (!newState) return;

    if (currentState)
        currentState->Exit(m_pOwner);

    currentState = std::move(newState);
    currentStateEnum = currentState->GetStateEnum();

    if (currentState)
        currentState->Enter(m_pOwner, this);
}

void MonsterStateMachine::SetWeight(float deltaTime)
{
    if (!m_pOwner || !animController) return;

    int n_Ani = m_pOwner->n_Animation;

    for (int i = 0; i < n_Ani; i++)
    {
        float prev = m_pOwner->prevWeights[i];
        float target = m_pOwner->targetWeights[i];

        if (fabs(prev - target) < ANIMATION_CALLBACK_EPSILON)
            continue;

        float newWeight = prev + (target - prev) * 5.0f * deltaTime;

        m_pOwner->prevWeights[i] = newWeight;
        animController->SetTrackWeight(i, newWeight);
       
    }

    /*if (m_pOwner->GetType() == Monster_Type::Fishman) {
        std::cout << "set weight - ";
        for (int i = 0; i < n_Ani; i++) {
            std::cout << animController->m_pAnimationTracks[i].m_fWeight << ", ";
        }
        std::cout << std::endl;
    }*/
}

void MonsterStateMachine::OnPrepareUpdate(float deltaTime)
{
 
    for (int i = 0; i < n_Ani; i++) {
        m_pOwner->prevWeights[i] = animController->m_pAnimationTracks[i].m_fWeight;
    }
    /*std::cout << "OnPrepareUpdate - ";
    for (int i = 0; i < n_Ani; i++) {
        std::cout << animController->m_pAnimationTracks[i].m_fWeight << ", ";
    }
    std::cout << std::endl;*/
  //  std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);
}
//
//void FishManStateMachine::update(float Elapsed_time)
//{
//    
//    static float switchInterval = 10.0f;
//    static float accumTime = 0.0f;
//    static int   currentTrack = int(m_pOwner->GetID()) % 2;
//
//    animController = m_pOwner->GetSkinnedAnimationController();
//    int n_Ani = m_pOwner->n_Animation;
//
//    for (int i = 0; i < n_Ani; i++) {
//        m_pOwner->prevWeights[i] = animController->m_pAnimationTracks[i].m_fWeight;
//    }
//
//    std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);
//
//    for (int i = 0; i < n_Ani; i++) {
//        m_pOwner->prevWeights[i] = 0.0f;
//        animController->SetTrackWeight(i, 0.0f);
//    }
//
//    animController->SetTrackWeight(currentTrack, 1.0f);
//
//    if (int(m_pOwner->GetID()) % 2 == TRACK_FISHMAN_IDLE) {
//        m_pOwner->targetWeights[TRACK_FISHMAN_IDLE] = 1.0f;
//    } else if (int(m_pOwner->GetID()) % 2 == TRACK_FISHMAN_IDLE_BREAK) {
//        m_pOwner->targetWeights[TRACK_FISHMAN_IDLE_BREAK] = 1.0f;
//    }
//
//    for (int i = 0; i < n_Ani; i++) {
//        animController->SetTrackWeight(i, 0.0f);
//    }
//
//    if (int(m_pOwner->GetID()) % 2 == TRACK_FISHMAN_IDLE) {
//        animController->SetTrackWeight(TRACK_FISHMAN_IDLE, 1.0f);
//    } else if (int(m_pOwner->GetID()) % 2 == TRACK_FISHMAN_IDLE_BREAK) {
//        animController->SetTrackWeight(TRACK_FISHMAN_IDLE_BREAK, 1.0f);
//    }
//
//    static const float centerX = 1500.0f;
//    static const float centerZ = 700.0f;
//    static const float halfRange = 0.0f;
//    static int   dir = +1;
//    static const float moveSpeed = 150.0f;
//    static const float spinSpeedDeg = 180.0f;
//
//    XMFLOAT3 pos = m_pOwner->GetPosition();
//
//    static bool first = true;
//    if (first) {
//        pos.x = centerX;
//        pos.z = centerZ;
//        first = false;
//    }
//
//    float leftX = centerX - halfRange;
//    float rightX = centerX + halfRange;
//
//    if (pos.x >= rightX) dir = -1;
//    if (pos.x <= leftX)  dir = +1;
//
//    pos.x += dir * moveSpeed * Elapsed_time;
//    pos.z = centerZ;
//
//    int offset = GET_MONSTER_INDEX(m_pOwner->GetID());
//    m_pOwner->SetPosition(XMFLOAT3(1500.0f + 10 * offset, 0, 700));
//
//    float deltaYaw = dir * XMConvertToRadians(spinSpeedDeg) * Elapsed_time;
//    m_pOwner->Rotate(0.0f, deltaYaw, 0.0f);
//    
//}
