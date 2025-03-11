#pragma once
#include <iostream>

class Player
{
public:
    int id;
    float x, y, z;
    int state;

    Player(int playerId, float startX, float startY, float startZ, int startState = 0)
        : id(playerId), x(startX), y(startY), z(startZ), state(startState) {}

    void setPosition(float newX, float newY, float newZ)
    {
        x = newX;
        y = newY;
        z = newZ;
    }
};
