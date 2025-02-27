#pragma once
#include <string>

struct Player
{
    int id;
    float x, y, z;
    int state; // 0: idle, 1: 이동, 2: 공격

    Player() : id(-1), x(0), y(0), z(0), state(0) {}

    Player(int playerId, float startX, float startY, float startZ, int startState = 0)
        : id(playerId), x(startX), y(startY), z(startZ), state(startState) {}

    void update(float newX, float newY, float newZ, int newState)
    {
        x = newX;
        y = newY;
        z = newZ;
        state = newState;
    }

    std::string Serialize()
    {
        return "PLAYER_DATA," + std::to_string(id) + "," +
            std::to_string(x) + "," + std::to_string(y) + "," +
            std::to_string(z) + "," + std::to_string(state);
    }
};
