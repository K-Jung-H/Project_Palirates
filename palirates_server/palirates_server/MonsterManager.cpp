#pragma once
#include "stdafx.h"
#include "Monster.h"
#include "MonsterManager.h"


void MonsterManager::UpdateAI(float deltaTime)
{
    for (auto& [id, m] : monster_map)
    {
        m.stateElapsedTime += deltaTime;

        if (m.stateElapsedTime >= m.stateChangeInterval)
        {
            if (m.GetState() == Monster_State::Idle) 
                m.SetState(Monster_State::Walk);
            else 
                m.SetState(Monster_State::Idle);

            m.stateElapsedTime = 0.0f;
            m.stateChangeInterval = 1.0f + (id % 5);
        }

        auto& weights = m.GetTrackWeights();
        std::fill(weights.begin(), weights.end(), 0.0f);

        Monster_State monster_state = m.GetState();

        if (monster_state == Monster_State::Idle)
            weights[0] = 1.0f; // Idle
        else if (monster_state == Monster_State::Walk)
            weights[1] = 1.0f; // Walk

        auto& positions = m.GetTrackPositions();
        for (float& pos : positions)
        {
            pos += deltaTime;
            if (pos > 1.0f) pos -= 1.0f;
        }

        if (m.GetState() == Monster_State::Walk)
        {
            XMFLOAT3 current_pos = m.GetPosition();
            XMFLOAT3 current_look = m.GetLook();

            // 이동 거리 계산
            float dx = current_look.x * deltaTime * 5.0f;
            float dz = current_look.z * deltaTime * 5.0f;

            // 새로운 위치
            XMFLOAT3 new_pos = 
            {
                current_pos.x + dx,
                current_pos.y, 
                current_pos.z + dz
            };

            m.SetPosition(new_pos);
        }
    }
}