#include "stdafx.h"
#include "Player.h"
#include "PlayerState.h"
#include "PlayerStateMachine.h"
#include "AnimationRegistry.h"
#include "Scene.h"
#include <memory>

void PlayerState::PrepareForStateEnter(State state, Player* player, PlayerStateMachine* sm)
{
	auto trackIdx = AnimationRegistry::GetPlayerAnimationTrack(state);
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	sm->animController->SetTrackWeight(trackIdx, 1.0f);
	sm->animController->m_pAnimationTracks[trackIdx].m_fPosition = 0.0f;
	player->PlayAnimation(state);
	if (auto controller = player->GetSkinnedAnimationController()) {
		controller->m_xmf3PrevHipsPosition = controller->HipsPosition;
	}
	auto currScene = dynamic_cast<Stage_Scene*>(player->m_pOwnerScene);
	if (currScene) {
		StateChangeInfo data;
		data.ID = player->Client_ID;
		data.stateNum = int(state);
		currScene->QueueStateChangeCommand(data);
		//cout << "queue in " << "\n";
	}
}

/////////////////////////// normal ///////////////////////////////

void PlayerNormalState::Enter(Player* player, PlayerStateMachine* sm) {
	//std::cout << "PlayerNormalState Enter" << std::endl;
	PrepareForStateEnter(State::Idle, player, sm);
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

/////////////////////////// run ///////////////////////////////

void PlayerRunState::Enter(Player* player, PlayerStateMachine* sm) {
	//std::cout << "PlayerNormalState Enter" << std::endl;
	PrepareForStateEnter(State::Run, player, sm);
}

void PlayerRunState::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	/*if (player->GetAnimationSyncData().changedStateNum == int(State::Dive)) {
		player->motion_blur = true;
	}
	else player->motion_blur = false;*/
}

void PlayerRunState::Exit(Player* player) {
	//std::cout << "PlayerNormalState Exit" << std::endl;
	//player->motion_blur = false;
}

/////////////////////////// attack1 ///////////////////////////////

void PlayerAttack1State::Enter(Player* player, PlayerStateMachine* sm) {
	//player->Weapon_ptr->SetCanCollide(true);
	for (auto& w : player->Weapon_ptr) {
		w->SetCanCollide(true);
	}
	//std::cout << "PlayerAttack1State Enter" << std::endl;
	PrepareForStateEnter(State::Attack1, player, sm);
}

void PlayerAttack1State::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	if (sm->animController->m_pAnimationTracks[TRACK_ATTACK1].m_bFinished) {
		sm->animController->m_pAnimationTracks[TRACK_ATTACK1].m_bFinished = false;
		//std::cout << "PlayerAttackState finished" << std::endl;
		sm->ChangeState(std::make_unique<PlayerNormalState>());
	}
}

void PlayerAttack1State::Exit(Player* player) {
	//player->Weapon_ptr->SetCanCollide(false);
	for (auto& w : player->Weapon_ptr) {
		w->SetCanCollide(false);
	}
	//std::cout << "PlayerAttack1State Exit" << std::endl;
}

/////////////////////////// attack2 ///////////////////////////////

void PlayerAttack2State::Enter(Player* player, PlayerStateMachine* sm) {
	//player->Weapon_ptr->SetCanCollide(true);
	for (auto& w : player->Weapon_ptr) {
		w->SetCanCollide(true);
	}
	//std::cout << "PlayerAttack2State Enter" << std::endl;
	PrepareForStateEnter(State::Attack2, player, sm);
}

void PlayerAttack2State::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	if (sm->animController->m_pAnimationTracks[TRACK_ATTACK2].m_bFinished) {
		sm->animController->m_pAnimationTracks[TRACK_ATTACK2].m_bFinished = false;
		//std::cout << "PlayerAttackState finished" << std::endl;
		sm->ChangeState(std::make_unique<PlayerNormalState>());
	}
}

void PlayerAttack2State::Exit(Player* player) {
	//player->Weapon_ptr->SetCanCollide(false);
	for (auto& w : player->Weapon_ptr) {
		w->SetCanCollide(false);
	}
	//std::cout << "PlayerAttack2State Exit" << std::endl;
}

/////////////////////////// attack3 ///////////////////////////////

void PlayerAttack3State::Enter(Player* player, PlayerStateMachine* sm) {
	//player->Weapon_ptr->SetCanCollide(true);
	for (auto& w : player->Weapon_ptr) {
		w->SetCanCollide(true);
	}
	std::cout << "PlayerAttack3State Enter" << std::endl;
	PrepareForStateEnter(State::Attack3, player, sm);
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
	//player->Weapon_ptr->SetCanCollide(false);
	for (auto& w : player->Weapon_ptr) {
		w->SetCanCollide(false);
	}
	//std::cout << "PlayerAttack3State Exit" << std::endl;
}

/////////////////////////// GetHit ///////////////////////////////

void PlayerGetHitState::Enter(Player* player, PlayerStateMachine* sm) {
	player->SetCanCollide(false);
	if (!player->BreathHit)
		player->SetIsInvincible(true);
	//std::cout << "PlayerGetHitState Enter" << std::endl;
	PrepareForStateEnter(State::Get_Hit_F2, player, sm);
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
	PrepareForStateEnter(State::Knock_Down, player, sm);
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
	PrepareForStateEnter(State::Get_Up, player, sm);
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
	PrepareForStateEnter(State::Dive, player, sm);
}

void PlayerDiveState::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	if (sm->animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished) {
		sm->animController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_bFinished = false;
		sm->ChangeState(std::make_unique<PlayerNormalState>());
	}
}

void PlayerDiveState::Exit(Player* player) {
}

/////////////////////////// Observer ///////////////////////////////

void PlayerObserverState::Enter(Player* player, PlayerStateMachine* sm) {
	for (int i = 0; i < sm->animController->m_nAnimationTracks; ++i) {
		sm->animController->m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
	PrepareForStateEnter(State::Idle, player, sm);
	player->SetCanCollide(false);
	player->SetIsInvincible(true);
	auto currPos = player->GetPosition();
	currPos.y = 100.0f;
	player->SetPosition(currPos);
}

void PlayerObserverState::Update(Player* player, float deltaTime, PlayerStateMachine* sm) {
	uint32_t moveMask = sm->lastMoveMask;
	if (!moveMask) return; 

	XMFLOAT3 forward = player->GetLook();
	forward.y = 0.0f;
	forward = Vector3::Normalize(forward);

	XMFLOAT3 right = player->GetRight();
	right.y = 0.0f;
	right = Vector3::Normalize(right);

	XMFLOAT3 dir = { 0, 0, 0 };

	if (moveMask & INPUT_W) dir = Vector3::Add(dir, forward);
	if (moveMask & INPUT_S) dir = Vector3::Subtract(dir, forward);
	if (moveMask & INPUT_D) dir = Vector3::Add(dir, right);
	if (moveMask & INPUT_A) dir = Vector3::Subtract(dir, right);

	if (Vector3::LengthSquared(dir) > 0.0f) {
		dir = Vector3::Normalize(dir);
		float speed = 900.0f;
		XMFLOAT3 deltaMove = Vector3::ScalarProduct(dir, speed * deltaTime, false);
		player->Move(deltaMove); 
	}
}

void PlayerObserverState::Exit(Player* player) {
	player->SetCanCollide(true);
	player->SetIsInvincible(false);
	auto currPos = player->GetPosition();
	currPos.y = 0.0f;
	player->SetPosition(currPos);
}