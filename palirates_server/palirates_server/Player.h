#pragma once
#include <iostream>

struct Player
{
    int id;
    float x, y, z;
    int state;

    Player(int playerId, float startX, float startY, float startZ, int startState = 0)
        : id(playerId), x(startX), y(startY), z(startZ), state(startState) {}

    void update(float newX, float newY, float newZ, int newState)
    {
        x = newX;
        y = newY;
        z = newZ;
        state = newState;
    }
};
