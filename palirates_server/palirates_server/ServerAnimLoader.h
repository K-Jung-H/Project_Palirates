#pragma once
#include "AnimationSetCore.h"

BYTE ReadStringSafeFromFile(FILE* pInFile, char* pstrToken, size_t bufferSize);
int  ReadIntegerFromFile(FILE* pInFile);
float ReadFloatFromFile(FILE* pInFile);

class GameObject;

class CSkinnedMesh
{
public:
    CSkinnedMesh() = default;
    ~CSkinnedMesh() = default;

    int                                m_nSkinningBones = 0;        
    std::vector<std::string>           m_boneNames;                 
    std::vector<GameObject*>           m_boneFrameCaches;            
    std::vector<DirectX::XMFLOAT4X4>   m_bindPoseOffsets;           

    void AddRef() {}
    void Release() {}

    void PrepareSkinning(const std::shared_ptr<GameObject>& root) {  }

    void LoadSkinInfoFromFile(FILE* fp);
};

class CLoadedModelInfo
{
public:
	CLoadedModelInfo() = default;
	virtual ~CLoadedModelInfo() = default;

	std::shared_ptr<GameObject>                  m_pModelRootObject = NULL;

	int                      m_nSkinnedMeshes = 0;
	std::vector<std::shared_ptr<CSkinnedMesh>> m_ppSkinnedMeshes; //[SkinnedMeshes], Skinned Mesh Cache

	CAnimationSets* m_pAnimationSets = NULL;

public:
	void PrepareSkinning();
};