#include "stdafx.h"
#include "Player.h"

Player::Player(int playerId) : Skinned_GameObject()
{
    player_id = playerId;
    player_state = Player_State::Idle;
}

void Player::key_input(uint32_t keyState)
{

}

void Player::animate(float Elapsedtime)
{

}

void Player::update()
{

}
