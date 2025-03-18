#pragma once
#include <iostream>

class GameCharacter
{
public:
    int id;
    float x, y, z;
    int state;

    GameCharacter() : id(-1), x(0), y(0), z(0), state(0) {}

    GameCharacter(int playerId, float startX, float startY, float startZ, int startState = 0)
        : id(playerId), x(startX), y(startY), z(startZ), state(startState) {}

    void setPosition(float newX, float newY, float newZ)
    {
        x = newX;
        y = newY;
        z = newZ;
    }

    void setState(int newState)
    {
        state = newState;
    }

    void printPosition()
    {
        std::cout << "캐릭터 " << id << " 위치: (" << x << ", " << y << ", " << z << "), 상태: " << state << std::endl;
    }
};
