#pragma once
#include <string>
#include <iostream>
#include <cstdlib>

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
    bool inGame;

    Player() : id(0), pos{ 0, 0 }, action("idle"), inGame(false) {}

    Player(int playerId, float x, float y, const std::string& act)
        : id(playerId), pos{ x, y }, action(act), inGame(false) {}

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

    virtual void update(float x, float y, const std::string& act)
    {
        pos.x = x;
        pos.y = y;
        action = act;

        // 간단한 랜덤 이동 AI 추가
        pos.x += (rand() % 3 - 1) * 5;
        pos.y += (rand() % 3 - 1) * 5;
    }
};

class BossMonster : public Monster
{
public:
    BossMonster(int monsterId, float x, float y, const std::string& act)
        : Monster(monsterId, x, y, act) {}

    void update(float x, float y, const std::string& act) override
    {
        pos.x = x;
        pos.y = y;
        action = act;

        //간단한 이동 로직
        pos.x += (rand() % 5 - 2) * 10;
        pos.y += (rand() % 5 - 2) * 10;
    }
};
