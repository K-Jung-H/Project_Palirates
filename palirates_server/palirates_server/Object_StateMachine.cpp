#include "stdafx.h"
#include "Object_StateMachine.h"
#include "GameObject.h"

void MonsterStateMachine::update(float Elapsed_time)
{

}

void MonsterStateMachine::SetWeight(float Elapsed_time)
{
    //animController = m_pOwner->GetSkinnedAnimationController();
    int n_Ani = m_pOwner->n_Animation;

    for (int i = 0; i < n_Ani; i++)
    {
        float prev = m_pOwner->prevWeights[i];
        float target = m_pOwner->targetWeights[i];

        if (fabs(prev - target) == 0.0f)
            continue;

        float newWeight = prev + (target - prev) * 5.0f * Elapsed_time;
        animController->SetTrackWeight(i, newWeight);
    }
}

void FishManStateMachine::update(float Elapsed_time)
{

    static float switchInterval = 10.0f;   // 몇 초마다 바꿀지
    static float accumTime = 0.0f;   // 누적 시간
    static int   currentTrack = int(m_pOwner->GetID()) % 2; ;    // 0 또는 1

    //accumTime += Elapsed_time;                 // elapsed == 프레임당 경과 시간

    //if (accumTime >= switchInterval)
    //{
    //    accumTime = 0.0f;
    //    currentTrack = (currentTrack == 0) ? 1 : 0;  // 토글
    //}

    animController = m_pOwner->GetSkinnedAnimationController();
    int n_Ani = m_pOwner->n_Animation;

    for (int i = 0; i < n_Ani; i++) {
        m_pOwner->prevWeights[i] = animController->m_pAnimationTracks[i].m_fWeight;
    }
    std::fill(m_pOwner->targetWeights.begin(), m_pOwner->targetWeights.end(), 0.0f);
    for (int i = 0; i < n_Ani; i++) {
        m_pOwner->prevWeights[i] = 0.0f;
        animController->SetTrackWeight(i, 0.0f);
    }
    animController->SetTrackWeight(currentTrack, 1.0f);
    if (int(m_pOwner->GetID()) % 2 == TRACK_FISHMAN_IDLE) {
        m_pOwner->targetWeights[TRACK_FISHMAN_IDLE] = 1.0f;
    }
    else if (int(m_pOwner->GetID()) % 2 == TRACK_FISHMAN_IDLE_BREAK) {
        m_pOwner->targetWeights[TRACK_FISHMAN_IDLE_BREAK] = 1.0f;
    }

    for (int i = 0; i< n_Ani; i++)
    {
        animController->SetTrackWeight(i, 0.0f);
	}

    if (int(m_pOwner->GetID()) % 2 == TRACK_FISHMAN_IDLE) {
        animController->SetTrackWeight(TRACK_FISHMAN_IDLE, 1.0f);
    }
    else if (int(m_pOwner->GetID()) % 2 == TRACK_FISHMAN_IDLE_BREAK) {
        animController->SetTrackWeight(TRACK_FISHMAN_IDLE_BREAK, 1.0f);
    }
	//SetWeight(Elapsed_time);

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
        m_pOwner->SetPosition(XMFLOAT3(1500.0f + 10 * m_pOwner->GetID(), 0, 700));

        /* --- 자전 ------------------------------- */
        float deltaYaw = dir * XMConvertToRadians(spinSpeedDeg) * Elapsed_time;
        m_pOwner->Rotate(0.0f, deltaYaw, 0.0f);

        /* --- 디버그 출력 ------------------------ */
        auto look = m_pOwner->GetLook();
      /*  std::cout << pos.x << ", " << pos.y << ", " << pos.z
            << "   |   " << look.x << ", " << look.y << ", " << look.z
            << std::endl;*/
    }
}