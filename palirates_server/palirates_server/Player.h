#pragma once
#include <iostream>
#include <DirectXMath.h>

using namespace DirectX;

enum class EState : int
{
    Idle,
    Walk,
    Run,
    Jump,
    Attack,
    Dash,
    ETC
};


class Player
{
public:
    int id;
    XMFLOAT3 Position;
    XMFLOAT3 lookVector;
    EState state;
    
    std::vector<float> animPositions;
    std::vector<float> animWeights;


    Player(int playerId, XMFLOAT3 start_pos, XMFLOAT3 start_lookvector) :
        id(playerId),
        Position(start_pos),
        lookVector(start_lookvector)
    {
        state = EState::Idle;
    }

    void setPosition(XMFLOAT3 new_pos) { setPosition(new_pos.x, new_pos.y, new_pos.z); }
    void setPosition(float newX, float newY, float newZ)
    {
        Position.x = newX;
        Position.y = newY;
        Position.z = newZ;
    }

    void setLookVec(XMFLOAT3 newLook) { setLookVec(newLook.x, newLook.y, newLook.z); }
    void setLookVec(float newLookX, float newLookY, float newLookZ)
    {
        lookVector.x = newLookX;
        lookVector.y = newLookY;
        lookVector.z = newLookZ;
    }


    void setState(::EState newState) { state = newState; }

    void Update(uint32_t keyState) {}


    void printInfo()
    {
        std::cout << "캐릭터 " << id << " 위치: (" << Position.x << ", " << Position.y << ", " << Position.z << "), 상태: " << static_cast<int>(state) << std::endl;
    }


};