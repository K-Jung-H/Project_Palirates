#include "stdafx.h"
#include "Player.h"
#include "GameObject.h"
#include "PlayerState.h"
#include "PlayerStateMachine.h"

void PlayerStateMachine::update(float deltaTime)
{
    if (currentState) {
        currentState->Update(m_pOwner, deltaTime, this);
    }
}

void PlayerStateMachine::ChangeState(std::unique_ptr<PlayerState> newState)
{
    if (!newState) return;

    if (currentState)
        currentState->Exit(m_pOwner);

    currentState = std::move(newState);

    if (currentState)
        currentState->Enter(m_pOwner, this);
}