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

void MonsterManager::AddFishmanMonster(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp, int state, Monster_Type type)
{
    Monster m;
    m.id = 10000 + id;
    m.x = x;
	m.y = y;
    m.z = z;
    m.state = 0; // Idle
    m.stateElapsedTime = 0.0f;
    m.stateChangeInterval = 1.0f + (id % 5);
    m.lookX = 0.0f;
	m.lookY = 1.0f; 
    m.lookZ = 0.0f; 
    m.trackPositions.resize(8, 1.0f);
    m.trackWeights.resize(8, 1.0f);
	monsters[id] = m; // Assign a unique ID based on the monster's ID
}

void MonsterManager::RemoveFishmanMonster(int id)
{
    monsters.erase(id);
}

void MonsterManager::GetFishmanMonsters(std::vector<Monster>& outMonsters) const
{
    outMonsters.clear();
    for (const auto& [id, m] : monsters)
    {
        outMonsters.push_back(m);
    }
}

void MonsterManager::AddAnubisMonster(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp, int state, Monster_Type type)
{
    Monster m;
    m.id = 11000 + id;
    m.x = x;
    m.y = y;
    m.z = z;
    m.state = 0; // Idle
    m.stateElapsedTime = 0.0f;
    m.stateChangeInterval = 1.0f + (id % 5);
    m.lookX = 0.0f;
    m.lookY = 1.0f; 
    m.lookZ = 0.0f; 
    m.trackPositions.resize(9, 1.0f);
    m.trackWeights.resize(9, 1.0f);
    monsters[id] = m; // Fix: Assign the Monster object directly instead of its ID
}

void MonsterManager::RemoveAnubisMonster(int id)
{
    monsters.erase(id);
}

void MonsterManager::GetAnubisMonsters(std::vector<Monster>& outMonsters) const
{
    outMonsters.clear();
    for (const auto& [id, m] : monsters)
    {
        outMonsters.push_back(m);
    }
}

void MonsterManager::AddDragonMonster(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp, int state, Monster_Type type)
{
    Monster m;
    m.id = 12000 + id;
    m.x = x;
    m.y = y;
    m.z = z;
    m.state = 0; // Idle
    m.stateElapsedTime = 0.0f;
    m.stateChangeInterval = 1.0f + (id % 5);
    m.lookX = 0.0f;
    m.lookY = 1.0f; 
    m.lookZ = 0.0f; 
    m.trackPositions.resize(12, 1.0f);
    m.trackWeights.resize(12, 1.0f);
    monsters[id] = m; // Fix: Assign the Monster object directly instead of its ID
}

void MonsterManager::RemoveDragonMonster(int id)
{
    monsters.erase(id);
}

void MonsterManager::GetDragonMonsters(std::vector<Monster>& outMonsters) const
{
    outMonsters.clear();
    for (const auto& [id, m] : monsters)
    {
        outMonsters.push_back(m);
    }
}