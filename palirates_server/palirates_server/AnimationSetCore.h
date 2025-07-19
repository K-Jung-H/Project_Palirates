#pragma once
#include "stdafx.h"
using namespace DirectX;

class GameObject;
class CLoadedModelInfo;
class CSkinnedMesh;

struct Animation_Sync
{
    int track_index;
    float weight;
    float track_position;
};

struct ServerSyncData
{
    XMFLOAT3 position;
    XMFLOAT3 lookVector;
    std::vector<Animation_Sync> track_info_list;
    bool stateChanged = false;
};

class CAnimationSet
{
public:
    CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char* pstrName);
    ~CAnimationSet();

    char                     m_pstrAnimationSetName[64];
    float         m_fLength = 0.f;   // seconds
    int           m_nFramesPerSecond = 0;
    int           m_nKeyFrames = 0;
    float* m_pfKeyFrameTimes = NULL;
    XMFLOAT4X4** m_ppxmf4x4KeyFrameTransforms = NULL;

    XMFLOAT4X4 GetSRT(int nBone, float fPosition);
};

class CAnimationSets
{
public:
    CAnimationSets(int nAnimationSets);
    ~CAnimationSets();

    int                        m_nAnimationSets = 0;
    std::vector<std::shared_ptr<CAnimationSet>> m_pAnimationSet_list;
    int                                         m_nBoneFrames = 0;
    // 클라에선 게임 오브젝트인데, 이 캐시가 가져야하는건 이름이랑 부모 행렬 뿐임
    // + 자식 본, 형제 본, 그 모든 정보를 찾아주는 Obj_Info 함수 정도
    std::vector<GameObject*>                          m_ppBoneFrameCaches;

private:
    static std::unordered_map<std::string, std::shared_ptr<CAnimationSet>> s_GlobalAnimationSetCache;
public: 
    static std::shared_ptr<CAnimationSet> AddOrGetSharedAnimationSet(std::shared_ptr<CAnimationSet> animSet, const std::string& fileName);
private:
    static std::unordered_map<std::string,
        std::shared_ptr<CAnimationSet>> s_globalCache;

private:
    int                        m_nReferences = 0;

public:
    void AddRef() { m_nReferences++; }
    void Release() { if (--m_nReferences <= 0) delete this; }
    void Bone_Info();
};

class CAnimationTrack
{
public:
    CAnimationTrack();
    ~CAnimationTrack();

public:
    BOOL                      m_bEnable = true;
    float                      m_fSpeed = 1.0f;
    float                      m_fPosition = -ANIMATION_CALLBACK_EPSILON;
    float                      m_fWeight = 1.0f;
    bool m_bFinished{ false };

    int                      m_nAnimationSet = 0; //AnimationSet Index

    int                      m_nType = ANIMATION_TYPE_LOOP; //Once, Loop, PingPong

public:
    void SetAnimationSet(int nAnimationSet) { m_nAnimationSet = nAnimationSet; }

    void SetEnable(bool bEnable) { m_bEnable = bEnable; }
    void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
    void SetWeight(float fWeight) { m_fWeight = fWeight; }

    void SetPosition(float fPosition) { m_fPosition = fPosition; }
    float UpdatePosition(float fTrackPosition, float fTrackElapsedTime, float fAnimationLength);
};

class CAnimationController
{
public:
    CAnimationController(int nAnimationTracks, CLoadedModelInfo* pModel);
    ~CAnimationController();

public:
    float                      m_fTime = 0.0f;

    int                      m_nAnimationTracks = 0;
    CAnimationTrack* m_pAnimationTracks = NULL;

    CAnimationSets* m_pAnimationSets = NULL;

    int                      m_nSkinnedMeshes = 0;
    std::vector<std::shared_ptr<CSkinnedMesh>> m_ppSkinnedMeshes; //[SkinnedMeshes], Skinned Mesh Cache

    XMFLOAT4X4** m_ppcbxmf4x4MappedSkinningBoneTransforms = NULL; //[SkinnedMeshes]

    int RootIndex{ 0 };

public:
    void Bone_Info();

    void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);

    void SetTrackEnable(int nAnimationTrack, bool bEnable);
    void SetTrackPosition(int nAnimationTrack, float fPosition);
    void SetTrackSpeed(int nAnimationTrack, float fSpeed);
    void SetTrackWeight(int nAnimationTrack, float fWeight);

    void AdvanceTime(float fElapsedTime, GameObject* pRootGameObject);

    //void ServerAdvanceTime(const ServerSyncData& syncData);
    std::vector<Animation_Sync> MakeSyncData();
    void ResetWeight();

public:
    std::shared_ptr<GameObject>            m_pModelRootObject = NULL;

    virtual void OnRootMotion(GameObject* pRootGameObject) {}
    virtual void OnAnimationIK(GameObject* pRootGameObject) {}

    XMFLOAT3 HipsPosition{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3PrevHipsPosition{ 0.0f, 0.0f, 0.0f };
};
