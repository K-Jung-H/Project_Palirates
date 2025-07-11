#include "stdafx.h"
#include "Player.h"

Player::Player(int playerId) : Skinned_GameObject()
{
    model_index = playerId;
    player_state = Player_State::Idle;

    m_localOBB = std::make_shared<BoundingOrientedBox>(
        XMFLOAT3(0.0f, 8.0f, 0.0f),
        XMFLOAT3(4.0f, 8.0f, 4.0f),
        XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
    );

    m_worldOBB = std::make_shared<BoundingOrientedBox>();
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
    if (!m_localOBB || !m_worldOBB) return;

    XMMATRIX worldMatrix = XMLoadFloat4x4(&m_xmf4x4World);
    m_localOBB->Transform(*m_worldOBB, worldMatrix);
}

void Player::Set_Collider_OBB_Center(const XMFLOAT3& newWorldCenter)
{
    const XMFLOAT3& offset = m_localOBB->Center;

    XMFLOAT3 newPos = {
        newWorldCenter.x - offset.x,
        newWorldCenter.y - offset.y,
        newWorldCenter.z - offset.z
    };

    SetPosition(newPos);
    UpdateWorldOBB(); 
}