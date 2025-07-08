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

    animController = m_pOwner->GetSkinnedAnimationController();
    int n_Ani = m_pOwner->n_Animation;
    for (int i = 0; i < n_Ani; i++) {
        m_pOwner->prevWeights[i] = 0.0f;
        animController->SetTrackWeight(i, 0.0f);
    }
    animController->SetTrackWeight(0, 1.0f);

    {
        static float centerX = m_pOwner->GetPosition().x; // 시작 위치를 기준점으로
        static float halfRange = 300.0f;                    // 좌우 300 범위
        static int   dir = +1;                        // +1 → 오른쪽, -1 → 왼쪽
        static float moveSpeed = 150.0f;                    // 이동 속도
        static float spinSpeedDeg = 180.0f;                    // 초당 회전 각도(도)

        /* --- 이동 --- */
        XMFLOAT3 pos = m_pOwner->GetPosition();

        float leftX = centerX - halfRange;
        float rightX = centerX + halfRange;

        if (pos.x >= rightX) dir = -1;
        if (pos.x <= leftX)  dir = +1;

        pos.x += dir * moveSpeed * Elapsed_time;
        m_pOwner->SetPosition(pos);

        /* --- 자전 --- */
        // dir에 따라 시계/반시계로 회전
        float deltaYaw = dir * XMConvertToRadians(spinSpeedDeg) * Elapsed_time;
        m_pOwner->Rotate(0.0f, deltaYaw, 0.0f);  // (pitch, yaw, roll)

        /* --- 정면 벡터를 대략 맞춰줌(선택) --- */
       // m_pOwner->SetLook((dir > 0) ? XMFLOAT3{ 1,0,0 } : XMFLOAT3{ -1,0,0 });

        auto tpos = m_pOwner->GetPosition();
        auto look = m_pOwner->GetLook();

        std::cout << tpos.x << ", " << tpos.y << ", " << tpos.x << ", " << look.x << ", " << look.y << ", " << look.z << std::endl;
    }
}