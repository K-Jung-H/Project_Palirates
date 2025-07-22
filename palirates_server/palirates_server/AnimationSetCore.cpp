#include "stdafx.h"
#include "AnimationSetCore.h"
#include "ServerAnimLoader.h"
#include "GameObject.h"
#include "Monster.h"

using std::shared_ptr;
using std::string;

std::unordered_map<std::string, std::shared_ptr<CAnimationSet>> CAnimationSets::s_GlobalAnimationSetCache;

CAnimationSet::CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrames, int nAnimatedBones, char* pstrName)
{
	m_fLength = fLength;
	m_nFramesPerSecond = nFramesPerSecond;
	m_nKeyFrames = nKeyFrames;

	strcpy_s(m_pstrAnimationSetName, 64, pstrName);

	m_pfKeyFrameTimes = new float[nKeyFrames];
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4 * [nKeyFrames];
	for (int i = 0; i < nKeyFrames; i++) m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nAnimatedBones];
}

CAnimationSet::~CAnimationSet()
{
	if (m_pfKeyFrameTimes)
		delete[] m_pfKeyFrameTimes;

	for (int j = 0; j < m_nKeyFrames; j++)
		if (m_ppxmf4x4KeyFrameTransforms[j])
			delete[] m_ppxmf4x4KeyFrameTransforms[j];

	if (m_ppxmf4x4KeyFrameTransforms)
		delete[] m_ppxmf4x4KeyFrameTransforms;

	//DebugOutput("\nDelete AnimationSet: ", m_pstrAnimationSetName);
}

XMFLOAT4X4 CAnimationSet::GetSRT(int nBone, float fPosition)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();

	if (fPosition >= m_pfKeyFrameTimes[m_nKeyFrames - 1])
		return m_ppxmf4x4KeyFrameTransforms[m_nKeyFrames - 1][nBone];

	for (int i = 0; i < (m_nKeyFrames - 1); i++)
	{
		if ((m_pfKeyFrameTimes[i] <= fPosition) && (fPosition < m_pfKeyFrameTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameTimes[i]) / (m_pfKeyFrameTimes[i + 1] - m_pfKeyFrameTimes[i]);
			xmf4x4Transform = Matrix4x4::Interpolate(m_ppxmf4x4KeyFrameTransforms[i][nBone], m_ppxmf4x4KeyFrameTransforms[i + 1][nBone], t);
			break;
		}
	}

	return(xmf4x4Transform);
}

std::unordered_map<std::string, shared_ptr<CAnimationSet>> CAnimationSets::s_globalCache;

CAnimationSets::CAnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	m_pAnimationSet_list.resize(nAnimationSets);
}

CAnimationSets::~CAnimationSets()
{
	m_pAnimationSet_list.clear();
}

std::shared_ptr<CAnimationSet> CAnimationSets::AddOrGetSharedAnimationSet(std::shared_ptr<CAnimationSet> animSet, const std::string& fileName)
{
	std::string key = fileName + "::" + animSet->m_pstrAnimationSetName;

	auto it = s_GlobalAnimationSetCache.find(key);
	if (it != s_GlobalAnimationSetCache.end())
		return it->second;

	s_GlobalAnimationSetCache[key] = animSet;
	return animSet;
}

void CAnimationSets::Bone_Info()
{
	for (int i = 0; i < m_nBoneFrames; ++i)
	{
		if (strcmp(m_ppBoneFrameCaches[i]->m_pstrFrameName, "Mesh") == 0)
			continue;

		TCHAR tFrameName[64];
		MultiByteToWideChar(CP_ACP, 0, m_ppBoneFrameCaches[i]->m_pstrFrameName, -1, tFrameName, 64);


		TCHAR pstrDebug[256] = { 0 };

		_stprintf_s(pstrDebug, 256, _T("----------------------------------------------- [Bone: %s]\n"), tFrameName);
		OutputDebugString(pstrDebug);

		m_ppBoneFrameCaches[i]->Obj_Info();

		_stprintf_s(pstrDebug, 2, _T("\n"));
		OutputDebugString(pstrDebug);
	}
}

CAnimationTrack::CAnimationTrack()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.0f, 3.0f);

	m_fPosition = dist(gen);
}

CAnimationTrack::~CAnimationTrack()
{
}

float CAnimationTrack::UpdatePosition(float fTrackPosition, float fElapsedTime, float fAnimationLength)
{
	float fTrackElapsedTime = fElapsedTime * m_fSpeed;
	switch (m_nType)
	{
	case ANIMATION_TYPE_LOOP:
	{
		/*if (m_fPosition < 0.0f)
			m_fPosition = 0.0f;
		else
		{
			m_fPosition = fTrackPosition + fTrackElapsedTime;
			if (m_fPosition > fAnimationLength)
			{
				m_fPosition = 0;
				return(fAnimationLength);
			}
		}*/
		float newPosition = m_fPosition + fTrackElapsedTime;
		m_fPosition = fmod(newPosition, fAnimationLength);
		break;
	}
	case ANIMATION_TYPE_ONCE:
		if (m_fPosition < 0.0f)
			m_fPosition = 0.0f;
		else {
			if (!m_bFinished) {
				m_fPosition = fTrackPosition + fTrackElapsedTime;
				if (m_fPosition > fAnimationLength) {
					m_fPosition = fAnimationLength;
					m_bFinished = true;
					return(fAnimationLength);
				}
			}
		}
		break;
	case ANIMATION_TYPE_PINGPONG:
		break;
	}

	return(m_fPosition);
}

CAnimationController::CAnimationController(int nAnimationTracks, CLoadedModelInfo* pModel)
{
	m_nAnimationTracks = nAnimationTracks;
	m_pAnimationTracks = new CAnimationTrack[nAnimationTracks];

	m_pAnimationSets = pModel->m_pAnimationSets;
	m_pAnimationSets->AddRef();

	m_pModelRootObject = pModel->m_pModelRootObject;

	m_nSkinnedMeshes = pModel->m_nSkinnedMeshes;
	m_ppSkinnedMeshes.resize(m_nSkinnedMeshes);

	for (int i = 0; i < m_nSkinnedMeshes; i++)
		m_ppSkinnedMeshes[i] = pModel->m_ppSkinnedMeshes[i];

	m_ppcbxmf4x4MappedSkinningBoneTransforms = new XMFLOAT4X4 * [m_nSkinnedMeshes]();
}

CAnimationController::~CAnimationController()
{
	if (m_pAnimationTracks)
		delete[] m_pAnimationTracks;

	if (m_ppcbxmf4x4MappedSkinningBoneTransforms)
		delete[] m_ppcbxmf4x4MappedSkinningBoneTransforms;

	if (m_pAnimationSets)
		m_pAnimationSets->Release();

	m_ppSkinnedMeshes.clear();
}

void CAnimationController::Bone_Info()
{
	m_pAnimationSets->Bone_Info();

}

void CAnimationController::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pAnimationTracks)
		m_pAnimationTracks[nAnimationTrack].m_nAnimationSet = nAnimationSet;
}

void CAnimationController::SetTrackEnable(int nAnimationTrack, bool bEnable)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetEnable(bEnable);
}

void CAnimationController::SetTrackPosition(int nAnimationTrack, float fPosition)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetPosition(fPosition);
}

void CAnimationController::SetTrackSpeed(int nAnimationTrack, float fSpeed)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
}

void CAnimationController::SetTrackWeight(int nAnimationTrack, float fWeight)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetWeight(fWeight);
}

void CAnimationController::AdvanceTime(float fTimeElapsed, GameObject* pRootGameObject, const std::vector<BoundingOrientedBox>* obblist)
{
	m_fTime += fTimeElapsed;

	if (m_pAnimationTracks)
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
		{
			m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = Matrix4x4::Zero();
		}

		float totalWeight = 0.0f;
		int dominantTrackIndex = -1;
		float maxWeight = -1.0f;
		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_fWeight > ANIMATION_CALLBACK_EPSILON)
			{
				totalWeight += m_pAnimationTracks[k].m_fWeight;
				if (m_pAnimationTracks[k].m_fWeight > maxWeight) {
					maxWeight = m_pAnimationTracks[k].m_fWeight;
					dominantTrackIndex = k;
				}
			}
		}
		if (totalWeight == 0.0f) return;
	
		float fPrevPos = 0.0f;
		bool bTrackLooped = false;
		if (maxWeight <= 0.8f) {
			dominantTrackIndex = -1;
			bTrackLooped = true;
		}
		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_fWeight > ANIMATION_CALLBACK_EPSILON)
			{
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet].get();
				if (dominantTrackIndex == k) {
					fPrevPos = m_pAnimationTracks[k].m_fPosition;
				}
				float fPosition = m_pAnimationTracks[k].UpdatePosition(m_pAnimationTracks[k].m_fPosition, fTimeElapsed, pAnimationSet->m_fLength);
				if (dominantTrackIndex == k) {
					bTrackLooped = fPrevPos > fPosition;
				}
				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent;
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);

					float normalizedWeight = m_pAnimationTracks[k].m_fWeight / totalWeight;
					XMFLOAT4X4 blendedTransform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, normalizedWeight));

					if (pRootGameObject->GetType() == Object_Type::monster) {
						if (j == RootIndex) {
							if (!m_pAnimationTracks[k].m_bFinished && pRootGameObject->RootMotionTrackSet.find(k) != pRootGameObject->RootMotionTrackSet.end()) {
								HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);
							}

							blendedTransform._41 = 0.0f;
							//blendedTransform._42 = 0.0f;
							blendedTransform._43 = 0.0f;
						}

					}
					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = blendedTransform;
				}

			}
			if (m_pAnimationTracks[k].m_fWeight >= 1.0f)
				break;
		}

		pRootGameObject->UpdateTransform(NULL);

		OnRootMotion(pRootGameObject, bTrackLooped, obblist);
		OnAnimationIK(pRootGameObject);
	}
}

std::vector<Animation_Sync> CAnimationController::MakeSyncData()
{
	std::vector<Animation_Sync> data;
	for (int i = 0; i < m_nAnimationTracks; ++i) {
		if (m_pAnimationTracks[i].m_fWeight > ANIMATION_CALLBACK_EPSILON) {
			Animation_Sync t;
			t.track_index = i;
			t.track_position = m_pAnimationTracks[i].m_fPosition;
			t.weight = m_pAnimationTracks[i].m_fWeight;
			data.push_back(t);
		}
	}
	if (data.size() >= 2) {
		
	}
	return data;
}

void CAnimationController::ResetWeight()
{
	for (int i = 0; i < m_nAnimationTracks; ++i) {
		m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
}

void CAnimationController::OnRootMotion(GameObject* pRootGameObject, bool bTrackLooped, const std::vector<BoundingOrientedBox>* obblist)
{
	if (pRootGameObject->GetType() != Object_Type::monster) return;
	const float multiplier = pRootGameObject->m_fScale * 1;
	XMFLOAT3 deltaMove;
	if (bTrackLooped)
		deltaMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	else
		deltaMove = Vector3::Subtract(HipsPosition, m_xmf3PrevHipsPosition);
	deltaMove = XMFLOAT3(deltaMove.x * multiplier, deltaMove.y * multiplier, deltaMove.z * multiplier);
	if (deltaMove.x == 0.0f && deltaMove.y == 0.0f && deltaMove.z == 0.0f) {
		m_xmf3PrevHipsPosition = HipsPosition;
		return;
	}
	if (Vector3::LengthSquared(deltaMove) > 0.000001f) 
	{
		float length = Vector3::Length(deltaMove);
		if (length >= 1.0f) {
			std::cout << "[RootMotion] deltaMove: (" << deltaMove.x << ", " << deltaMove.y << ", " << deltaMove.z
				<< "), Length: " << length << " - " << HipsPosition.z << ", " << m_xmf3PrevHipsPosition.z << std::endl;
		}
		XMFLOAT3 look = pRootGameObject->GetLook();
		float yaw = XMConvertToDegrees(atan2f(look.x, look.z));
		XMMATRIX rot = XMMatrixRotationY(XMConvertToRadians(yaw));

		XMVECTOR deltaVec_full = XMLoadFloat3(&deltaMove);
		deltaVec_full = XMVector3TransformCoord(deltaVec_full, rot);

		XMVECTOR deltaVec_flat = XMVectorSet(
			XMVectorGetX(deltaVec_full),
			0.0f,
			XMVectorGetZ(deltaVec_full),
			0.0f
		);
		constexpr float rayLength = 4.0f;
		constexpr float offsetX = 0.0f;
		constexpr float offsetY = 4.0f;
		constexpr float offsetZ = 0.0f;

	
		XMVECTOR worldOffset = XMVectorSet(offsetX, offsetY, offsetZ, 0.0f);
		XMVECTOR pos = XMLoadFloat3(&pRootGameObject->GetPosition());
		XMVECTOR rayDir = XMVector3Normalize(deltaVec_flat);
		//XMVECTOR end = pos + rayDir * rayLength;

		constexpr float lateralOffset = 4.0f;
		std::vector<XMVECTOR> rayOrigins = {
			pos + XMVectorSet(0.0f, offsetY, 0.0f, 0.0f),
			pos + XMVectorSet(-lateralOffset, offsetY, 0.0f, 0.0f),
			pos + XMVectorSet(lateralOffset, offsetY, 0.0f, 0.0f)
		};

		bool blocked = false;
		
		if (obblist) 
		{
			for (const XMVECTOR& rayOrigin : rayOrigins) {
				for (const auto& obb : *obblist)
				{
					float distance = 0.0f;
					if (obb.Intersects(rayOrigin, rayDir, distance)) {
						if (distance <= rayLength) {
							blocked = true;

							XMVECTOR obbCenter = XMLoadFloat3(&obb.Center);
							XMVECTOR toRay = XMVector3Normalize(rayOrigin - obbCenter);

							XMVECTOR axisX = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMLoadFloat4(&obb.Orientation));
							XMVECTOR axisY = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), XMLoadFloat4(&obb.Orientation));
							XMVECTOR axisZ = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&obb.Orientation));

							float dotX = fabsf(XMVectorGetX(XMVector3Dot(toRay, axisX)));
							float dotY = fabsf(XMVectorGetX(XMVector3Dot(toRay, axisY)));
							float dotZ = fabsf(XMVectorGetX(XMVector3Dot(toRay, axisZ)));

							XMVECTOR normal;
							if (dotX > dotY && dotX > dotZ)
								normal = axisX;
							else if (dotY > dotZ)
								normal = axisY;
							else
								normal = axisZ;

							normal = XMVectorSet(XMVectorGetX(normal), 0.0f, XMVectorGetZ(normal), 0.0f);
							normal = XMVector3Normalize(normal);

							XMVECTOR slideVec = deltaVec_flat - XMVector3Dot(deltaVec_flat, normal) * normal;
							std::cout << "Ãæµ¹" << ")\n";
							if (XMVectorGetX(XMVector3LengthSq(slideVec)) > 0.0001f) {
								pos += slideVec;
								XMFLOAT3 deltaVec_fulld;
								XMStoreFloat3(&deltaVec_fulld, deltaVec_full);
								std::cout << "deltaMove = (" << deltaMove.x << ", " << deltaMove.y << ", " << deltaMove.z << ")\n";
								std::cout << "deltaVec_full = (" << deltaVec_fulld.x << ", " << deltaVec_fulld.y << ", " << deltaVec_fulld.z << ")\n";
								XMFLOAT3 slideVecd;
								XMStoreFloat3(&slideVecd, slideVec);
								std::cout << "slideVec = (" << slideVecd.x << ", " << slideVecd.y << ", " << slideVecd.z << ")\n";
								XMFLOAT3 newPos;
								XMStoreFloat3(&newPos, pos);
								pRootGameObject->SetPosition(newPos);
							}
							deltaVec_full = XMVectorZero();
							break;
						}
					}
				}
			}
		}

		if (!blocked) {
			pos += deltaVec_full;
			XMFLOAT3 newPos;
			XMStoreFloat3(&newPos, pos);

			pRootGameObject->SetPosition(newPos);
		}
	}

	m_xmf3PrevHipsPosition = HipsPosition; 
}