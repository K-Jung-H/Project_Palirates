//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer() 
	: m_StateMachine(std::make_unique<PlayerStateMachine>(this))
{
	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;

}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController.reset();
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		if (Vector3::Length(xmf3Shift) > 0.0f)
		{
			xmf3Shift = Vector3::Normalize(xmf3Shift);
			xmf3Shift = Vector3::Scale(xmf3Shift, fDistance);
		}

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		if (m_pCamera)
			m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Animate_test()
{
	if (Anime_test_FallingLoop) Anime_test_FallingLoop = false;
	else Anime_test_FallingLoop = true;
}

void CPlayer::Update(float fTimeElapsed)
{
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);

	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	float fMaxVelocityY = m_fMaxVelocityY;

	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}

	XMFLOAT3 look = GetLook();   
	XMFLOAT3 right = GetRight();

	XMFLOAT3 velocityXZ = XMFLOAT3(m_xmf3Velocity.x, 0.0f, m_xmf3Velocity.z);
	float velocityLength = Vector3::Length(velocityXZ);
	XMFLOAT3 normalizedVelocity = velocityLength > 0.0f ? Vector3::Normalize(velocityXZ) : XMFLOAT3(0, 0, 0);

	moveZ = Vector3::DotProduct(normalizedVelocity, look);  
	moveX = Vector3::DotProduct(normalizedVelocity, right); 

	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);

	if (fLength > m_fMaxVelocityY) 
		m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) 
		OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
		m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	if (m_pCameraUpdatedContext) 
		OnCameraUpdateCallback(fTimeElapsed);

	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) 
		m_pCamera->SetLookAt(m_xmf3Position);

	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);

	if (fDeceleration > fLength) 
		fDeceleration = fLength;

	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			pNewCamera = new CFirstPersonCamera(m_pCamera);
			break;
		case THIRD_PERSON_CAMERA:
			pNewCamera = new CThirdPersonCamera(m_pCamera);
			break;
		case SPACESHIP_CAMERA:
			pNewCamera = new CSpaceShipCamera(m_pCamera);
			break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareAnimate()
{
	m_xmf4x4Parent._11 = m_xmf3Right.x; m_xmf4x4Parent._12 = m_xmf3Right.y; m_xmf4x4Parent._13 = m_xmf3Right.z;
	m_xmf4x4Parent._21 = m_xmf3Up.x; m_xmf4x4Parent._22 = m_xmf3Up.y; m_xmf4x4Parent._23 = m_xmf3Up.z;
	m_xmf4x4Parent._31 = m_xmf3Look.x; m_xmf4x4Parent._32 = m_xmf3Look.y; m_xmf4x4Parent._33 = m_xmf3Look.z;
	m_xmf4x4Parent._41 = m_xmf3Position.x; m_xmf4x4Parent._42 = m_xmf3Position.y; m_xmf4x4Parent._43 = m_xmf3Position.z;

	m_xmf4x4Parent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4Parent);
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) 
		CGameObject::Render(pd3dCommandList, pCamera);
}


void CPlayer::SetLookDirection(const XMFLOAT3& look)
{
	CGameObject::SetLookDirection(look);
	m_xmf3Look = GetLook();
	m_xmf3Right = GetRight();
	m_xmf3Up = GetUp();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
#define _WITH_DEBUG_CALLBACK_DATA

void CSoundCallbackHandler::HandleCallback(void *pCallbackData, float fTrackPosition)
{
   _TCHAR *pWavName = (_TCHAR *)pCallbackData; 
#ifdef _WITH_DEBUG_CALLBACK_DATA
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s(%f)\n"), pWavName, fTrackPosition);
	OutputDebugString(pstrDebug);
#endif
#ifdef _WITH_SOUND_RESOURCE
   PlaySound(pWavName, ::ghAppInstance, SND_RESOURCE | SND_ASYNC);
#else
   PlaySound(pWavName, NULL, SND_FILENAME | SND_ASYNC);
#endif
}

CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext) : CPlayer()
{
	Object_type = OBJECT_TPYE_MAIN_PLAYER;

	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	  
	//CLoadedModelInfo *pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/First_Mate_v12.bin", NULL);
	//CLoadedModelInfo *pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Seaman_v12.bin", NULL);
	//CLoadedModelInfo *pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Wench_v12.bin", NULL);
	CLoadedModelInfo *pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Captain_v16.bin", NULL);
	//CLoadedModelInfo *pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Characters_ani12.bin", NULL);
	Set_Child(pAngrybotModel->m_pModelRootObject);

	n_Animation = 16;
	prevWeights.resize(n_Animation, 0.0f);
	targetWeights.resize(n_Animation, 0.0f);
	m_pSkinnedAnimationController = std::make_shared<CAnimationController>(pd3dDevice, pd3dCommandList, n_Animation, pAngrybotModel);
	//m_pSkinnedAnimationController->SetTrackWeight(TRACK_IDLE, 1.0f);
	//m_pSkinnedAnimationController->SetTrackWeight(1, 0.2f);
	//m_pSkinnedAnimationController->SetTrackWeight(2, 0.5f);
	for (int i = 0; i < n_Animation; ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
	}
	/*m_pSkinnedAnimationController->SetTrackEnable(TRACK_IDLE, true);
	m_pSkinnedAnimationController->SetTrackEnable(TRACK_RUN_FORWARD, false);
	for (int i = 2; i < n_Animation; ++i) {
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}*/
	for (int i = 0; i < n_Animation; ++i) {
		m_pSkinnedAnimationController->SetTrackEnable(i, true);
	}
	m_pSkinnedAnimationController->Bone_Info();
	// Once type Setting
	for (int i = 0; i < n_Animation; ++i) {
		if (kUpdateHipsTracks.contains(i)) {
			m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
		}
	}
	
	//m_pSkinnedAnimationController->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_nType = ANIMATION_TYPE_ONCE;
	//m_pSkinnedAnimationController->m_pAnimationTracks[TRACK_GET_UP].m_nType = ANIMATION_TYPE_ONCE;
	//m_pSkinnedAnimationController->m_pAnimationTracks[TRACK_ATTACK1].m_nType = ANIMATION_TYPE_ONCE;
	//m_pSkinnedAnimationController->m_pAnimationTracks[TRACK_ATTACK2].m_nType = ANIMATION_TYPE_ONCE;
	//m_pSkinnedAnimationController->m_pAnimationTracks[TRACK_ATTACK3].m_nType = ANIMATION_TYPE_ONCE;

	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else
//	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.2f, _T("Sound/Footstep01.wav"));
//	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.5f, _T("Sound/Footstep02.wav"));
//	m_pSkinnedAnimationController->SetCallbackKey(1, 2, 0.39f, _T("Sound/Footstep03.wav"));
#endif
	CAnimationCallbackHandler *pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;
	if (pTerrain != NULL)
		SetPosition(XMFLOAT3(25.0f, pTerrain->Get_Height(25.0f, 25.0f, true, last_tile_ptr), 25.0f));
	
	SetScale(XMFLOAT3(10.0f, 10.0f, 10.0f));
	if (pAngrybotModel) delete pAngrybotModel;
}

CTerrainPlayer::~CTerrainPlayer()
{
}

CCamera *CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -400.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(5000.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(1000.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case THIRD_PERSON_CAMERA:
			SetFriction(800.0f);
			SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
			SetMaxVelocityXZ(500.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		default:
			break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);

	float fHeight = pTerrain->Get_Height(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad, last_tile_ptr);

	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
		On_Ground = true;
	}
	else
		On_Ground = false;

#ifdef DEBUG_MESSAGE
#ifdef DEBUG_MESSAGE_TILE_MAP
	int tile_num = pTerrain->Get_Tile(xmf3PlayerPosition.x, xmf3PlayerPosition.z);


	string debug_message = "Tile_" + std::to_string(tile_num) + ",\t Height: " + std::to_string(fHeight) + "\n";
	DebugOutput(debug_message);
#endif
#endif
}

void CTerrainPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);

	float fHeight = pTerrain->Get_Height(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad, last_tile_ptr) + 5.0f;


	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}

void CTerrainPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
	}

	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

void CTerrainPlayer::Animate(float fTimeElapsed)
{
	OnPrepareAnimate();

	if (m_pSkinnedAnimationController)
	{
		/*if (Anime_test_FallingLoop)
			m_pSkinnedAnimationController->AdvanceTime2(fTimeElapsed, this);
		else*/
		if (Object_type == OBJECT_TPYE_MAIN_PLAYER && !CheckMultiMode()) {
			m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);
			GetStateMachine()->update(fTimeElapsed);
		}
		else if (Object_type == OBJECT_TPYE_PLAYER) {
			//m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);
			//GetStateMachine()->update(fTimeElapsed);
		}
	}

	if (On_Ground)
	{
		CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
		XMFLOAT3 xmf3PlayerPosition = GetPosition();
		XMFLOAT3 world_normal = pTerrain->Get_Mesh_Normal(xmf3PlayerPosition.x, xmf3PlayerPosition.z, last_tile_ptr);
		AlignWithNormal(world_normal);
	}

	shared_ptr<CGameObject> sibling_ptr = Get_Sibling();
	if (sibling_ptr != nullptr)
		sibling_ptr->Animate(fTimeElapsed);

	shared_ptr<CGameObject> child_ptr = Get_Child();
	if (child_ptr != nullptr)
		child_ptr->Animate(fTimeElapsed);

	
}

void CTerrainPlayer::Update(float fTimeElapsed)
{
	CPlayer::Update(fTimeElapsed);


	if (m_pSkinnedAnimationController)
	{

		if (Anime_test_FallingLoop) {
			if (m_fFallingTimer < 0.0f) {

				m_fFallingTimer += fTimeElapsed;

				float weight0 = 1.0f - (m_fFallingTimer / 1.0f); 
				m_pSkinnedAnimationController->SetTrackWeight(TRACK_RUN_FORWARD, weight0);

	
				float weight2 = m_fFallingTimer / 1.0f; 
				m_pSkinnedAnimationController->SetTrackWeight(TRACK_RUN_BACKWARD, weight2);

				m_pSkinnedAnimationController->SetTrackEnable(TRACK_IDLE, false);
				m_pSkinnedAnimationController->SetTrackEnable(TRACK_RUN_FORWARD, true);
				m_pSkinnedAnimationController->SetTrackEnable(TRACK_RUN_BACKWARD, true);
			}
			else {

				if (m_pSkinnedAnimationController->m_pAnimationTracks[TRACK_RUN_FORWARD].m_fWeight != 0.5f)
					m_pSkinnedAnimationController->SetTrackWeight(TRACK_RUN_FORWARD, 0.5f);
				if (m_pSkinnedAnimationController->m_pAnimationTracks[TRACK_RUN_BACKWARD].m_fWeight != 0.5f)
					m_pSkinnedAnimationController->SetTrackWeight(TRACK_RUN_BACKWARD, 0.5f);

				m_pSkinnedAnimationController->SetTrackEnable(TRACK_IDLE, false);
				m_pSkinnedAnimationController->SetTrackEnable(TRACK_RUN_FORWARD, true);
				m_pSkinnedAnimationController->SetTrackEnable(TRACK_RUN_BACKWARD, true);

				
			}
		}
		else {
		
		}
	}

}

void CTerrainPlayer::AlignWithNormal(XMFLOAT3& normal)
{
	m_xmf3Up = Vector3::Normalize(normal);

	if (fabs(Vector3::DotProduct(m_xmf3Look, m_xmf3Up)) > 0.99f)
		m_xmf3Look = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Right, m_xmf3Up, true));
	
	m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true));
	m_xmf3Look = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Right, m_xmf3Up, true));
}

ServerAnimationSyncData CTerrainPlayer::MakeSyncData()
{
	ServerAnimationSyncData data = CGameObject::MakeSyncData();
	data.currentState = GetStateMachine()->Get_State();
	for (int i = 0; i < n_Animation; i++) {
		data.trackPositions.push_back(GetSkinnedAnimationController()->m_pAnimationTracks[i].m_fPosition);
		data.Weights.push_back(GetSkinnedAnimationController()->m_pAnimationTracks[i].m_fWeight);
	}

	return data;
}

void CTerrainPlayer::ApplySyncData(const ServerAnimationSyncData& syncData)
{
	CGameObject::ApplySyncData(syncData);
	SetPosition(syncData.position);
	GetStateMachine()->SetState(syncData.currentState);
	//GetStateMachine()->changeState(syncData.currentState, Key_Value::None);
	for (int i = 0; i < n_Animation; i++) {
		GetSkinnedAnimationController()->m_pAnimationTracks[i].m_fPosition = syncData.trackPositions[i];
		GetSkinnedAnimationController()->m_pAnimationTracks[i].m_fWeight = syncData.Weights[i];
	}
	GetSkinnedAnimationController()->ApplyCurrentAnimationPose(this);
}


//==================================================================


Observer::Observer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
	: CPlayer()
{
	Object_type = OBJECT_TPYE_MAIN_PLAYER;
	n_Animation = 0;

	m_pCamera = ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);

	SetPosition(XMFLOAT3(0.0f, 50.0f, 0.0f));

}

Observer::~Observer()
{
}

CCamera* Observer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;

	default:
		ChangeCamera(FIRST_PERSON_CAMERA, fTimeElapsed);		
		return(m_pCamera);

	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

void Observer::OnPlayerUpdateCallback(float fTimeElapsed)
{
}

void Observer::OnCameraUpdateCallback(float fTimeElapsed)
{

}

void Observer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

void Observer::Animate(float fTimeElapsed)
{
	OnPrepareAnimate();
	CGameObject::Animate(fTimeElapsed);
}

void Observer::Update(float fTimeElapsed)
{
	CPlayer::Update(fTimeElapsed);
}


ServerAnimationSyncData Observer::MakeSyncData()
{
	ServerAnimationSyncData data = CGameObject::MakeSyncData();
	data.currentState = GetStateMachine()->Get_State();
	for (int i = 0; i < n_Animation; i++) 
	{
		data.trackPositions.push_back(GetSkinnedAnimationController()->m_pAnimationTracks[i].m_fPosition);
		data.Weights.push_back(GetSkinnedAnimationController()->m_pAnimationTracks[i].m_fWeight);
	}

	return data;
}

void Observer::ApplySyncData(const ServerAnimationSyncData& syncData)
{
	CGameObject::ApplySyncData(syncData);
	SetPosition(syncData.position);
	GetStateMachine()->SetState(syncData.currentState);
	for (int i = 0; i < n_Animation; i++) 
	{
		GetSkinnedAnimationController()->m_pAnimationTracks[i].m_fPosition = syncData.trackPositions[i];
		GetSkinnedAnimationController()->m_pAnimationTracks[i].m_fWeight = syncData.Weights[i];
	}
	GetSkinnedAnimationController()->ApplyCurrentAnimationPose(this);
}