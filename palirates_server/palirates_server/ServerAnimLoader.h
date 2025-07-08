#pragma once
#include "AnimationSetCore.h"

#define VERTEXT_POSITION				0x0001
#define VERTEXT_BONE_INDEX_WEIGHT		0x1000

class GameObject;

class CStandardMesh
{
public:
    CStandardMesh() = default;
    virtual ~CStandardMesh() = default;

    void LoadMeshFromFile(FILE* pInFile);
    void LoadMeshFrom_OtherFile(const char* pstrFileName);
    void AddRef() { m_nReferences++; }
    void Release() { if (--m_nReferences <= 0) delete this; }

    char							m_pstrMeshName[64] = { 0 };

private:
    int								m_nReferences = 0;

protected:
    UINT							m_nType = 0x00;

    XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);

    int								m_nVertices = 0;
    XMFLOAT3* m_pxmf3Positions = NULL;

    int								m_nSubMeshes = 0;
    int* m_pnSubSetIndices = NULL;
    UINT** m_ppnSubSetIndices = NULL;
};

class CSkinnedMesh : public CStandardMesh
{
public:
    CSkinnedMesh() = default;
    virtual ~CSkinnedMesh() = default;

    int                                m_nSkinningBones = 0;        
    char							(*m_ppstrSkinningBoneNames)[64];
    std::vector<GameObject*> m_ppSkinningBoneFrameCaches;
    XMFLOAT4X4* m_pxmf4x4BindPoseBoneOffsets = NULL;

    void AddRef() {}
    void Release() {}

    void PrepareSkinning(const std::shared_ptr<GameObject>& root) {  }

    void LoadSkinInfoFromFile(FILE* pInFile);

protected:
    int								m_nBonesPerVertex = 4;
    XMINT4* m_pxmn4BoneIndices = NULL;
    XMFLOAT4* m_pxmf4BoneWeights = NULL;
};



class MeshManager
{
public:
    static std::unordered_map<std::string, std::shared_ptr<CStandardMesh>> mesh_cache;
    static std::mutex cache_mutex;  

    static std::shared_ptr<CStandardMesh> GetMesh(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = mesh_cache.find(name);
        if (it != mesh_cache.end()) return it->second;
        return nullptr;
    }

    static void AddMesh(const std::string& name, std::shared_ptr<CStandardMesh> mesh)
    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        mesh_cache[name] = mesh;
    }

    static bool Contains(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        return mesh_cache.find(name) != mesh_cache.end();
    }
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