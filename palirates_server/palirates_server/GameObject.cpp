#include "stdafx.h"
#include "GameObject.h"
#include "ServerAnimLoader.h"

void GameObject::Set_Name(std::string_view name)
{
	Obj_Name = name;
}

void GameObject::Set_Child(std::shared_ptr<GameObject> pChild)
{
	if (pChild)
		pChild->m_pParent = shared_from_this();


	if (child_obj)
	{
		if (pChild)
			pChild->sibling_obj = child_obj->sibling_obj;

		child_obj->sibling_obj = pChild;
	}
	else
		child_obj = pChild;
}

std::shared_ptr<GameObject> GameObject::Get_Child()
{
	if (child_obj != nullptr)
		return child_obj;
	else
		return nullptr;
}

std::shared_ptr<GameObject> GameObject::Get_Sibling()
{
	if (sibling_obj != nullptr)
		return sibling_obj;
	else
		return nullptr;
}


std::shared_ptr<GameObject> GameObject::FindFrame(std::string_view name)
{
	if (Obj_Name.size() && Obj_Name == name)
		return shared_from_this();

	std::shared_ptr<GameObject> found;

	if (sibling_obj)
	{
		found = sibling_obj->FindFrame(name);
		if (found) return found;
	}

	if (child_obj)
	{
		found = child_obj->FindFrame(name);
		if (found) return found;
	}

	return nullptr;
}

void GameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Parent, *pxmf4x4Parent) : m_xmf4x4Parent;

	if (sibling_obj) sibling_obj->UpdateTransform(pxmf4x4Parent);
	if (child_obj) child_obj->UpdateTransform(&m_xmf4x4World);
}

void GameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4Parent._41 = x;
	m_xmf4x4Parent._42 = y;
	m_xmf4x4Parent._43 = z;

	UpdateTransform(NULL);
}


void GameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void GameObject::Move(XMFLOAT3 xmf3Offset)
{
	m_xmf4x4Parent._41 += xmf3Offset.x;
	m_xmf4x4Parent._42 += xmf3Offset.y;
	m_xmf4x4Parent._43 += xmf3Offset.z;

	UpdateTransform(NULL);
}


void GameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4Parent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Parent);

	UpdateTransform(NULL);
}

void GameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4Parent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Parent);

	UpdateTransform(NULL);
}

void GameObject::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4Parent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Parent);

	UpdateTransform(NULL);
}


XMFLOAT3 GameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 GameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 GameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 GameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void GameObject::SetLook(XMFLOAT3 xmf3Look)
{
	// 기본 방향 보정
	if (Vector3::Length(xmf3Look) < 1e-6f)
		xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f); // Z+ 고정

	XMFLOAT3 look = Vector3::Normalize(xmf3Look);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	// 예외 처리: look과 up이 거의 평행할 경우
	if (fabs(Vector3::DotProduct(look, up)) > 0.999f)
		up = XMFLOAT3(1.0f, 0.0f, 0.0f); // X+ 로 보정

	XMFLOAT3 right = Vector3::Normalize(Vector3::CrossProduct(up, look));
	up = Vector3::Normalize(Vector3::CrossProduct(look, right));

	m_xmf4x4Parent._11 = right.x;  m_xmf4x4Parent._12 = right.y;  m_xmf4x4Parent._13 = right.z;
	m_xmf4x4Parent._21 = up.x;     m_xmf4x4Parent._22 = up.y;     m_xmf4x4Parent._23 = up.z;
	m_xmf4x4Parent._31 = look.x;   m_xmf4x4Parent._32 = look.y;   m_xmf4x4Parent._33 = look.z;

	UpdateTransform(nullptr);
}

void GameObject::SetUp(XMFLOAT3 xmf3Up)
{
	if (Vector3::Length(xmf3Up) < 1e-6f)
		xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f); // Y+ 고정

	XMFLOAT3 up = Vector3::Normalize(xmf3Up);
	XMFLOAT3 look = GetLook(); // Z
	if (fabs(Vector3::DotProduct(up, look)) > 0.999f)
		look = XMFLOAT3(0.0f, 0.0f, 1.0f); // 보정

	XMFLOAT3 right = Vector3::Normalize(Vector3::CrossProduct(up, look));
	look = Vector3::Normalize(Vector3::CrossProduct(right, up));

	m_xmf4x4Parent._11 = right.x;  m_xmf4x4Parent._12 = right.y;  m_xmf4x4Parent._13 = right.z;
	m_xmf4x4Parent._21 = up.x;     m_xmf4x4Parent._22 = up.y;     m_xmf4x4Parent._23 = up.z;
	m_xmf4x4Parent._31 = look.x;   m_xmf4x4Parent._32 = look.y;   m_xmf4x4Parent._33 = look.z;

	UpdateTransform(nullptr);
}

void GameObject::SetRight(XMFLOAT3 xmf3Right)
{
	if (Vector3::Length(xmf3Right) < 1e-6f)
		xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f); // X+ 고정

	XMFLOAT3 right = Vector3::Normalize(xmf3Right);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	if (fabs(Vector3::DotProduct(right, up)) > 0.999f)
		up = XMFLOAT3(0.0f, 0.0f, 1.0f); // 보정

	XMFLOAT3 look = Vector3::Normalize(Vector3::CrossProduct(right, up));
	up = Vector3::Normalize(Vector3::CrossProduct(look, right));

	m_xmf4x4Parent._11 = right.x;  m_xmf4x4Parent._12 = right.y;  m_xmf4x4Parent._13 = right.z;
	m_xmf4x4Parent._21 = up.x;     m_xmf4x4Parent._22 = up.y;     m_xmf4x4Parent._23 = up.z;
	m_xmf4x4Parent._31 = look.x;   m_xmf4x4Parent._32 = look.y;   m_xmf4x4Parent._33 = look.z;

	UpdateTransform(nullptr);
}

void GameObject::SetMesh(std::shared_ptr<CStandardMesh> pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

static void SkipMaterialsBlock(FILE* fp)
{
	char tok[64]{};
	int  dummyInt = 0;
	float dummyFloat[4]{};

	while (true)
	{
		ReadStringFromFile(fp, tok);

		if (!strcmp(tok, "<Material>:"))
		{
			ReadIntegerFromFile(fp);                 // material index
		}
		else if (!strcmp(tok, "<AlbedoColor>:") ||
			!strcmp(tok, "<EmissiveColor>:") ||
			!strcmp(tok, "<SpecularColor>:"))
		{
			fread(dummyFloat, sizeof(float), 4, fp); // skip vec4
		}
		else if (!strcmp(tok, "<SpecularHighlight>:") ||
			!strcmp(tok, "<Smoothness>:") ||
			!strcmp(tok, "<Metallic>:") ||
			!strcmp(tok, "<Glossiness>:") ||
			!strcmp(tok, "<GlossyReflection>:"))
		{
			fread(dummyFloat, sizeof(float), 1, fp); // skip scalar
		}
		else if (!strcmp(tok, "<AlbedoMap>:") ||
			!strcmp(tok, "<SpecularMap>:") ||
			!strcmp(tok, "<NormalMap>:") ||
			!strcmp(tok, "<MetallicMap>:") ||
			!strcmp(tok, "<EmissionMap>:"))
		{
			char texPath[260];
			ReadStringFromFile(fp, texPath);   
		}
		else if (!strcmp(tok, "</Materials>"))
		{
			return;                          
		}
		else
		{
			char junk[260];
			ReadStringFromFile(fp, junk);
		}
	}
}

CLoadedModelInfo* GameObject::LoadGeometryAndAnimationFromFile(char* pstrFileName)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();

	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				std::shared_ptr<GameObject> ModelRootObject_raw_ptr = GameObject::LoadFrameHierarchyFromFile( NULL, pInFile, &pLoadedModel->m_nSkinnedMeshes);

				std::shared_ptr<GameObject> ModelRootObject_shared_ptr(ModelRootObject_raw_ptr);
				pLoadedModel->m_pModelRootObject = ModelRootObject_shared_ptr;

				::ReadStringFromFile(pInFile, pstrToken);
			}
			else if (!strcmp(pstrToken, "<Animation>:"))
			{
				GameObject::LoadAnimationFromFile(pInFile, pLoadedModel, pstrFileName);
				pLoadedModel->PrepareSkinning();
			}
			else if (!strcmp(pstrToken, "</Animation>:"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	return(pLoadedModel);
}

std::shared_ptr<GameObject> GameObject::LoadFrameHierarchyFromFile( std::shared_ptr<GameObject> pParent, FILE* pInFile, int* pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	std::shared_ptr<GameObject> pGameObject = std::make_shared<GameObject>();

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4Parent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			shared_ptr<CStandardMesh> pMesh = make_shared<CStandardMesh>();
			pMesh->LoadMeshFromFile(pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			shared_ptr<CSkinnedMesh> pSkinnedMesh = make_shared<CSkinnedMesh>();
			pSkinnedMesh->LoadSkinInfoFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken);
			if (!strcmp(pstrToken, "<Mesh>:")) {
				pSkinnedMesh->LoadMeshFromFile(pInFile);
			}

			pGameObject->SetSkinnedMesh(pSkinnedMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			SkipMaterialsBlock(pInFile);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					std::shared_ptr<GameObject> pChild_raw_ptr = GameObject::LoadFrameHierarchyFromFile(pGameObject, pInFile, pnSkinnedMeshes);

					std::shared_ptr<GameObject> pChild(pChild_raw_ptr);
					if (pChild)
						pGameObject->Set_Child(pChild);
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

void GameObject::LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel, char* pstrFileName)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nAnimationSets = 0;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nAnimationSets = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets = new CAnimationSets(nAnimationSets);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			pLoadedModel->m_pAnimationSets->m_nBoneFrames = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches.resize(pLoadedModel->m_pAnimationSets->m_nBoneFrames, nullptr);

			for (int j = 0; j < pLoadedModel->m_pAnimationSets->m_nBoneFrames; j++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				shared_ptr<GameObject> frame_ptr = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);
				pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches[j] = frame_ptr.get();
			}
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken);

			float fLength = ::ReadFloatFromFile(pInFile);
			int nFramesPerSecond = ::ReadIntegerFromFile(pInFile);
			int nKeyFrames = ::ReadIntegerFromFile(pInFile);

			auto animSet = std::make_shared<CAnimationSet>(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames, pstrToken);
			std::string filename_key(pstrFileName);
			auto sharedAnimSet = CAnimationSets::AddOrGetSharedAnimationSet(animSet, filename_key);

			pLoadedModel->m_pAnimationSets->m_pAnimationSet_list[nAnimationSet] = sharedAnimSet;

			bool bIsNew = (sharedAnimSet == animSet);

			if (bIsNew)
			{
				for (int i = 0; i < nKeyFrames; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken); 
					if (!strcmp(pstrToken, "<Transforms>:"))
					{
						CAnimationSet* pAnimationSet = sharedAnimSet.get();

						int nKey = ::ReadIntegerFromFile(pInFile);
						float fKeyTime = ::ReadFloatFromFile(pInFile);

						pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
						nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i],
							sizeof(XMFLOAT4X4),
							pLoadedModel->m_pAnimationSets->m_nBoneFrames,
							pInFile);
					}
				}
			}
			else
			{
				for (int i = 0; i < nKeyFrames; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken); 
					int nKey = ::ReadIntegerFromFile(pInFile); // i
					float fKeyTime = ::ReadFloatFromFile(pInFile); // skip
					fseek(pInFile, sizeof(XMFLOAT4X4) * pLoadedModel->m_pAnimationSets->m_nBoneFrames, SEEK_CUR); // skip fread
				}
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}

void GameObject::FindAndSetSkinnedMesh(std::vector<std::shared_ptr<CSkinnedMesh>>& outSkinnedMeshes)
{
	if (m_pMesh) {
		auto pSkinned = std::dynamic_pointer_cast<CSkinnedMesh>(m_pMesh);
		if (pSkinned) {
			outSkinnedMeshes.push_back(pSkinned);
		}
	}

	if (sibling_obj)
		sibling_obj->FindAndSetSkinnedMesh(outSkinnedMeshes);

	if (child_obj)
		child_obj->FindAndSetSkinnedMesh(outSkinnedMeshes);
}

void GameObject::SetSkinnedMesh(std::shared_ptr<CSkinnedMesh> pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

//===================================================================

Boat_Object::Boat_Object()
	: GameObject()
{
	m_fMaxVelocityXZ = 200.0f;
	m_xmf3Velocity = XMFLOAT3(300.0f, 0.0f, 0.0f);
	m_fFriction = 500.0f;
	SetLook(XMFLOAT3(0.0f, 0.0f, 1.0f));
}

Boat_Object::~Boat_Object() {}

void Boat_Object::MoveForward(float speed)
{
	XMFLOAT3 look = Vector3::Normalize(GetLook());
	XMFLOAT3 shift = Vector3::ScalarProduct(look, speed, false);
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, shift);
}

void Boat_Object::Add_Rotate(float angleDelta)
{
	m_fRotationSpeed += angleDelta;
	m_fRotationSpeed = std::clamp<float>(m_fRotationSpeed, -45.0f, 45.0f);
}

void Boat_Object::Animate(float fTimeElapsed)
{
	// --- 속도 크기 측정 ---
	float velocityFull = Vector3::Length(m_xmf3Velocity);

	// --- 최대 속도 제한 ---
	if (velocityFull > m_fMaxVelocityXZ)
	{
		float scale = m_fMaxVelocityXZ / velocityFull;
		m_xmf3Velocity.x *= scale;
		m_xmf3Velocity.z *= scale;
		velocityFull = m_fMaxVelocityXZ;
	}

	// --- 이동 처리 ---
	XMFLOAT3 lookDir = Vector3::Normalize(GetLook());
	XMFLOAT3 velocityXZ = Vector3::ScalarProduct(lookDir, velocityFull, false);

	XMFLOAT3 pos = GetPosition();
	XMFLOAT3 deltaMove = Vector3::ScalarProduct(velocityXZ, fTimeElapsed, false);
	SetPosition(Vector3::Add(pos, deltaMove));

	// --- 회전 처리 ---
	XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
	Rotate(&up, m_fRotationSpeed * fTimeElapsed);
	//m_fRotationSpeed = std::lerp(m_fRotationSpeed, 0.0f, 0.01f);
	m_fRotationSpeed = lerp(m_fRotationSpeed, 0.0f, 0.01f);

	// --- 감속 처리 ---
	float decel = m_fFriction * fTimeElapsed;
	if (decel > velocityFull) decel = velocityFull;
	velocityFull -= decel;

	// --- 감속 적용: Look 방향 기준으로 감속된 속도 재계산 ---
	XMFLOAT3 newVelocity = Vector3::ScalarProduct(lookDir, velocityFull, false);
	m_xmf3Velocity = XMFLOAT3(newVelocity.x, 0.0f, newVelocity.z);
}
void Boat_Object::HandleBoundaryReflection(float boundary)
{
	XMFLOAT3 pos = GetPosition();
	XMFLOAT3 vel = Get_Velocity();
	bool bounced = false;

	if (pos.x > boundary || pos.x < -boundary) {
		vel.x *= -1.0f; bounced = true;
	}
	if (pos.z > boundary || pos.z < -boundary) {
		vel.z *= -1.0f; bounced = true;
	}

	if (bounced) {
		Set_Velocity(vel);
		XMFLOAT3 newLook = Vector3::Normalize(vel);
		SetLook(newLook);
	}
}
