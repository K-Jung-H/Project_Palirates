#pragma once
#include <string>

enum class Monster_Type : int
{
    jJol,
    midBoss,
    Boss
};


struct Monster
{
    int id;
    float x, y, z;
    float lookX, lookY, lookZ;
    int hp;
    int state; // 0: idle, 1: 이동, 2: 공격

    Monster() : id(-1), x(0), y(0), z(0), hp(100), state(0) {}

    Monster(int monsterId, float startX, float startY, float startZ, int health)
        : id(monsterId), x(startX), y(startY), z(startZ), hp(health), state(0) {}

    void update(float newX, float newY, float newZ, int newState, int newHp)
    {
        x = newX;
        y = newY;
        z = newZ;
        state = newState;
        hp = newHp;
    }

    std::string Serialize()
    {
        return "MONSTER_DATA," + std::to_string(id) + "," +
            std::to_string(x) + "," + std::to_string(y) + "," +
            std::to_string(z) + "," + std::to_string(hp) + "," +
            std::to_string(state);
    }
};
