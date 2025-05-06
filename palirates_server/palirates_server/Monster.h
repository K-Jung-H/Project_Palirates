#pragma once
#include <string>

enum class Monster_Type : int
{
    jJol,
    midBoss,
    Boss,
    ETC
};


struct Monster
{
    int id;
    float x, y, z;
    float lookX, lookY, lookZ;
    int hp;
    int state; // 0: idle, 1: 이동, 2: 공격
    Monster_Type type;

    Monster() : id(-1), x(0), y(0), z(0), lookX(0), lookY(1), lookZ(0), hp(100), state(0), type(Monster_Type::ETC) {}

    Monster(int monsterId, float startX, float startY, float startZ,
        float lookvecX, float lookvecY, float lookvecZ,
        int health, int state, Monster_Type type)
        : id(monsterId), x(startX), y(startY), z(startZ),
        lookX(lookvecX), lookY(lookvecY), lookZ(lookvecZ),
        hp(health), state(state), type(type) {}

    void update(float newX, float newY, float newZ, float newlookVecX, float newlookVecY, float newlookVecZ, int newState, int newHp)
    {
        x = newX;
        y = newY;
        z = newZ;
        lookX = newlookVecX;
        lookY = newlookVecY;
        lookZ = newlookVecZ;
        state = newState;
        hp = newHp;
    }

    std::string Serialize()
    {
        return "MONSTER_DATA," + std::to_string(id) + "," +
            std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + "," +
            std::to_string(lookX) + "," + std::to_string(lookY) + "," + std::to_string(lookZ) + "," +
            std::to_string(hp) + "," + std::to_string(state) + "," +
            std::to_string(static_cast<int>(type));
    }
};
