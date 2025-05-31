#pragma once
#include "Monster.h"
#include <unordered_map>
#include <vector>

class MonsterManager
{
private:
    std::unordered_map<int, Monster> monsters;

public:
    //MonsterManager();
    //void SpawnMonster(int id, float x, float y, float z, int hp);
    //void UpdateMonster(int id, float x, float y, float z, int hp, int state);
    //std::vector<std::string> GetAllMonsterData();
    void UpdateAI(float deltaTime);
    void RemoveFishmanMonster(int id);
    void GetFishmanMonsters(std::vector<Monster>& outMonsters) const;
    void AddAnubisMonster(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp, int state, Monster_Type type);
    void RemoveAnubisMonster(int id);
    void GetAnubisMonsters(std::vector<Monster>& outMonsters) const;
    void AddDragonMonster(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp, int state, Monster_Type type);
    void RemoveDragonMonster(int id);
    void GetDragonMonsters(std::vector<Monster>& outMonsters) const;
    //void AddMonster(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp, int state, Monster_Type type);
    void AddFishmanMonster(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp, int state, Monster_Type type);
};
