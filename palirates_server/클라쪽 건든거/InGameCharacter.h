#pragma once
#include <iostream>

class InGameCharacter
{
public:
    int id;    // 플레이어 고유 ID
    float x, y, z;
    int state;  // 0: idle, 1: 이동, 2: 공격

    InGameCharacter(int playerId, float startX, float startY, float startZ, int startState = 0)
        : id(playerId), x(startX), y(startY), z(startZ), state(startState) {}

    void setPosition(float newX, float newY, float newZ)
    {
        x = newX;
        y = newY;
        z = newZ;
    }

    void printPosition() const
    {
        std::cout << "[플레이어 " << id << "] 현재 위치: (" << x << ", " << y << ", " << z << ")" << std::endl;
    }
};
