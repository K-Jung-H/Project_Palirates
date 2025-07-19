#pragma once
#include "PlayerState.h"

class Player;
class CAnimationController;

class PlayerStateMachine
{
protected:
	Player* m_pOwner = nullptr;
	std::unique_ptr<PlayerState> currentState;
	State currentStateEnum = State::Idle;
	int lastStateChange = -1;
	
public:
	PlayerStateMachine(Player* owner)
		: m_pOwner(owner) {
		currentState = std::make_unique<PlayerNormalState>();
	}

	std::shared_ptr<CAnimationController> animController;
	int n_Ani{ 0 };

	void update(float deltaTime);
	void ChangeState(std::unique_ptr<PlayerState> newState);
	int GetCurrentStateAsInt() { return lastStateChange; }
	State GetCurrentStateEnum() const { return currentStateEnum; }
	PlayerState* GetCurrentState() const { return currentState.get(); }
};