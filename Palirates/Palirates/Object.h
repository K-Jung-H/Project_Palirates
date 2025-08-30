//-----------------------------------------------------------------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"
#include "Object_StateMachine.h"

#define DIR_FORWARD               0x01
#define DIR_BACKWARD            0x02
#define DIR_LEFT               0x04
#define DIR_RIGHT               0x08
#define DIR_UP                  0x10
#define DIR_DOWN               0x20

class CShader;
class CTerrainShader;
class CStandardShader;
class Plane_Shader;

class Deferred_CTerrainShader;
class Deferred_Plane_Shader;
class CS_Wave_Shader;
class CSkyBoxShader;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define RESOURCE_TEXTURE1D         0x01
#define RESOURCE_TEXTURE2D         0x02
#define RESOURCE_TEXTURE2D_ARRAY   0x03   
#define RESOURCE_TEXTURE2DARRAY      0x04
#define RESOURCE_TEXTURE_CUBE      0x05
#define RESOURCE_BUFFER            0x06
#define RESOURCE_STRUCTURED_BUFFER 0x07

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
    bool bStateChange = false;
    int changedStateNum = -1;
    float hp;
    bool bBreathHit;
    int stateEnum;
    AnimUpdateMode animMode = AnimUpdateMode::Full;
};

class CTexture
{
public:
    CTexture() = default;

    CTexture(int nTextures, UINT nTextureType,
        int nSamplers,
        int nGraphicsSrvRootParameters,
        int nComputeUavRootParameters,
        int nComputeSrvRootParameters,
        int nGraphicsSrvGpuHandles,
        int nComputeUavGpuHandles,
        int nComputeSrvGpuHandles,
        int nDsvHandles = 0);

    virtual ~CTexture();

public:
    void AddRef() { ++m_nReferences; }
    void Release() { if (--m_nReferences <= 0) delete this; }

    ID3D12Resource* GetResource(int index) const { return m_ppd3dTextures[index]; }
    void SetResource(ID3D12Resource* resource, int index) { m_ppd3dTextures[index] = resource; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int index) const { return m_GraphicsRootParameter_Srv_GpuDescriptorHandles[index]; }
    UINT GetTextureType() const { return m_nTextureType; }
    UINT GetTextureType(int index) const { return m_pnResourceTypes[index]; }

    int GetTextures() const { return static_cast<int>(m_ppd3dTextures.size()); }
    int GetGraphicsSrvRootParameters() const { return static_cast<int>(m_pnGraphicsSrvRootParameterIndices.size()); }
    int GetComputeSrvRootParameters() const { return static_cast<int>(m_pnComputeSrvRootParameterIndices.size()); }
    int GetComputeUavRootParameters() const { return static_cast<int>(m_pnComputeUavRootParameterIndices.size()); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGraphicsSrvGpuDescriptorHandle(int index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetComputeUavGpuDescriptorHandle(int index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetComputeSrvGpuDescriptorHandle(int index) const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVDescriptorHandle(int index) const { return m_d3dDsvCPUDescriptorHandles[index]; }

    void SetGraphicsSrvGpuDescriptorHandle(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle);
    void SetComputeUavGpuDescriptorHandle(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle);
    void SetComputeSrvGpuDescriptorHandle(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle);

    void SetGraphicsSrvRootParameter(int index, int rootParamIndex, int gpuHandleIndex, int srvDescriptors);
    void SetComputeSrvRootParameter(int index, int rootParamIndex, int gpuHandleIndex, int srvDescriptors);
    void SetComputeUavRootParameter(int index, int rootParamIndex, int gpuHandleIndex, int uavDescriptors);

    int GetGraphicsSrvRootParameterIndex(int index) const;
    int GetComputeSrvRootParameterIndex(int index) const;
    int GetComputeUavRootParameterIndex(int index) const;

    void UpdateGraphicsSrvShaderVariables(ID3D12GraphicsCommandList* commandList);
    void UpdateGraphicsSrvShaderVariable(ID3D12GraphicsCommandList* commandList, int parameterIndex, int textureIndex);
    void BindGraphicsSrvToRootParameter(ID3D12GraphicsCommandList* pd3dCommandList, int rootParamIndex, int textureIndex);

    void UpdateComputeSrvShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    void UpdateComputeSrvShaderVariable(ID3D12GraphicsCommandList* commandList, int parameterIndex, int textureIndex);
    void BindComputeSrvToRootParameter(ID3D12GraphicsCommandList* commandList, int rootParamIndex, int textureIndex);


    void UpdateComputeUavShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    void UpdateComputeUavShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int paramIndex, int textureIndex);
    void BindComputeUavToRootParameter(ID3D12GraphicsCommandList* pd3dCommandList, int rootParamIndex, int textureIndex);

    void ReleaseShaderVariables();
    void ReleaseUploadBuffers();

    void SetSampler(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle);
    void SetDSV(int index, D3D12_CPU_DESCRIPTOR_HANDLE handle) { m_d3dDsvCPUDescriptorHandles[index] = handle; }


    void LoadTextureFromDDSFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, wchar_t* filename, UINT resourceType, UINT index);
    void LoadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, void* data, UINT elements, UINT stride, DXGI_FORMAT format, UINT index);

    void CreateBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT index, void* data, UINT elements, UINT stride, DXGI_FORMAT format, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES state);
    void CreateStructuredBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT index, void* data, UINT elements, UINT stride, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES state);

    ID3D12Resource* CreateTexture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT index, UINT resourceType, UINT width, UINT height, UINT elements, UINT mips, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, D3D12_CLEAR_VALUE* clearValue);

    void SetRootParameterIndex(int index, UINT rootParameterIndex);

    DXGI_FORMAT GetBufferFormat(int index) const;
    int GetBufferElements(int index) const;
    int GetBufferStrides(int index) const;


    D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int index);
    D3D12_UNORDERED_ACCESS_VIEW_DESC GetUnorderedAccessViewDesc(int index);

    UINT GetTextureWidth(int index = 0) const {
        if (index >= 0 && index < static_cast<int>(m_nTextureWidths.size()))
            return m_nTextureWidths[index];
        return 0;
    }

    UINT GetTextureHeight(int index = 0) const {
        if (index >= 0 && index < static_cast<int>(m_nTextureHeights.size()))
            return m_nTextureHeights[index];
        return 0;
    }

private:
    int m_nReferences = 0;
    char m_pstrTextureName[64] = {};

    UINT m_nTextureType = 0;

    std::vector<UINT> m_nTextureWidths;
    std::vector<UINT> m_nTextureHeights;

    std::vector<UINT>                        m_pnResourceTypes;
    std::vector<ID3D12Resource*>             m_ppd3dTextures;
    std::vector<ID3D12Resource*>             m_ppd3dTextureUploadBuffers;

    std::vector<DXGI_FORMAT>                 m_pdxgiBufferFormats;
    std::vector<int>                         m_pnBufferElements;
    std::vector<int>                         m_pnBufferStrides;

    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_pd3dGraphicsSrvGpuDescriptorHandles;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_pd3dComputeUavGpuDescriptorHandles;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_pd3dComputeSrvGpuDescriptorHandles;

    std::vector<int>                         m_pnGraphicsSrvRootParameterIndices;
    std::vector<int>                         m_pnGraphicsSrvRootParameterDescriptors;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_GraphicsRootParameter_Srv_GpuDescriptorHandles;

    std::vector<int>                         m_pnComputeUavRootParameterIndices;
    std::vector<int>                         m_pnComputeUavRootParameterDescriptors;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_pd3dComputeUavRootParameterGpuDescriptorHandles;

    std::vector<int>                         m_pnComputeSrvRootParameterIndices;
    std::vector<int>                         m_pnComputeSrvRootParameterDescriptors;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_pd3dComputeSrvRootParameterGpuDescriptorHandles;

    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_pd3dSamplerGpuDescriptorHandles;

    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_d3dDsvCPUDescriptorHandles;


};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MATERIAL_ALBEDO_MAP            0x01
#define MATERIAL_SPECULAR_MAP         0x02
#define MATERIAL_NORMAL_MAP            0x04
#define MATERIAL_METALLIC_MAP         0x08
#define MATERIAL_EMISSION_MAP         0x10
#define MATERIAL_DETAIL_ALBEDO_MAP      0x20
#define MATERIAL_DETAIL_NORMAL_MAP      0x40



#define MATERIAL_Object_Type_ID_None 0
#define MATERIAL_Object_Type_ID_Player 1
#define MATERIAL_Object_Type_ID_Monster 2
#define MATERIAL_Object_Type_ID_Environment 3


class CGameObject;

struct Light_Material_Info
{
    float gRoughness;
    float gMetallic;
    float padding0;
    float padding1;

    XMFLOAT4 gSpecular;    // Specular: rgb + intensity
    XMFLOAT4 gEmissive;    // Emissive: rgb + intensity

    Light_Material_Info()
        : gRoughness(0.9f),
        gMetallic(0.001f),
        gSpecular(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)),
        gEmissive(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)),
        padding0(0.0f),
        padding1(0.0f)
    {
    }
};

struct Material_GPU_Packet
{
    XMFLOAT4 gAlbedoColor;
    UINT light_material_ID;
    UINT Blur_Mask_ID;
    UINT Object_Type_ID;
    UINT Outline_Color_ID;
};

class CMaterial
{
public:
    CMaterial(int nTextures);
    CMaterial(const CMaterial& other);
    virtual ~CMaterial();

    ::shared_ptr<CMaterial> CloneWithSharedTextures() const;
public:
    XMFLOAT4 m_cAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    UINT m_Material_ID = 0;
    UINT Blur_Mask_ID = 0;
    UINT Outline_Color_ID = 0;
    UINT Object_Type_ID = 0;

public:
    // not use
    float m_fGlossiness = 0.0f;
    float m_fGlossyReflection = 0.0f;

public:
    // Don't apply Shared_ptr
    CShader* m_pShader = NULL;


    UINT                     m_nType = 0x00; // Texture Map Type

    int                      m_nTextures = 0;
    std::vector<std::array<_TCHAR, 64>> m_ppstrTextureNames;
    std::vector<std::shared_ptr<CTexture>> m_ppTextures; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

//    CTexture** m_ppTextures = NULL; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

//    void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, std::shared_ptr<CGameObject> pParent, FILE* pInFile, CShader* pShader);
    void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, std::vector<std::shared_ptr<CTexture>>& textures, UINT textureIndex, std::shared_ptr<CGameObject> pParent, FILE* pInFile, CShader* pShader);

    void SetShader(CShader* pShader);
    void SetMaterialType(UINT nType) { m_nType |= nType; }
    void SetTexture(shared_ptr<CTexture> pTexture, UINT nTexture = 0);

    virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void Update_TextureShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseUploadBuffers();


public:
    static CShader* m_pStandardShader;
    static CShader* m_pSkinnedAnimationShader;

    static void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);

    void SetStandardShader() { CMaterial::SetShader(m_pStandardShader); }
    void SetSkinnedAnimationShader() { CMaterial::SetShader(m_pSkinnedAnimationShader); }
};

class Light_Material_Manager
{
private:
    static UINT index; // Max : 255
    static std::vector<Light_Material_Info> light_material_list;
    static bool reserved_update;
    static CTexture* material_info_buffer;
public:
    static void Initialize();
    static UINT Add_Material(const Light_Material_Info& material);
    static void Update_Material_Info(UINT idx, const Light_Material_Info& material);

    static void CreateStructuredBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    static void Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    static void UpdateGraphicsShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);



    static const Light_Material_Info& Get_Material(UINT idx);
    static size_t Get_Material_Count();
    static void Release();

    // New: Find similar material
    static int Find_Similar_Material(const Light_Material_Info& material, float tolerance = 0.01f);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct CALLBACKKEY
{
    float                       m_fTime = 0.0f;
    void* m_pCallbackData = NULL;
};

#define _WITH_ANIMATION_INTERPOLATION

class CAnimationCallbackHandler
{
public:
    CAnimationCallbackHandler() {}
    ~CAnimationCallbackHandler() {}

public:
    virtual void HandleCallback(void* pCallbackData, float fTrackPosition) {}
};

class CRootMotionCallbackHandler : public CAnimationCallbackHandler
{
public:
    CRootMotionCallbackHandler() {}
    ~CRootMotionCallbackHandler() {}

public:
    virtual void HandleCallback(void* pCallbackData, float fTrackPosition);
};


//#define _WITH_ANIMATION_SRT

class CAnimationSet
{
public:
    CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char* pstrName);
    ~CAnimationSet();

public:
    char                     m_pstrAnimationSetName[64];

    float                     m_fLength = 0.0f;
    int                        m_nFramesPerSecond = 0; //m_fTicksPerSecond

    int                        m_nKeyFrames = 0;
    float* m_pfKeyFrameTimes = NULL;
    XMFLOAT4X4** m_ppxmf4x4KeyFrameTransforms = NULL;

#ifdef _WITH_ANIMATION_SRT
    int                        m_nKeyFrameScales = 0;
    float* m_pfKeyFrameScaleTimes = NULL;
    XMFLOAT3** m_ppxmf3KeyFrameScales = NULL;
    int                        m_nKeyFrameRotations = 0;
    float* m_pfKeyFrameRotationTimes = NULL;
    XMFLOAT4** m_ppxmf4KeyFrameRotations = NULL;
    int                        m_nKeyFrameTranslations = 0;
    float* m_pfKeyFrameTranslationTimes = NULL;
    XMFLOAT3** m_ppxmf3KeyFrameTranslations = NULL;
#endif

public:
    XMFLOAT4X4 GetSRT(int nBone, float fPosition);
};

class CAnimationSets
{
public:
    CAnimationSets(int nAnimationSets);
    ~CAnimationSets();

private:
    int                        m_nReferences = 0;

public:
    void AddRef() { m_nReferences++; }
    void Release() { if (--m_nReferences <= 0) delete this; }

public:
    int                        m_nAnimationSets = 0;
   // CAnimationSet** m_pAnimationSet_list = NULL;
    std::vector<std::shared_ptr<CAnimationSet>> m_pAnimationSet_list;

    std::vector<int> m_vecUpperBodyBoneIndices;  
    std::vector<int> m_vecLowerBodyBoneIndices;  

    int                        m_nBoneFrames = 0;
    std::vector< CGameObject*>   m_ppBoneFrameCaches;
    void Bone_Info();
    std::string CAnimationSets::GetBoneName(int index);
    void ClassifyBones();

private:
    static std::unordered_map<std::string, std::shared_ptr<CAnimationSet>> s_GlobalAnimationSetCache;
public:
    static std::shared_ptr<CAnimationSet> AddOrGetSharedAnimationSet(std::shared_ptr<CAnimationSet> animSet, const std::string& fileName)
    {
        std::string key = fileName + "::" + animSet->m_pstrAnimationSetName;

        auto it = s_GlobalAnimationSetCache.find(key);
        if (it != s_GlobalAnimationSetCache.end())
            return it->second;

        s_GlobalAnimationSetCache[key] = animSet;
        return animSet;
    }
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

    int                      m_nCallbackKeys = 0;
    CALLBACKKEY* m_pCallbackKeys = NULL;

    CAnimationCallbackHandler* m_pAnimationCallbackHandler = NULL;

public:
    void SetAnimationSet(int nAnimationSet) { m_nAnimationSet = nAnimationSet; }

    void SetEnable(bool bEnable) { m_bEnable = bEnable; }
    void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
    void SetWeight(float fWeight) { m_fWeight = fWeight; }

    void SetPosition(float fPosition) { m_fPosition = fPosition; }
    float UpdatePosition(float fTrackPosition, float fTrackElapsedTime, float fAnimationLength);

    void SetCallbackKeys(int nCallbackKeys);
    void SetCallbackKey(int nKeyIndex, float fTime, void* pData);
    void SetAnimationCallbackHandler(CAnimationCallbackHandler* pCallbackHandler);

    void HandleCallback();
};

class CLoadedModelInfo
{
public:
    CLoadedModelInfo() {}
    ~CLoadedModelInfo();

    std::shared_ptr<CGameObject>                  m_pModelRootObject = NULL;


    int                      m_nSkinnedMeshes = 0;
    std::vector<std::shared_ptr<CSkinnedMesh>> m_ppSkinnedMeshes; //[SkinnedMeshes], Skinned Mesh Cache

    CAnimationSets* m_pAnimationSets = NULL;

public:
    void PrepareSkinning();
};


class CAnimationController
{
public:
    CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel);
    ~CAnimationController();

public:
    float                      m_fTime = 0.0f;

    int                      m_nAnimationTracks = 0;
    CAnimationTrack* m_pAnimationTracks = NULL;

    CAnimationSets* m_pAnimationSets = NULL;


    int                      m_nSkinnedMeshes = 0;
    std::vector<std::shared_ptr<CSkinnedMesh>> m_ppSkinnedMeshes; //[SkinnedMeshes], Skinned Mesh Cache

    ID3D12Resource** m_ppd3dcbSkinningBoneTransforms = NULL; //[SkinnedMeshes]
    XMFLOAT4X4** m_ppcbxmf4x4MappedSkinningBoneTransforms = NULL; //[SkinnedMeshes]

    int RootIndex{ 0 };

public:
    void Bone_Info();

    void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

    void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);

    void SetTrackEnable(int nAnimationTrack, bool bEnable);
    void SetTrackPosition(int nAnimationTrack, float fPosition);
    void SetTrackSpeed(int nAnimationTrack, float fSpeed);
    void SetTrackWeight(int nAnimationTrack, float fWeight);

    void SetCallbackKeys(int nAnimationTrack, int nCallbackKeys);
    void SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fTime, void* pData);
    void SetAnimationCallbackHandler(int nAnimationTrack, CAnimationCallbackHandler* pCallbackHandler);

    void AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject);
    void AdvanceTime2(float fElapsedTime, CGameObject* pRootGameObject);

    void ApplyCurrentAnimationPose(CGameObject* pRootGameObject);
    void ServerAdvanceTime(const ServerSyncData& syncData);
    std::vector<Animation_Sync> MakeSyncData();
    void ResetWeight();

public:
    std::shared_ptr<CGameObject>            m_pModelRootObject = NULL;

    virtual void OnRootMotion(CGameObject* pRootGameObject) {}
    virtual void OnAnimationIK(CGameObject* pRootGameObject) {}

    XMFLOAT3 HipsPosition{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3PrevHipsPosition{ 0.0f, 0.0f, 0.0f };
};

//==================================================================================

enum class EObjectType : uint32_t
{
    None = 0x00,
    MainPlayer = 0x01,
    Player = 0x02,
    Monster = 0x04,
    PlayerWeapon = 0x08,
    SelectPlayer = 0x10,
    MonsterWeapon = 0x20,
    DropWeapon = 0x40
};

inline EObjectType operator|(EObjectType a, EObjectType b)
{
    return static_cast<EObjectType>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline EObjectType operator&(EObjectType a, EObjectType b)
{
    return static_cast<EObjectType>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline EObjectType& operator|=(EObjectType& a, EObjectType b)
{
    a = a | b;
    return a;
}

class CHeightMapTerrain;

class WeaponObject
{
public:
    WeaponObject() {};
    ~WeaponObject() {};
    std::vector<std::shared_ptr<CGameObject>> pWeapon;
};


enum class TransformMode { Inherit, Independent, Disabled };


class CGameObject : public std::enable_shared_from_this<CGameObject>
{
private:
    TransformMode m_eTransformMode = TransformMode::Inherit;
    std::shared_ptr<CGameObject> m_pChild = nullptr;
    std::shared_ptr<CGameObject> m_pSibling = nullptr;

    bool Active = true;


public:
    static std::unordered_map<std::string, std::shared_ptr<CMesh>> MeshCache;
    static std::shared_ptr<CMesh> LoadMeshWithCache(const std::string& meshPath, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    static void ClearMeshCache();

    std::shared_ptr<CMesh> m_pMesh = NULL;

    std::vector<std::shared_ptr<CMaterial>>  Material_list;


public:
    std::shared_ptr<CAnimationController> m_pSkinnedAnimationController = NULL;
    int n_Animation = 0;
    std::shared_ptr<CGameObject> m_pRootModel = NULL;

public:
    char                     m_pstrFrameName[64];
    EObjectType type = EObjectType::None;
    Monster_Type mType = Monster_Type::ETC;
    bool HasType(EObjectType mask) const
    {
        return (type & mask) != EObjectType::None;
    }

    std::shared_ptr<CGameObject> m_pParent = NULL;

    XMFLOAT4X4            m_xmf4x4Parent{};
    XMFLOAT4X4            m_xmf4x4World{};

    XMFLOAT3 m_xmf3RotationAxis;

    XMVECTOR target_dir{};
    bool     m_bInAir = false;
    XMVECTOR m_vVelocity = XMVectorZero();
    XMFLOAT3 previous_position{ 0.0f,0.0f,0.0f };

    float    m_fMoveSpeed = 5.0f;
    float    m_fRotationSpeed = 360.0f;
    float    m_fInitialUpSpeed = 10.0f;
    float    m_fGravity = 9.8f;

    XMFLOAT3 Blending_color{};
    float   Blending_value = 0.0f;
    
    void SetMoveSpeed(float s) { m_fMoveSpeed = s; }
    void SetInitialUpSpeed(float s) { m_fInitialUpSpeed = s; }
    void Launch(const XMVECTOR& target_dir);
    WeaponObject* pWeapon;

    bool bIsControllable{ true };
    bool bIsInvincible = false;
    float invincibleTimeRemaining = 0.0f;
    float invincibleDuration = 2.0f;

    std::unordered_set<int> RootMotionTrackSet;

    int RootIndex{ 0 };

    char* WeaponName = "";
    BoundingOrientedBox m_WorldOBB;
    BoundingOrientedBox cachedWorldOBB;
    XMMATRIX customRotation = XMMatrixIdentity();
    XMFLOAT4X4 WeaponMatrix{};

    XMFLOAT3 m_TargetPosition{ 0.0f,0.0f,0.0f };

    bool bUpdateOBB{ true };
    //std::shared_ptr<CGameObject> Weapon_ptr = nullptr;
    std::vector<std::shared_ptr<CGameObject>> Weapon_ptr;

    bool Test_Mode{ false };

    float maxHP{ 100.0f };
    float currentHP{ 100.0f };

public:
    CGameObject(const std::string_view& name = "No_name");
    CGameObject(int nMaterials, const std::string_view& name = "No_name");

    CGameObject(const CGameObject& other);
    CGameObject& operator=(const CGameObject& other);


    // Deep Copy
    std::shared_ptr<CGameObject> Clone(bool withHierarchy = true);

    // Deep Copy Hierarchy & Shallow Copy Resource
    static std::shared_ptr<CGameObject> Make_Instance(std::shared_ptr<CGameObject> modelRoot, bool withHierarchy = true);


    virtual ~CGameObject();

    void Set_TransformMode(TransformMode new_TransformMode) { m_eTransformMode = new_TransformMode; }

    std::shared_ptr<CGameObject> Get_Child();
    std::shared_ptr<CGameObject> Get_Sibling();

    void Set_Active(bool active, bool bIsRoot = true);
    bool Get_Active() { return Active; }

    void SetMesh(std::shared_ptr<CMesh> pMesh);
    void SetShader(CShader* pShader);
    void SetShader(int nMaterial, CShader* pShader);
    void SetMaterial(int nMaterial, CMaterial* pMaterial);

    void SetOutlineColor(int id);
    void SetObject_Type_ID(int id); // for zoom effect type
    void SetBlurMask(bool value);

    bool GetBlurMask();
    int GetOutlineColor();
    int GetObject_Type_ID();

    void Set_Color_Blending(XMFLOAT3& color = XMFLOAT3(1.0f, 1.0f, 1.0), float blending_value = 1.0f);
    void Update_Color_Blending(float update_bleeding_value = 1.0f);


    void Set_Child(std::shared_ptr<CGameObject> pChild);

    void Obj_Info(int depth = 0);
    void Set_Name(std::string_view name);

    const char* Get_Name() const { return m_pstrFrameName; }

    virtual void BuildMaterials(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {}

    virtual void OnPrepareAnimate() {}
    virtual void Animate(float fTimeElapsed);

    virtual bool IsVisible(CCamera* pCamera);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
    virtual void Render_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
    virtual void Render_Depth(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

    
    virtual void OnLateUpdate() {}

    virtual void Set_Last_Pos(XMFLOAT3 pos);
    virtual void Record_Last_Pos();

    virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseShaderVariables();

    virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
    virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial);

    virtual void ReleaseUploadBuffers();

    XMFLOAT3 GetPosition();
    XMFLOAT3 GetLook();
    XMFLOAT3 GetUp();
    XMFLOAT3 GetRight();

    XMFLOAT3 GetToParentPosition();
    XMFLOAT3 Get_World_Position();

    std::shared_ptr<CGameObject> Get_Root_Object();
    XMFLOAT3 Get_Root_WorldPosition();
    XMFLOAT3 Get_Root_Obj_Displacement();

    void Move(XMFLOAT3 xmf3Offset);

    void Modify_World_Position(XMFLOAT3 newPosition);
    void Modify_World_Up_Vector(XMFLOAT3 newUpvector);

    virtual void SetPosition(float x, float y, float z);
    virtual void SetPosition(XMFLOAT3 xmf3Position);
    void SetScale(float x, float y, float z, bool keep_pos = false);
    void SetScale(XMFLOAT3 scale, bool keepPosition = false);
    void MoveStrafe(float fDistance = 1.0f);
    void MoveUp(float fDistance = 1.0f);
    void MoveForward(float fDistance = 1.0f);

    void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
    void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }

    void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
    void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
    void Rotate(XMFLOAT4* pxmf4Quaternion);

    void RotateInWorldAroundUp(float fAngle);
    void RotateInWorld(XMFLOAT3* pxmf3WorldAxis, float fAngle);

    void SetLookDirection(float x, float y, float z);
    virtual void SetLookDirection(const XMFLOAT3& look);

    void Set_LookDirection_LookAt(float x, float y, float z);
    void Set_LookDirection_LookAt(const XMFLOAT3& lookDir);

    virtual void AlignWithNormal(XMFLOAT3& newNormal);


    std::shared_ptr<CGameObject> GetParent() { return(m_pParent); }
    void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);


    std::shared_ptr<CGameObject> FindFrame(const char* pstrFrameName);

    std::shared_ptr<CTexture>FindReplicatedTexture(const _TCHAR* pstrTextureName);

    UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

public:
    //void FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh);
    void FindAndSetSkinnedMesh(std::vector<std::shared_ptr<CSkinnedMesh>>& outSkinnedMeshes);
    void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
    void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);
    
    void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::shared_ptr<CGameObject> pParent, FILE* pInFile, CShader* pShader);

    static void LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel, char* pstrFileName);
    static std::shared_ptr<CGameObject> LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, std::shared_ptr<CGameObject> pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes);
    static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader);

    static std::shared_ptr<CGameObject> Load_Scene_HierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, std::shared_ptr<CGameObject> pParent, FILE* pInFile, CShader* pShader);
    static CLoadedModelInfo* Load_Scene_File(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader);
    static void FlattenGameObjectHierarchy(std::shared_ptr<CGameObject> node, std::vector<shared_ptr<CGameObject>>& outList);

    static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);

    virtual std::string  Get_Mesh_Name();


    virtual BoundingOrientedBox* Get_Collider();
    virtual void Add_Collider(float cube_length);
    virtual void Set_Collider(BoundingOrientedBox* ptr = NULL);


    std::shared_ptr<CAnimationController> GetSkinnedAnimationController() { return m_pSkinnedAnimationController; }
    void DelSkinnedAnimationController() { m_pSkinnedAnimationController.reset(); }

public:
    // Using CHeightMapTerrain
    virtual int Get_Tile(float x, float z) { return -1; };
    virtual void Get_Active_TileNum_List(std::vector<int>& tile_list) {};
    virtual void Check_Culling(CCamera* pCamera) {};

public:
    virtual ServerSyncData MakeSyncData();
    virtual void ApplySyncData(const ServerSyncData& syncData);

    virtual std::shared_ptr<CGameObject> DropWeapon(const char* targetName);
    virtual void RestoreWeapon(const char* targetName);
    std::shared_ptr<CGameObject> GetWeapon(bool withHierarchy);

    std::vector<float> prevWeights;
    std::vector<float> targetWeights;

    void bUpdateOBBOn() { bUpdateOBB = true; }
    void bUpdateOBBOff() { bUpdateOBB = false; }
    bool GetbUpdateOBB() {
        return bUpdateOBB;
    }
    virtual void SetupWeaponCollider() {};

    //==================

    int m_nPlayerId = -1;
    void SetID(int id) { m_nPlayerId = id; }
    int GetID() const { return m_nPlayerId; }

protected:
    bool bCanCollide = true;

public:
    void SetCanCollide(bool canCollide) { bCanCollide = canCollide; }
    bool CanCollide() const { return bCanCollide; }
};

//==================================================================================


class CHeightMapTerrain : public CGameObject
{
private:
    shared_ptr<CTexture> m_TerrainBaseTexture;
    shared_ptr<CTexture> m_TerrainDetailTexture;
    Deferred_CTerrainShader* m_TerrainShader = nullptr;
    CMaterial* m_TerrainMaterial = nullptr;
    shared_ptr<CHeightMapImage> m_pHeightMapImage = nullptr;
    shared_ptr<CMesh> m_pFullMesh = NULL;
private:
    int                     m_nWidth;
    int                     m_nLength;
    int                     m_nDepth;
    XMFLOAT3               m_xmf3Scale;


    int         tile_number = 0;
    XMFLOAT2 Tile_Start_Pos{};
    XMFLOAT2 Area_LT{};
    XMFLOAT2 Area_RB{};

    
public:
    CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, LPCTSTR pFileName,
        int start_x_pos, int start_z_pos, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, int Vertex_gap = 1, int nMaxDepth = 1, shared_ptr<CHeightMapImage> sharedHeightMapImage = NULL);
    virtual ~CHeightMapTerrain();

    void DivideIntoChildren(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, LPCTSTR pFileName, XMFLOAT3 xmf3Scale, int Vertex_gap);

    void Set_Tile(int n);
    void Set_FullMesh(shared_ptr<CMesh> new_mesh) { m_pFullMesh = new_mesh; }

    shared_ptr<CMesh> Get_FullMesh() { return m_pFullMesh; }
    XMFLOAT2 Get_Terrain_LT() const { return Area_LT; }
    XMFLOAT2 Get_Terrain_RB() const { return Area_RB; }

    float Get_Height(float x, float z, bool bReverseQuad = false);
    float Get_Height(float x, float z, bool bReverseQuad, CHeightMapTerrain*& last_tile_ptr);
    float Get_Mesh_Height(float x, float z, bool bReverseQuad = false);
    float Get_Mesh_Height(float x, float z, bool bReverseQuad, CHeightMapTerrain*& last_tile_ptr);

    XMFLOAT3 Get_Mesh_Normal(float x, float z);
    XMFLOAT3  Get_Mesh_Normal(float x, float z, CHeightMapTerrain*& last_tile_ptr);

    int Get_Tile(float x, float z);
    int Get_Tile(float x, float z, CHeightMapTerrain*& last_tile_ptr);

    int Get_TileNum() { return tile_number; }
    virtual void Get_Active_TileNum_List(std::vector<int>& tile_list);
    virtual BoundingOrientedBox* Get_Collider();


    int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
    int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

    XMFLOAT3 GetScale() { return(m_xmf3Scale); }
    float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
    float GetLength() { return(m_nLength * m_xmf3Scale.z); }

    void Check_Culling(CCamera* pCamera);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
    virtual void Render_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

    void Reset_Obj_List_Height(std::vector<std::shared_ptr<CGameObject>> obj_list);
    void Reset_Obj_List_Up_Vector(std::vector<std::shared_ptr<CGameObject>> obj_list);
};

class Boat_Object : public CGameObject
{
private:
    XMFLOAT3 wave_normal_vector{};
    float wave_height = 0.0f;
    float smoothedHeight = 0.0f;

    XMFLOAT3 boat_up_vector{};
    XMFLOAT3 m_xmf3Velocity{};

    float                    m_fMaxVelocityXZ = 0.0f;
    float                    m_fFriction = 0.0f;

    bool Sail_Mode = true; // false == Stay_Mode
public:
    std::unordered_map<std::string, shared_ptr<CGameObject>> Boat_Frames_Marker;


    Boat_Object();
    virtual ~Boat_Object();

    virtual void Move(float fSpeed, bool bUpdateVelocity);
    virtual void MoveForward(float speed);
    void Yaw(float angle);

    void UpdateRotationFromWave(float fTimeElapsed);
    void UpdateMovementOnWave(float fTimeElapsed);
    void Set_Wave_Normal(XMFLOAT3& normal) { wave_normal_vector = normal; }
    void Set_Wave_Height(float height) { wave_height = height; }

    void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
    void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }
    void Add_Rotate(float angleDelta);

    virtual void Animate(float fTimeElapsed);
    void HandleBoundaryReflection(float boundary);

    void Set_Velocity(XMFLOAT3 new_velo) { m_xmf3Velocity = new_velo; }
    XMFLOAT3 Get_Velocity() { return m_xmf3Velocity; }
    float Get_RotationSpeed() { return m_fRotationSpeed; }
    virtual void Record_Last_Pos();

    void RegisterMarker(const std::string& name, shared_ptr<CGameObject> node) { Boat_Frames_Marker[name] = node; }

    bool GetMarkerWorldPosition(const std::string& name, XMFLOAT3& outWorldPos);

    bool Is_Moving();
    bool Get_Sail_Mode() { return Sail_Mode; }
    void Set_Sail_Mode(bool mode) { Sail_Mode = mode; }
    void Change_Model(bool is_stay_mode);

};

class Plane_Object : public CGameObject
{
public:
    static Plane_Shader* plane_shader;
    static Deferred_Plane_Shader* deferred_plane_shader;

private:
    shared_ptr<CTexture> Plane_BaseTexture = NULL;
    shared_ptr<CTexture> Plane_DetailTexture = NULL;

public:
    CMaterial* Plane_Material = NULL;

    Plane_Object() {}
    Plane_Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, int nLength, XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
    virtual ~Plane_Object();

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    virtual void Render_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

    void Set_BaseTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* filename);
    void Set_DetailTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* filename);

};

#define Max_Wave_Trail 640

struct BoatWakeTrail 
{
    XMFLOAT2 position;
    XMFLOAT2 direction;
    float age;
    float boat_velocity;

    float padding0;
    float padding1;
};

struct CB_Wave_Trail_Info
{
    UINT  g_NumTrails;
    float g_GlobalTime;
    float g_BaseStrength;
    float g_DecayRate;
    float g_TimeDecayRate;
};

class Wave_Object : public Plane_Object
{
public:
    static CS_Wave_Shader* cs_wave_shader;

protected:
    std::vector<BoatWakeTrail> wakeTrails; // 크기 고정 (예: 64)
    int wakeTrailIndex = 0;
    int wakeTrailCount = 0;

    CTexture* wave_trail_data_texture = NULL;
    ID3D12Resource* m_pWakeTrailUploadBuffer = NULL;

    ID3D12Resource* wave_trail_info = nullptr;
    CB_Wave_Trail_Info* mapped_wave_trail_info = nullptr;

protected:
    CTexture* wave_general_data_texture = NULL; // 0: Reading_Height, 1: Writting_Height, 2: Writting_Normal -> Using for render is 1, 2
    ID3D12Resource* Pos_Normal_ReadBack_buffer = NULL;


    UINT desiredTexelSize = 0;
    UINT Tex_Length = 0;
    float Side_Length = 0.0f;
    bool bPingPongToggle = false;


    XMFLOAT3 World_Boat_Pos = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 World_Boat_Dir = { 0.0f,1.0f, 0.0f };
    float World_Boat_Velocity = 0.0f;

    XMFLOAT3 BoatPos_WaveNormal = { 0.0f, 0.0f, 0.0f };
    float BoatPos_WaveHeight = 0.0f;

public:
    Wave_Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, int nLength, int side_vertex_n = 100, bool use_deferred_shader = true);
    virtual ~Wave_Object();

    void Copy_Buffer_Data(ID3D12GraphicsCommandList* pd3dCommandList);
    XMFLOAT3 Readback_Buffer_Data();
    
    void Animate(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
    void Animate_Wave_Trail_Buffer(float fTimeElapsed);
    void Update_Wave_Trail_Buffer(ID3D12GraphicsCommandList* pd3dCommandList);
    void Set_Wave_Trail_Info(CB_Wave_Trail_Info& wave_trail_info);

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    virtual void Render_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

    void Synchronize_Wave_to_Boat(Boat_Object* boat_ptr);

};



class CSkyBox : public CGameObject
{
private:
    static CSkyBoxShader* pSkybox_shader;
    static shared_ptr<CSkyBoxMesh> pSkyBoxMesh;

    shared_ptr<CTexture> skybox_texture = NULL;
    shared_ptr<CMaterial>skybox_material = NULL;
public:
    static void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);

    CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    virtual ~CSkyBox();

    virtual void Set_BaseTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* filename);

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};


class Trail_Object : public CGameObject
{
private:
    Trail_Mesh* trail_mesh = NULL;
    float m_fAccumulatedTime = 0.0f;

    shared_ptr<CGameObject> m_pTargetObject = nullptr;
    bool m_bUseTargetScale = true;

    XMFLOAT3 m_vLocalTop = {};
    XMFLOAT3 m_vLocalBottom = {};

    // Minimum segment creation interval N/s
    float m_fSegmentInterval = 0.001f;
    float m_fSegmentTimer = 0.0f;

public:
    Trail_Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4 main_color = {1.0f,1.0f, 1.0f, 1.0f});
    virtual ~Trail_Object();

    void Set_Trail_Target(shared_ptr<CGameObject> target, bool bUseScale = true) { m_pTargetObject = target;  m_bUseTargetScale = bUseScale; }
    void Set_Trail_LocalOffset(const XMFLOAT3& top, const XMFLOAT3& bottom) { m_vLocalTop = top; m_vLocalBottom = bottom; }
    void Set_Main_Color(XMFLOAT4 new_color) { if (trail_mesh) trail_mesh->Set_MainColor(new_color); }
    void Set_SubColor(XMFLOAT4 new_color) { if (trail_mesh) trail_mesh->Set_SubColor(new_color); }

    virtual void Animate(float fTimeElapsed);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
    Trail_Mesh* GetTrailMesh() { return trail_mesh; }

};

//==================================================================================

//==================================================================================


class CMonsterObject : public CGameObject
{
protected:
    std::unique_ptr<MonsterStateMachine> m_StateMachine;

public:
    int test_num{ 0 };
    int Hit_Track_idx = -1;
    AnimUpdateMode animMode = AnimUpdateMode::Full;
    int            poseSkipCounter = 0;

    CMonsterObject() {};
    virtual ~CMonsterObject();

    virtual void Animate(float fTimeElapsed);
    virtual MonsterStateMachine* GetStateMachine() { return m_StateMachine.get(); }

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
    virtual void Render_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

    virtual void SetupWeaponCollider();
    void ApplySyncData(const ServerSyncData& syncData) override;
};

class CFishManObject : public CMonsterObject
{
public:
    CFishManObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
    virtual ~CFishManObject() {};

    FishManStateMachine* GetStateMachine() override { return static_cast<FishManStateMachine*>(m_StateMachine.get()); }
};

class CAnubisObject : public CMonsterObject
{
public:
    CAnubisObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
    virtual ~CAnubisObject() {};

    AnubisStateMachine* GetStateMachine() override { return static_cast<AnubisStateMachine*>(m_StateMachine.get()); }
};

class CDragonObject : public CMonsterObject
{
public:
    CDragonObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
    virtual ~CDragonObject() {};

    DragonStateMachine* GetStateMachine() override { return static_cast<DragonStateMachine*>(m_StateMachine.get()); }
};

class CGargoyleObject : public CMonsterObject
{
public:
    CGargoyleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
    virtual ~CGargoyleObject() {};

    GargoyleStateMachine* GetStateMachine() override { return static_cast<GargoyleStateMachine*>(m_StateMachine.get()); }
};
