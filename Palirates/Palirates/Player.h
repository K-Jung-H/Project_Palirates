#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#include "Object_StateMachine.h"
#include "Object.h"
#include "Camera.h"

enum AnimationTrack
{
	TRACK_IDLE = 0,
	TRACK_RUN_FORWARD_LEFT = 1,
	TRACK_RUN_FORWARD = 2,
	TRACK_RUN_FORWARD_RIGHT = 3,
	TRACK_RUN_BACKWARD_LEFT = 4,
	TRACK_RUN_BACKWARD = 5,
	TRACK_RUN_BACKWARD_RIGHT = 6,
	TRACK_RUN_LEFT = 7, 
	TRACK_RUN_RIGHT = 8, 
	TRACK_DIVEROLL_FORWARD = 9,
	TRACK_KNOCK_DOWN = 10,
	TRACK_GET_UP = 11
};

class CPlayer : public CGameObject
{
protected:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	LPVOID						m_pPlayerUpdatedContext = NULL;
	LPVOID						m_pCameraUpdatedContext = NULL;

	CCamera						*m_pCamera = NULL;
	CHeightMapTerrain* last_tile_ptr = NULL;

	float stateElapsedTime{ 0.0f };


	bool Anime_test_FallingLoop = false;
	float m_fFallingTimer = 0.0f;

	float moveX{ 0.0f };
	float moveZ{ 0.0f };

	//=================서버=================
	int id;  
	int state;

private:
	std::unique_ptr<StateMachine> m_StateMachine;

public:
	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }

	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

//	virtual void Animate(float fTimeElapsed);

	virtual void Animate_test();

	virtual void Update(float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

	virtual CHeightMapTerrain*& Get_Last_Tile() { return last_tile_ptr; }


	virtual void FallingTimer_Reset() { m_fFallingTimer = 0.0f; }

	std::unique_ptr<StateMachine>& GetStateMachine() { return m_StateMachine; }

	void SetStateElapsedTime(float time) { stateElapsedTime = time; }

	void SetMoveX(float x) { moveX = x; }
	void SetMoveZ(float x) { moveZ = x; }
	float GetMoveX() { return moveX; }
	float GetMoveZ() { return moveZ; }

	std::vector<float> prevWeights;
	std::vector<float> targetWeights;


	//=================서버=================
	CPlayer::CPlayer(int playerId, float startX, float startY, float startZ, int startState)
		: id(playerId), state(startState)
	{
		m_xmf3Position = XMFLOAT3(startX, startY, startZ);
	}

	int GetID() const { return id; }
	void SetID(int playerId) { id = playerId; }

	int GetState() const { return state; }
	void SetState(int newState) { state = newState; }

	std::string Serialize();
};


class CSoundCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSoundCallbackHandler() { }
	~CSoundCallbackHandler() { }

public:
	virtual void HandleCallback(void *pCallbackData, float fTrackPosition); 
};

class CTerrainPlayer : public CPlayer
{
private:
	bool On_Ground = false;

public:
	CTerrainPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~CTerrainPlayer();

public:
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);

	virtual void Move(DWORD nDirection, float fDistance, bool bVelocity = false);

	virtual void Animate(float fTimeElapsed);
	virtual void Update(float fTimeElapsed);

	void AlignWithNormal(XMFLOAT3 normal);
	virtual CHeightMapTerrain*& Get_Last_Tile() { return last_tile_ptr; }
};

