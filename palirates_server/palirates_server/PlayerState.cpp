#include "stdafx.h"
#include "Player.h"
#include "PlayerState.h"
#include "PlayerStateMachine.h"
#include "AnimationRegistry.h"
#include "Scene.h"
#include <memory>

/////////////////////////// normal ///////////////////////////////

void PlayerNormalState::Enter(Player* player, PlayerStateMachine* sm) {
	//std::cout << "PlayerNormalState Enter" << std::endl;
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_IDLE, 1.0f);
	player->PlayAnimation(State::Idle);
}

void PlayerNormalState::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	/*if (player->GetAnimationSyncData().changedStateNum == int(State::Dive)) {
		player->motion_blur = true;
	}
	else player->motion_blur = false;*/
}

void PlayerNormalState::Exit(Player* player) {
	//std::cout << "PlayerNormalState Exit" << std::endl;
	//player->motion_blur = false;
}

/////////////////////////// attack1 ///////////////////////////////

void PlayerAttack1State::Enter(Player* player, PlayerStateMachine* sm) {
	player->Weapon_ptr->SetCanCollide(true);
	//std::cout << "PlayerAttack1State Enter" << std::endl;
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_ATTACK1, 1.0f);
	sm->animController->m_pAnimationTracks[TRACK_ATTACK1].m_fPosition = 0.0f;
	player->PlayAnimation(State::Attack1);
}

void PlayerAttack1State::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	if (sm->animController->m_pAnimationTracks[TRACK_ATTACK1].m_bFinished) {
		sm->animController->m_pAnimationTracks[TRACK_ATTACK1].m_bFinished = false;
		//std::cout << "PlayerAttackState finished" << std::endl;
		sm->ChangeState(std::make_unique<PlayerNormalState>());
	}
}

void PlayerAttack1State::Exit(Player* player) {
	player->Weapon_ptr->SetCanCollide(false);
	//std::cout << "PlayerAttack1State Exit" << std::endl;
}

/////////////////////////// attack2 ///////////////////////////////

void PlayerAttack2State::Enter(Player* player, PlayerStateMachine* sm) {
	player->Weapon_ptr->SetCanCollide(true);
	//std::cout << "PlayerAttack2State Enter" << std::endl;
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_ATTACK2, 1.0f);
	sm->animController->m_pAnimationTracks[TRACK_ATTACK2].m_fPosition = 0.0f;
	player->PlayAnimation(State::Attack2);
}

void PlayerAttack2State::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	if (sm->animController->m_pAnimationTracks[TRACK_ATTACK2].m_bFinished) {
		sm->animController->m_pAnimationTracks[TRACK_ATTACK2].m_bFinished = false;
		//std::cout << "PlayerAttackState finished" << std::endl;
		sm->ChangeState(std::make_unique<PlayerNormalState>());
	}
}

void PlayerAttack2State::Exit(Player* player) {
	player->Weapon_ptr->SetCanCollide(false);
	//std::cout << "PlayerAttack2State Exit" << std::endl;
}

/////////////////////////// attack3 ///////////////////////////////

void PlayerAttack3State::Enter(Player* player, PlayerStateMachine* sm) {
	player->Weapon_ptr->SetCanCollide(true);
	std::cout << "PlayerAttack3State Enter" << std::endl;
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_ATTACK3, 1.0f);
	sm->animController->m_pAnimationTracks[TRACK_ATTACK3].m_fPosition = 0.0f;
	player->PlayAnimation(State::Attack3);
}

void PlayerAttack3State::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	cout << sm->animController->m_pAnimationTracks[TRACK_ATTACK3].m_fPosition << ", " << sm->animController->m_pAnimationTracks[TRACK_ATTACK3].m_fWeight << "\n";
	if (sm->animController->m_pAnimationTracks[TRACK_ATTACK3].m_bFinished) {
		sm->animController->m_pAnimationTracks[TRACK_ATTACK3].m_bFinished = false;
		//std::cout << "PlayerAttackState finished" << std::endl;
		sm->ChangeState(std::make_unique<PlayerNormalState>());
	}
}

void PlayerAttack3State::Exit(Player* player) {
	player->Weapon_ptr->SetCanCollide(false);
	//std::cout << "PlayerAttack3State Exit" << std::endl;
}

/////////////////////////// GetHit ///////////////////////////////

void PlayerGetHitState::Enter(Player* player, PlayerStateMachine* sm) {
	player->SetCanCollide(false);
	if (!player->BreathHit)
		player->SetIsInvincible(true);
	//std::cout << "PlayerGetHitState Enter" << std::endl;
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_GET_HIT_F2, 1.0f);
	sm->animController->m_pAnimationTracks[TRACK_GET_HIT_F2].m_fPosition = 0.0f;
	//std::cout << "PlayerGetHitState Enter" << std::endl;
	player->PlayAnimation(State::Get_Hit_F2);
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
	if (player->BreathHit) {
		player->BreathHit = false;
	}
	//std::cout << "PlayerGetHitState Exit" << std::endl;
}

/////////////////////////// Dead ///////////////////////////////

void PlayerDeadState::Enter(Player* player, PlayerStateMachine* sm) {
	player->SetCanCollide(false);
	player->SetIsInvincible(true);
	player->bDead = true;
	player->SetHP(0.0f);
	//std::cout << "PlayerGetHitState Enter" << std::endl;
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_KNOCK_DOWN, 1.0f);
	sm->animController->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_fPosition = 0.0f;
	//std::cout << "PlayerDeadState Enter" << std::endl;
	player->PlayAnimation(State::Knock_Down);
}

void PlayerDeadState::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	constexpr float respawntime = 5.0f;
	static float time = 0.0;
	time += deltaTime;
	if (time > respawntime) {
		time = 0.0f;
		sm->ChangeState(std::make_unique<PlayerGetUpState>());
	}
}

void PlayerDeadState::Exit(Player* player) {
	if (player->BreathHit) {
		player->BreathHit = false;
	}
}

/////////////////////////// GetUp ///////////////////////////////

void PlayerGetUpState::Enter(Player* player, PlayerStateMachine* sm) {
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_GET_UP, 1.0f);
	sm->animController->m_pAnimationTracks[TRACK_GET_UP].m_fPosition = 0.0f;
	player->PlayAnimation(State::Get_Up);
}

void PlayerGetUpState::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	if (sm->animController->m_pAnimationTracks[TRACK_GET_UP].m_bFinished) {
		sm->animController->m_pAnimationTracks[TRACK_GET_UP].m_bFinished = false;
		sm->ChangeState(std::make_unique<PlayerNormalState>());
	}
}

void PlayerGetUpState::Exit(Player* player) {
	player->SetCanCollide(true);
	player->SetIsInvincible(false);
	player->SetHP(50.0f);
	player->bDead = false;
}

/////////////////////////// Dive ///////////////////////////////

void PlayerDiveState::Enter(Player* player, PlayerStateMachine* sm) {
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(TRACK_DIVEROLL_FORWARD, 1.0f);
	sm->animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_fPosition = 0.0f;
	player->PlayAnimation(State::Dive);
	if (auto controller = player->GetSkinnedAnimationController()) {
		//controller->AdvanceTime(0.0f, player);
		controller->m_xmf3PrevHipsPosition = controller->HipsPosition;
	}
	auto currScene = dynamic_cast<Stage_Scene*>(player->m_pOwnerScene);
	if (currScene) {
		StateChangeInfo data;
		data.ID = player->GetID();
		data.stateNum = int(State::Dive);
		currScene->QueueStateChangeCommand(data);
		cout << "queue in " << "\n";
	}
}

void PlayerDiveState::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	if (sm->animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished) {
		sm->animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished = false;
		sm->ChangeState(std::make_unique<PlayerNormalState>());
	}
}

void PlayerDiveState::Exit(Player* player) {
}