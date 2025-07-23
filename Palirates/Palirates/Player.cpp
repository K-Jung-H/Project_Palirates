//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer() 
{
	
	m_StateMachine = std::make_unique<PlayerStateMachine>(this);
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
		if (!m_bSliding)
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
	if (m_bSliding)
	{
		float speed = Vector3::Length(m_xmf3Velocity) * 0.5;
		XMFLOAT3 direction = Vector3::Normalize(m_xmf3SlideVector);
		m_xmf3Velocity = Vector3::ScalarProduct(direction, speed, false);
		DisableSliding();
	}

	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);

	float fXZLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	if (fXZLength > m_fMaxVelocityXZ)
	{
		float ratio = m_fMaxVelocityXZ / fXZLength;
		m_xmf3Velocity.x *= ratio;
		m_xmf3Velocity.z *= ratio;
	}

	XMFLOAT3 look = GetLook();
	XMFLOAT3 right = GetRight();
	XMFLOAT3 velocityXZ = XMFLOAT3(m_xmf3Velocity.x, 0.0f, m_xmf3Velocity.z);
	float velocityLength = Vector3::Length(velocityXZ);
	XMFLOAT3 normalizedVelocity = (velocityLength > 0.0f) ? Vector3::Normalize(velocityXZ) : XMFLOAT3(0, 0, 0);

	moveZ = Vector3::DotProduct(normalizedVelocity, look);
	moveX = Vector3::DotProduct(normalizedVelocity, right);

	float fYLength = fabsf(m_xmf3Velocity.y);
	if (fYLength > m_fMaxVelocityY)
	{
		m_xmf3Velocity.y *= (m_fMaxVelocityY / fYLength);
	}

	XMFLOAT3 scaledVelocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(scaledVelocity, false);

	if (m_pPlayerUpdatedContext)
		OnPlayerUpdateCallback(fTimeElapsed);

	if (m_pCamera)
	{
		DWORD nCurrentCameraMode = m_pCamera->GetMode();
		if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
		{
			m_pCamera->Update(m_xmf3Position, fTimeElapsed);
			m_pCamera->SetLookAt(m_xmf3Position);
		}
		if (m_pCameraUpdatedContext)
			OnCameraUpdateCallback(fTimeElapsed);

		m_pCamera->RegenerateViewMatrix();
	}

	float speed = Vector3::Length(m_xmf3Velocity);
	float frictionAmount = m_fFriction * fTimeElapsed;
	if (frictionAmount > speed) frictionAmount = speed;

	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -frictionAmount, true));
}

shared_ptr<CCamera> CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	shared_ptr<CCamera> pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = std::make_shared<CFirstPersonCamera>(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = std::make_shared<CThirdPersonCamera>(m_pCamera);
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = std::make_shared<CSpaceShipCamera>(m_pCamera);
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
	if (nCameraMode == THIRD_PERSON_CAMERA || HasType(EObjectType::SelectPlayer))
		CGameObject::Render(pd3dCommandList, pCamera);
}


void CPlayer::SetLookDirection(const XMFLOAT3& look)
{
	CGameObject::SetLookDirection(look);
	m_xmf3Look = GetLook();
	m_xmf3Right = GetRight();
	m_xmf3Up = GetUp();

}

void CPlayer::SetupWeaponCollider()
{
	std::shared_ptr<CGameObject> model = FindFrame(WeaponName);

	if (!model || !model->m_pMesh) return;

	model->type = EObjectType::PlayerWeapon;

	XMFLOAT4X4 worldMatrixFloat = model->m_xmf4x4World;
	XMVECTOR scale, rotationQuat, translation;
	XMFLOAT4 quaternion;
	XMMATRIX worldMatrix = XMLoadFloat4x4(&worldMatrixFloat);

	if (XMMatrixDecompose(&scale, &rotationQuat, &translation, worldMatrix))
		XMStoreFloat4(&quaternion, rotationQuat);
	else
		quaternion = XMFLOAT4(0, 0, 0, 1);

	BoundingOrientedBox* obb = new BoundingOrientedBox(
		model->m_pMesh->GetAABBCenter(),
		model->m_pMesh->GetAABBExtents(),
		quaternion
	);

	model->Set_Collider(obb);
	model->bUpdateOBBOff();
	Weapon_ptr = model;
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

CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, void *pContext, int ModelNum) : CPlayer()
{
	type = EObjectType::MainPlayer;

	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	char* modelPaths[] = {
	"Model/Captain_v17.bin",
	"Model/Deckhand_v17.bin",
	"Model/Female_Pirate_v17.bin",
	"Model/First_Mate_v17.bin",
	"Model/Seaman_v17.bin",
	"Model/Skeleton_v17.bin"
	};

	const int modelCount = sizeof(modelPaths) / sizeof(modelPaths[0]);
	if (ModelNum < 0 || ModelNum >= modelCount)
		ModelNum = 0; 
	CLoadedModelInfo* pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, modelPaths[ModelNum], NULL);
	m_pRootModel = pAngrybotModel->m_pModelRootObject;

	n_Animation = 17;
	RootIndex = 2;
	prevWeights.resize(n_Animation, 0.0f);
	targetWeights.resize(n_Animation, 0.0f);
	m_pSkinnedAnimationController = std::make_shared<CAnimationController>(pd3dDevice, pd3dCommandList, n_Animation, pAngrybotModel);
	m_pSkinnedAnimationController->RootIndex = RootIndex;
	for (int i = 0; i < n_Animation; ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
	}
	for (int i = 0; i < n_Animation; ++i) {
		m_pSkinnedAnimationController->SetTrackEnable(i, true);
	}
	// Once type Setting
	for (int i = 0; i < n_Animation; ++i) {
		if (GetUpdateHipsTracks().contains(i)) {
			m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
		}
	}

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

	WeaponName = "SM_Wep_Cutlass_01";

	BoundingOrientedBox* body = new BoundingOrientedBox(
		XMFLOAT3(0.0f, 0.8f, 0.0f),
		XMFLOAT3(0.4f, 0.8f, 0.4f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
	);
	Set_Collider(body);

	if (pAngrybotModel) delete pAngrybotModel;
}

CTerrainPlayer::~CTerrainPlayer()
{
}

shared_ptr<CCamera> CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
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
			m_pCamera->GenerateProjectionMatrix(CAMERA_NEAR, CAMERA_FAR, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(50.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(1000.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(CAMERA_NEAR, CAMERA_FAR, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case THIRD_PERSON_CAMERA:
			SetFriction(500.0f);
			SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
			SetMaxVelocityXZ(100.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
			m_pCamera->GenerateProjectionMatrix(CAMERA_NEAR, CAMERA_FAR, ASPECT_RATIO, 60.0f);
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
	if (pTerrain) {
		XMFLOAT3 xmf3Scale = pTerrain->GetScale();
		XMFLOAT3 xmf3PlayerPosition = GetPosition();
		int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
		bool bReverseQuad = ((z % 2) != 0);

		float fHeight = pTerrain->Get_Height(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad, last_tile_ptr);
		ClampPositionToTerrainBounds(pTerrain);

		xmf3PlayerPosition = GetPosition();
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
	}

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
			if (auto* thirdPersonCam = dynamic_cast<CThirdPersonCamera*>(m_pCamera.get()))
			{
				thirdPersonCam->SetLookAt(GetPosition());
			}
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

void CTerrainPlayer::ClampPositionToTerrainBounds(CHeightMapTerrain*  terrain_obj)
{
	shared_ptr<CMesh> full_mesh = terrain_obj->Get_FullMesh();
	if (full_mesh)
	{
		XMFLOAT2 areaLT = terrain_obj->Get_Terrain_LT();
		XMFLOAT2 areaRB = terrain_obj->Get_Terrain_RB();
		XMFLOAT3 player_pos = GetPosition();

		player_pos.x = std::clamp(player_pos.x, areaLT.x, areaRB.x);
		player_pos.z = std::clamp(player_pos.z, areaLT.y, areaRB.y);
		SetPosition(player_pos);
		UpdateTransform(NULL);
	}
}


void CTerrainPlayer::Animate(float fTimeElapsed)
{
	OnPrepareAnimate();

	if (m_pSkinnedAnimationController)
	{
		if (HasType(EObjectType::MainPlayer) && !CheckMultiMode()) {
			m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);
			GetStateMachine()->update(fTimeElapsed);
		}
		else if (HasType(EObjectType::SelectPlayer)) {
			m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);
			GetStateMachine()->update(fTimeElapsed);
		}
	}

	if (On_Ground)
	{
		CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
		if (pTerrain) 
		{
			XMFLOAT3 xmf3PlayerPosition = GetPosition();
			XMFLOAT3 world_normal = pTerrain->Get_Mesh_Normal(xmf3PlayerPosition.x, xmf3PlayerPosition.z, last_tile_ptr);
			AlignWithNormal(world_normal);
		}
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
}

void CTerrainPlayer::AlignWithNormal(XMFLOAT3& normal)
{
	m_xmf3Up = Vector3::Normalize(normal);

	if (fabs(Vector3::DotProduct(m_xmf3Look, m_xmf3Up)) > 0.99f)
		m_xmf3Look = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Right, m_xmf3Up, true));
	
	m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true));
	m_xmf3Look = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Right, m_xmf3Up, true));
}

ServerSyncData CTerrainPlayer::MakeSyncData()
{
	ServerSyncData data = CGameObject::MakeSyncData();
	data.changedStateNum = GetStateMachine()->GetCurrentStateAsInt();

	return data;
}

void CTerrainPlayer::ApplySyncData(const ServerSyncData& syncData)
{
	CGameObject::ApplySyncData(syncData);

	auto controller = GetSkinnedAnimationController();
	if (!controller) return;
	controller->ResetWeight();
	auto track = controller->m_pAnimationTracks;
	vector<Animation_Sync> track_list = syncData.track_info_list;

	for(Animation_Sync animation_track_info : track_list)
	{
		track[animation_track_info.track_index].m_fPosition = animation_track_info.track_position;
		track[animation_track_info.track_index].m_fWeight = animation_track_info.weight;
	}
	controller->ApplyCurrentAnimationPose(this);
}
//º¸·ù

//==================================================================


Observer::Observer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, void* pContext)
	: CPlayer()
{
	type = EObjectType::MainPlayer;
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

shared_ptr<CCamera> Observer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
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
		m_pCamera->GenerateProjectionMatrix(CAMERA_NEAR, CAMERA_FAR, ASPECT_RATIO, 60.0f);
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
	if (m_pCamera && m_pCamera->GetMode() == FIRST_PERSON_CAMERA)
	{
		m_xmf3Look = Vector3::Normalize(m_pCamera->GetLookVector());
		m_xmf3Right = Vector3::Normalize(m_pCamera->GetRightVector());
		m_xmf3Up = Vector3::Normalize(m_pCamera->GetUpVector());
	}
	//float fixedYValue = -5.0f;
	//m_xmf3Position.y = fixedYValue;
	//if (m_pCamera)
	//{
	//	XMFLOAT3 camPos = m_pCamera->GetPosition();
	//	camPos.y = fixedYValue + 20.0f; 
	//	m_pCamera->SetPosition(camPos);
	//}
}


ServerSyncData Observer::MakeSyncData()
{
	ServerSyncData data = CGameObject::MakeSyncData();

	return data;
}

void Observer::ApplySyncData(const ServerSyncData& syncData)
{
	CGameObject::ApplySyncData(syncData);
	SetPosition(syncData.position);

	GetSkinnedAnimationController()->ApplyCurrentAnimationPose(this);
}