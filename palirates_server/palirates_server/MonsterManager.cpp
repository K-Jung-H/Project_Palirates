#pragma once
#include "Monster.h"
#include <unordered_map>
#include <vector>
#include "MonsterManager.h"


    void MonsterManager::UpdateAI(float deltaTime)
    {
        for (auto& [id, m] : monsters)
        {
            m.stateElapsedTime += deltaTime;

            if (m.stateElapsedTime >= m.stateChangeInterval)
            {
                if (m.state == 0) m.state = 1;
                else m.state = 0;

                m.stateElapsedTime = 0.0f;
                m.stateChangeInterval = 1.0f + (id % 5);
            }
        
            std::fill(m.trackWeights.begin(), m.trackWeights.end(), 0.0f);

            if (m.state == 0)      m.trackWeights[0] = 1.0f; // Idle
            else if (m.state == 1) m.trackWeights[1] = 1.0f; // Walk

            for (int i = 0; i < m.trackPositions.size(); ++i)
            {
                m.trackPositions[i] += deltaTime;
                if (m.trackPositions[i] > 1.0f) m.trackPositions[i] -= 1.0f;
            }

            if (m.state == 1)
            {
                m.x += m.lookX * deltaTime * 5.0f;
                m.z += m.lookZ * deltaTime * 5.0f;
            }
        }
    }