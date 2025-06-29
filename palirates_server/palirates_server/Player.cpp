#include "stdafx.h"
#include "Player.h"

Player::Player(int playerId) : Skinned_GameObject()
{
    player_id = playerId;
    player_state = Player_State::Idle;
    inputDirection = { 0.0f, 0.0f, 0.0f };
}

void Player::key_input(uint32_t keyState)
{
    inputDirection = { 0.0f, 0.0f, 0.0f };

    if (keyState & 0x01)
    {
        inputDirection.x += 1.0f;
        test_value += 1.0f;
    }
    if (keyState & 0x02)
    {
        inputDirection.x -= 1.0f;
        test_value += 1.0f;
    }
    if (keyState & 0x04)
    {
        inputDirection.z += 1.0f;
        test_value += 1.0f;
    }
    if (keyState & 0x08)
    {
        inputDirection.z -= 1.0f;
        test_value += 1.0f;
    }

    //if (inputDirection.x != 0.0f || inputDirection.z != 0.0f)
    //    player_state = Player_State::Walk;
    //else
    //    player_state = Player_State::Idle;

    std::cout << "client ID : " << player_id << ", value : " << test_value << "\n";
}

void Player::animate(float Elapsedtime)
{
   
}

void Player::update()
{
    float speed = 5.0f;
    float dt = 1.0f / 60.0f;

    XMFLOAT3 pos = GetPosition();
    pos.x += inputDirection.x * speed * dt;
    pos.z += inputDirection.z * speed * dt;

    SetPosition(pos);
}