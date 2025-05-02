#pragma once
#include <iostream>
#include "Player.h"

class GameCharacter
{
public:
    int id;
    float x, y, z;
    float lookX, lookY, lookZ;
    EState state;

    GameCharacter() : id(-1), x(25), y(0), z(25), lookX(0), lookY(1), lookZ(0), state(EState::Idle) {}

    GameCharacter(int playerId, float startX, float startY, float startZ,
        float startLookX, float startLookY, float startLookZ, int startState = 1)
        : id(playerId), x(startX), y(startY), z(startZ),
        lookX(startLookX), lookY((startLookY == 0.0f) ? 1.0f : startLookY), lookZ(startLookZ),
        state(static_cast<EState>(startState)) {}

    GameCharacter(int playerId, float startX, float startY, float startZ,
        float startLookX, float startLookY, float startLookZ, EState startState)
        : id(playerId), x(startX), y(startY), z(startZ),
        lookX(startLookX), lookY((startLookY == 0.0f) ? 1.0f : startLookY), lookZ(startLookZ), state(startState) {}

    void setPosition(float newX, float newY, float newZ)
    {
        x = newX;
        y = newY;
        z = newZ;
    }

    void setLookVec(float newLookX, float newLookY, float newLookZ)
    {
        lookX = newLookX;
        lookY = newLookY;
        lookZ = newLookZ;
    }

    void setState(::EState newState)
    {
        state = newState;
    }

    void printPosition()
    {
        std::cout << "캐릭터 " << id << " 위치: (" << x << ", " << y << ", " << z << "), 상태: " << static_cast<int>(state) << std::endl;
    }
};
