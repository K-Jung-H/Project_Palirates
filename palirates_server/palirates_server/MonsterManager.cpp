#pragma once
#include "Monster.h"
#include <unordered_map>
#include <vector>

class MonsterManager
{
private:
    std::unordered_map<int, Monster> monsters;

public:
    MonsterManager();
    void SpawnMonster(int id, float x, float y, float z, int hp);
    void UpdateMonster(int id, float x, float y, float z, int hp, int state);
    std::vector<std::string> GetAllMonsterData();
};
