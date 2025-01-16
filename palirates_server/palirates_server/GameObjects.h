#pragma once
#include <string>

struct Position
{
    float x, y;
};

class Player 
{
public:
    int id;
    Position pos;
    std::string action;

    Player(int playerId, float x, float y, const std::string& act)
        : id(playerId), pos{ x, y }, action(act) {}

    void update(float x, float y, const std::string& act) 
    {
        pos.x = x;
        pos.y = y;
        action = act;
    }
};

class Monster 
{
public:
    int id;
    Position pos;
    std::string action;

    Monster(int monsterId, float x, float y, const std::string& act)
        : id(monsterId), pos{ x, y }, action(act) {}

    virtual void update() 
    {
        // ���� �̵� ����
    }
};

class BossMonster : public Monster
{
public:
    BossMonster(int monsterId, float x, float y, const std::string& act)
        : Monster(monsterId, x, y, act) {}

    void update() override
    {
        // ���� ������ Ư�� ����
    }
};

struct Particle 
{
    Position pos;
    std::string type;

    Particle(float x, float y, const std::string& particleType)
        : pos{ x, y }, type(particleType) {}
};
