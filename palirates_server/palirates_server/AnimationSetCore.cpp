#include "stdafx.h"
#include "AnimationSetCore.h"
#include "AnimationRegistry.h"
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
		bool bRootMotion = false;
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
				
					/*if (pRootGameObject->GetType() == Object_Type::monster)*/ {
						if (j == RootIndex) {
							if (!m_pAnimationTracks[k].m_bFinished && pRootGameObject->RootMotionTrackSet.find(k) != pRootGameObject->RootMotionTrackSet.end()) {
								HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);
								/*if (pRootGameObject->GetType() == Object_Type::player) {
									cout << HipsPosition.x << ", " << HipsPosition.y << ', ' << HipsPosition.z << "\n";
								}*/
								bRootMotion = true;
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

		if (bRootMotion)
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
	Skinned_GameObject* monster = dynamic_cast<Skinned_GameObject*>(pRootGameObject);
	if (!monster) return;
	const float multiplier = pRootGameObject->m_fScale * 1;
	XMFLOAT3 deltaMove;
	float currWeight = m_pAnimationTracks[monster->currStateTrackIdx].m_fWeight;
	if (bTrackLooped || currWeight < 0.3f) {
		//cout << "bTrackLooped : " << bTrackLooped << ", currWeight : " << currWeight << "\n";
		deltaMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	else
		deltaMove = Vector3::Subtract(HipsPosition, m_xmf3PrevHipsPosition);
	deltaMove = XMFLOAT3(deltaMove.x * multiplier, deltaMove.y * multiplier, deltaMove.z * multiplier);
	/*if (pRootGameObject->GetType() == Object_Type::player) {
		cout << "Player deltaMove : " << deltaMove.x << ", " << deltaMove.y << ", " << deltaMove.z << "\n";
	}*/
	if ((deltaMove.x == 0.0f && deltaMove.y == 0.0f && deltaMove.z == 0.0f) /*|| Vector3::LengthSquared(deltaMove) > 1.0f*/) {
		m_xmf3PrevHipsPosition = HipsPosition;
		return;
	}
	if (Vector3::LengthSquared(deltaMove) > 0.000001f) 
	{
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

							if (XMVectorGetX(XMVector3LengthSq(slideVec)) > 0.0001f) {
								pos += slideVec;
								XMFLOAT3 deltaVec_fulld;
								XMStoreFloat3(&deltaVec_fulld, deltaVec_full);
								//std::cout << "deltaMove = (" << deltaMove.x << ", " << deltaMove.y << ", " << deltaMove.z << ")\n";
								//std::cout << "deltaVec_full = (" << deltaVec_fulld.x << ", " << deltaVec_fulld.y << ", " << deltaVec_fulld.z << ")\n";
								XMFLOAT3 slideVecd;
								XMStoreFloat3(&slideVecd, slideVec);
								//std::cout << "slideVec = (" << slideVecd.x << ", " << slideVecd.y << ", " << slideVecd.z << ")\n";
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
			if (monster->currStateTrackIdx == TRACK_DIVEROLL_FORWARD)
				cout << "deltaMove Pos : " << deltaMove.x << ", " << deltaMove.y << ", " << deltaMove.z << "\n";
			XMFLOAT3 newPos;
			XMStoreFloat3(&newPos, pos);
			pRootGameObject->SetPosition(newPos);
			if (pRootGameObject->GetType() == Object_Type::player) {
				//cout << monster->currStateTrackIdx << "\n";
				//cout << "Player newPos : " << newPos.x << ", " << newPos.y << ", " << newPos.z << "\n";
			}
		}
	}

	m_xmf3PrevHipsPosition = HipsPosition; 
}

void CAnimationController::AdvanceTime2(float fTimeElapsed, GameObject* pRootGameObject, const std::vector<BoundingOrientedBox>* obblist)
{
	m_fTime += fTimeElapsed;
	if (!m_pAnimationTracks) return;

	for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
		m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = Matrix4x4::Zero();

	float totalWeight = 0.0f;
	std::vector<int> activeTracks;
	activeTracks.reserve(m_nAnimationTracks);
	for (int k = 0; k < m_nAnimationTracks; ++k) {
		if (m_pAnimationTracks[k].m_fWeight > ANIMATION_CALLBACK_EPSILON) {
			totalWeight += m_pAnimationTracks[k].m_fWeight;
			activeTracks.push_back(k);
		}
	}
	if (totalWeight == 0.0f) return;

	XMFLOAT3 weightedPrevHips = { 0,0,0 };
	XMFLOAT3 weightedCurrHips = { 0,0,0 };
	float    rootWsum = 0.0f;
	bool     anyLooped = false;

	struct TrackStep { float prevPos; float currPos; bool looped; };
	std::vector<TrackStep> steps(m_nAnimationTracks, { 0,0,false });

	for (int k : activeTracks) {
		CAnimationSet* set = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet].get();

		float prevPos = m_pAnimationTracks[k].m_fPosition;
		float currPos = m_pAnimationTracks[k].UpdatePosition(prevPos, fTimeElapsed, set->m_fLength);
		bool  looped = (prevPos > currPos); 

		steps[k] = { prevPos, currPos, looped };

		if (!m_pAnimationTracks[k].m_bFinished &&
			pRootGameObject->RootMotionTrackSet.find(k) != pRootGameObject->RootMotionTrackSet.end())
		{
			if (!looped) {
				XMFLOAT4X4 prevRoot = set->GetSRT(RootIndex, prevPos);
				XMFLOAT4X4 currRoot = set->GetSRT(RootIndex, currPos);

				XMFLOAT3 prevH = { prevRoot._41, prevRoot._42, prevRoot._43 };
				XMFLOAT3 currH = { currRoot._41, currRoot._42, currRoot._43 };

				float w = m_pAnimationTracks[k].m_fWeight;
				weightedPrevHips.x += prevH.x * w;  weightedPrevHips.y += prevH.y * w;  weightedPrevHips.z += prevH.z * w;
				weightedCurrHips.x += currH.x * w;  weightedCurrHips.y += currH.y * w;  weightedCurrHips.z += currH.z * w;
				rootWsum += w;
			}
			else {
				anyLooped = true;
			}
		}
	}

	XMFLOAT3 hipsPrev = { 0,0,0 };
	XMFLOAT3 hipsCurr = { 0,0,0 };
	bool bRootMotion = (rootWsum > 0.0f);

	if (bRootMotion && !anyLooped) {
		float inv = 1.0f / rootWsum;
		hipsPrev = { weightedPrevHips.x * inv, weightedPrevHips.y * inv, weightedPrevHips.z * inv };
		hipsCurr = { weightedCurrHips.x * inv, weightedCurrHips.y * inv, weightedCurrHips.z * inv };
	}
	else {
		hipsPrev = m_xmf3PrevHipsPosition;
		hipsCurr = m_xmf3PrevHipsPosition;
		bRootMotion = false;
	}

	for (int k : activeTracks) {
		CAnimationSet* set = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet].get();
		float fPos = steps[k].currPos; 
		float normW = m_pAnimationTracks[k].m_fWeight / totalWeight;

		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; ++j) {
			XMFLOAT4X4 accum = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent;
			XMFLOAT4X4 srt = set->GetSRT(j, fPos);

			if (j == RootIndex) {
				srt._41 = 0.0f;
				srt._42 = 0.0f;
				srt._43 = 0.0f;
			}

			XMFLOAT4X4 blended = Matrix4x4::Add(accum, Matrix4x4::Scale(srt, normW));
			m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = blended;
		}
	}

	if (bRootMotion) {
		XMFLOAT3 deltaLocal = { hipsCurr.x - hipsPrev.x, hipsCurr.y - hipsPrev.y, hipsCurr.z - hipsPrev.z };
		{
			float s = pRootGameObject->m_fScale; 
			deltaLocal.x *= s;
			deltaLocal.y *= s;
			deltaLocal.z *= s;
		}
		XMFLOAT3 look = pRootGameObject->GetLook();
		float yaw = XMConvertToDegrees(atan2f(look.x, look.z));
		XMMATRIX rotY = XMMatrixRotationY(XMConvertToRadians(yaw));

		XMVECTOR d = XMLoadFloat3(&deltaLocal);
		d = XMVector3TransformCoord(d, rotY);

		XMVECTOR dFlat = XMVectorSet(XMVectorGetX(d), 0.0f, XMVectorGetZ(d), 0.0f);

		XMVECTOR pos = XMLoadFloat3(&pRootGameObject->GetPosition());
		bool blocked = false;

		if (obblist) {
			constexpr float rayLength = 4.0f;
			constexpr float offsetY = 4.0f;
			constexpr float lateral = 4.0f;
			XMVECTOR dir = XMVector3Normalize(dFlat);

			std::array<XMVECTOR, 3> rays = {
				pos + XMVectorSet(0.0f, offsetY, 0.0f, 0.0f),
				pos + XMVectorSet(-lateral, offsetY, 0.0f, 0.0f),
				pos + XMVectorSet(lateral, offsetY, 0.0f, 0.0f)
			};

			for (auto& ro : rays) {
				for (const auto& obb : *obblist) {
					float dist = 0.0f;
					if (obb.Intersects(ro, dir, dist) && dist <= rayLength) {
						blocked = true;

						XMVECTOR axisX = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMLoadFloat4(&obb.Orientation));
						XMVECTOR axisY = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), XMLoadFloat4(&obb.Orientation));
						XMVECTOR axisZ = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&obb.Orientation));

						XMVECTOR toRay = XMVector3Normalize(ro - XMLoadFloat3(&obb.Center));
						float dotX = fabsf(XMVectorGetX(XMVector3Dot(toRay, axisX)));
						float dotY = fabsf(XMVectorGetX(XMVector3Dot(toRay, axisY)));
						float dotZ = fabsf(XMVectorGetX(XMVector3Dot(toRay, axisZ)));
						XMVECTOR n = (dotX > dotY && dotX > dotZ) ? axisX : (dotY > dotZ ? axisY : axisZ);
						n = XMVector3Normalize(XMVectorSet(XMVectorGetX(n), 0.0f, XMVectorGetZ(n), 0.0f));

						XMVECTOR slide = dFlat - XMVector3Dot(dFlat, n) * n;
						if (XMVectorGetX(XMVector3LengthSq(slide)) > 0.0001f)
							pos += slide;

						d = XMVectorZero(); 
						break;
					}
				}
				if (blocked) break;
			}
		}

		if (!blocked) pos += d;
		XMFLOAT3 newPos; XMStoreFloat3(&newPos, pos);
		newPos.y = 0.0f;
		pRootGameObject->SetPosition(newPos);
		//cout << newPos.x << ", " << newPos.y << ", " << newPos.z << "\n";
	}

	m_xmf3PrevHipsPosition = hipsCurr;

	pRootGameObject->UpdateTransform(nullptr);
	OnAnimationIK(pRootGameObject);
}