#pragma once
#include "PlayerState.h"

class Player;

class PlayerStateMachine
{
protected:
	Player* m_pOwner = nullptr;
	std::unique_ptr<PlayerState> currentState;
	//State currentState = State::Idle;
	int lastStateChange = -1;

public:
	void update(float deltaTime);
	void ChangeState(std::unique_ptr<PlayerState> newState);
};