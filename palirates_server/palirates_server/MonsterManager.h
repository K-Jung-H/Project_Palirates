#pragma once
#include <unordered_map>
#include <mutex>
#include <iostream>

struct Monster
{
    int id;
    float x, y, z;
    int state;
    int hp;
    int attackPower;

    Monster(int monsterId, float startX, float startY, float startZ, int startHp, int attack)
        : id(monsterId), x(startX), y(startY), z(startZ), state(0), hp(startHp), attackPower(attack) {}

    void update(float newX, float newY, float newZ, int newState)
    {
        x = newX;
        y = newY;
        z = newZ;
        state = newState;
    }

    void takeDamage(int damage)
    {
        hp -= damage;
        if (hp < 0) hp = 0;
    }
};

class MonsterManager
{
private:
    std::unordered_map<int, Monster> monsters;
    std::mutex monsterMutex;

public:
    void AddMonster(int id, float x, float y, float z, int hp, int attack);
    void UpdateMonster(int id, float x, float y, float z, int state);
    void DamageMonster(int id, int damage);
    void InitializeMonsters(MonsterManager& manager);
    Monster GetMonster(int id);
    std::unordered_map<int, Monster> GetAllMonsters();
    void UpdateMonsterAI();
};
