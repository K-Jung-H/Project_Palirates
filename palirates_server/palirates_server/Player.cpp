#include "stdafx.h"
#include "Player.h"

Player::Player(int playerId) : Skinned_GameObject()
{
    model_index = playerId;
    player_state = Player_State::Idle;

    m_OBB = std::make_shared<BoundingOrientedBox>(XMFLOAT3(0.0f, 0.8f * 10.0f, 0.0f), XMFLOAT3(0.4f * 10.0f, 0.8f * 10.0f, 0.4f * 10.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
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

void Player::UpdateWorldOBB()
{
	if (!m_OBB) return;
	XMFLOAT3 player_pos = GetPosition();
	const float y_offset = m_OBB->Center.y;

	m_OBB->Center = XMFLOAT3(player_pos.x, player_pos.y + y_offset, player_pos.z);
}
