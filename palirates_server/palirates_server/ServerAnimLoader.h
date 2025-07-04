#pragma once
#include <vector>
#include <string>
#include <memory>
#include <DirectXMath.h>
#include "GameObject.h"
#include "AnimationSetCore.h"

class CSkinnedMesh
{
public:
	CSkinnedMesh() = default;
	virtual ~CSkinnedMesh() = default;

protected:
	int								m_nBonesPerVertex = 4;

public:
	int								m_nSkinningBones = 0;

	char							(*m_ppstrSkinningBoneNames)[64]; 
	std::vector<GameObject*> m_ppSkinningBoneFrameCaches; //[m_nSkinningBones]

	XMFLOAT4X4* m_pxmf4x4BindPoseBoneOffsets = NULL; //[m_nSkinningBones], Transposed

public:
	void PrepareSkinning(std::shared_ptr<GameObject> pModelRootObject) {};
	void LoadSkinInfoFromFile(FILE* pInFile) {};

	BoundingOrientedBox Get_WorldOBB() {};
	BoundingOrientedBox Get_WorldOBB_FromSkinnedVertices() {};
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