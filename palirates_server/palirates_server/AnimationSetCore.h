#pragma once

using namespace DirectX;

class GameObject;

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
public: static std::shared_ptr<CAnimationSet> AddOrGetSharedAnimationSet(std::shared_ptr<CAnimationSet> animSet, const std::string& fileName);
private:
    static std::unordered_map<std::string,
        std::shared_ptr<CAnimationSet>> s_globalCache;
};
