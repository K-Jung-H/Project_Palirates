#include "stdafx.h"
#include "Player.h"
#include "PlayerState.h"
#include "PlayerStateMachine.h"
#include "AnimationRegistry.h"
#include <memory>

/////////////////////////// normal ///////////////////////////////

void PlayerNormalState::Enter(Player* player, PlayerStateMachine* sm) {
	//std::cout << "PlayerNormalState Enter" << std::endl;
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_IDLE, 1.0f);
}

void PlayerNormalState::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {

}

void PlayerNormalState::Exit(Player* player) {
	//std::cout << "PlayerNormalState Exit" << std::endl;
}

/////////////////////////// attack ///////////////////////////////

void PlayerAttackState::Enter(Player* player, PlayerStateMachine* sm) {
	player->Weapon_ptr->SetCanCollide(true);
	//std::cout << "PlayerAttackState Enter" << std::endl;
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_ATTACK1, 1.0f);
	sm->animController->m_pAnimationTracks[TRACK_ATTACK1].m_fPosition = 0.0f;
}

void PlayerAttackState::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	if (sm->animController->m_pAnimationTracks[TRACK_ATTACK1].m_bFinished) {
		sm->animController->m_pAnimationTracks[TRACK_ATTACK1].m_bFinished = false;
		//std::cout << "PlayerAttackState finished" << std::endl;
		sm->ChangeState(std::make_unique<PlayerNormalState>());
	}
}

void PlayerAttackState::Exit(Player* player) {
	player->Weapon_ptr->SetCanCollide(false);
	//std::cout << "PlayerAttackState Exit" << std::endl;
}

/////////////////////////// GetHit ///////////////////////////////

void PlayerGetHitState::Enter(Player* player, PlayerStateMachine* sm) {
	player->SetCanCollide(false);
	player->SetIsInvincible(true);
	//std::cout << "PlayerGetHitState Enter" << std::endl;
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_GET_HIT_F2, 1.0f);
	sm->animController->m_pAnimationTracks[TRACK_GET_HIT_F2].m_fPosition = 0.0f;
}

void PlayerGetHitState::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	if (sm->animController->m_pAnimationTracks[TRACK_GET_HIT_F2].m_bFinished) {
		sm->animController->m_pAnimationTracks[TRACK_GET_HIT_F2].m_bFinished = false;
		//std::cout << "PlayerGetHitState finished" << std::endl;
		sm->ChangeState(std::make_unique<PlayerNormalState>());
	}
}

void PlayerGetHitState::Exit(Player* player) {
	player->SetCanCollide(true);
	//std::cout << "PlayerGetHitState Exit" << std::endl;
}

