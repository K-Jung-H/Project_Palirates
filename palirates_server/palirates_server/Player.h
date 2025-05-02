#pragma once
#include <iostream>

enum class EState : int
{
    Idle,
    Walk,
    Run,
    Jump,
    Attack,
    Dash
};

class Player
{
public:
    int id;
    float x, y, z;
    float lookX, lookY, lookZ;
    EState state;

    Player(int playerId, float startX, float startY, float startZ,
        float startLookX, float startLookY, float startLookZ, int startState = 1)
        : id(playerId), x(startX), y(startY), z(startZ),
        lookX(startLookX), lookY(startLookY), lookZ(startLookZ),
        state(static_cast<EState>(startState)) {}

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

	void printInfo()
	{
        std::cout << "캐릭터 " << id << " 위치: (" << x << ", " << y << ", " << z << "), 상태: " << static_cast<int>(state) << std::endl;
	}
};