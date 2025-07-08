#include "stdafx.h"
#include "Object_StateMachine.h"
#include "GameObject.h"

void MonsterStateMachine::update(float Elapsed_time)
{

}

void FishManStateMachine::update(float Elapsed_time)
{
    //OnPrepareUpdate(6.0f, Elapsed_time);

    //if (!GetTargetSet()) {
    //    if (stateElapsedTime >= stateChangeTime) {
    //        switch (Get_State()) {
    //        case State::Idle:
    //            //changeState(State::Run, Key_Value::None);
    //            break;
    //        case State::Run:
    //            changeState(State::Idle, Key_Value::None);
    //            break;
    //        }

    //        stateElapsedTime = 0.0f;
    //        stateChangeTime = 1.0f + static_cast<float>(rand() % 10);
    //    }
    //}

    //switch (Get_State()) {
    //case State::Idle:
    //    RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
    //    if (TargetSet)  changeState(State::Run, Key_Value::None);
    //    m_pOwner->targetWeights[TRACK_FISHMAN_IDLE] = 1.0f;
    //    break;
    //case State::Run:
    //    RotateLookToTarget(m_TargetPosition, Elapsed_time, 3.0f, 70.0f);
    //    m_pOwner->targetWeights[TRACK_FISHMAN_WALK] = 1.0f;
    //    RootMotionMove(10.0f);
    //    {
    //        std::uniform_int_distribution<int> dist(0, 1);
    //        State attackState = (dist(m_Rng) == 0) ? State::Attack1 : State::Attack2;
    //        ChangeIfNear(attackState, 20.0f);
    //    }
    //    break;
    //case State::Get_Hit:
    //    if (animController->m_pAnimationTracks[TRACK_FISHMAN_GET_HIT].m_bFinished) {
    //        changeState(State::Idle, Key_Value::None);
    //    }
    //    m_pOwner->targetWeights[TRACK_FISHMAN_GET_HIT] = 1.0f;
    //    RootMotionMove(0.0f, true);
    //    break;
    //case State::Attack1:
    //    if (animController->m_pAnimationTracks[TRACK_FISHMAN_ATTACK1].m_bFinished) {
    //        changeState(State::Idle, Key_Value::None);
    //    }
    //    m_pOwner->targetWeights[TRACK_FISHMAN_ATTACK1] = 1.0f;
    //    RootMotionMove(20.0f);
    //    break;
    //case State::Attack2:
    //    if (animController->m_pAnimationTracks[TRACK_FISHMAN_ATTACK2].m_bFinished) {
    //        changeState(State::Idle, Key_Value::None);
    //    }
    //    m_pOwner->targetWeights[TRACK_FISHMAN_ATTACK2] = 1.0f;
    //    RootMotionMove(20.0f);
    //    break;
    //case State::Knock_Down:
    //    m_pOwner->targetWeights[TRACK_FISHMAN_DEAD] = 1.0f;
    //    RootMotionMove(10.0f);
    //    break;
    //}

    //SetWeight();

    static float switchInterval = 2.0f;   // 몇 초마다 바꿀지
    static float accumTime = 0.0f;   // 누적 시간
    static int   currentTrack = 0;      // 0 또는 1

    accumTime += Elapsed_time;                 // elapsed == 프레임당 경과 시간

    if (accumTime >= switchInterval)
    {
        accumTime = 0.0f;
        currentTrack = (currentTrack == 0) ? 1 : 0;  // 토글
    }

    animController = m_pOwner->GetSkinnedAnimationController();
    int n_Ani = m_pOwner->n_Animation;
    for (int i = 0; i < n_Ani; i++) {
        m_pOwner->prevWeights[i] = 0.0f;
        animController->SetTrackWeight(i, 0.0f);
    }
    animController->SetTrackWeight(currentTrack, 1.0f);

    {
        static const float centerX = 1500.0f; // ← 1500 부근
        static const float centerZ = 700.0f; // ← Z 고정
        static const float halfRange = 0.0f;  // ±300 오가도록
        static       int   dir = +1;      // +1→오른쪽, -1→왼쪽
        static const float moveSpeed = 150.0f;  // 유닛/초
        static const float spinSpeedDeg = 180.0f;  // °/초

        /* --- 이동 ------------------------------- */
        XMFLOAT3 pos = m_pOwner->GetPosition();

        // 최초 한 번 위치를 중심 Z에 맞춰주고 싶다면:
        static bool first = true;
        if (first) { pos.x = centerX; pos.z = centerZ; first = false; }

        float leftX = centerX - halfRange;
        float rightX = centerX + halfRange;

        if (pos.x >= rightX) dir = -1;
        if (pos.x <= leftX)  dir = +1;

        pos.x += dir * moveSpeed * Elapsed_time;
        pos.z = centerZ;                       // Z 고정
        //m_pOwner->SetPosition(pos);
        m_pOwner->SetPosition(XMFLOAT3(1500, 0, 700));

        /* --- 자전 ------------------------------- */
        float deltaYaw = dir * XMConvertToRadians(spinSpeedDeg) * Elapsed_time;
        m_pOwner->Rotate(0.0f, deltaYaw, 0.0f);

        /* --- 디버그 출력 ------------------------ */
        auto look = m_pOwner->GetLook();
        std::cout << pos.x << ", " << pos.y << ", " << pos.z
            << "   |   " << look.x << ", " << look.y << ", " << look.z
            << std::endl;
    }
}