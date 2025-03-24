//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

class CShader;
class CTerrainShader;
class CStandardShader;

class Deferred_CTerrainShader;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define RESOURCE_TEXTURE1D			0x01
#define RESOURCE_TEXTURE2D			0x02
#define RESOURCE_TEXTURE2D_ARRAY	0x03	//[]
#define RESOURCE_TEXTURE2DARRAY		0x04
#define RESOURCE_TEXTURE_CUBE		0x05
#define RESOURCE_BUFFER				0x06
#define RESOURCE_STRUCTURED_BUFFER 0x07


class CTexture 
{
public:
	CTexture(int nTextures, UINT nTextureType,
		int nSamplers,
		int nGraphicsSrvRootParameters,
		int nComputeUavRootParameters,
		int nComputeSrvRootParameters,
		int nGraphicsSrvGpuHandles,
		int nComputeUavGpuHandles,
		int nComputeSrvGpuHandles);

	virtual ~CTexture();

public:
	void AddRef() { ++m_nReferences; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	// 텍스처 정보
	ID3D12Resource* GetResource(int index) const { return m_ppd3dTextures[index]; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int index) const { return m_GraphicsRootParameter_Srv_GpuDescriptorHandles[index]; }
	UINT GetTextureType() const { return m_nTextureType; }
	UINT GetTextureType(int index) const { return m_pnResourceTypes[index]; }

	// 개수 반환
	int GetTextures() const { return static_cast<int>(m_ppd3dTextures.size()); }
	int GetGraphicsSrvRootParameters() const { return static_cast<int>(m_pnGraphicsSrvRootParameterIndices.size()); }
	int GetComputeSrvRootParameters() const { return static_cast<int>(m_pnComputeSrvRootParameterIndices.size()); }
	int GetComputeUavRootParameters() const { return static_cast<int>(m_pnComputeUavRootParameterIndices.size()); }

	// Descriptor Handle 접근
	D3D12_GPU_DESCRIPTOR_HANDLE GetGraphicsSrvGpuDescriptorHandle(int index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetComputeUavGpuDescriptorHandle(int index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetComputeSrvGpuDescriptorHandle(int index) const;

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

	void UpdateComputeSrvShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateComputeSrvShaderVariable(ID3D12GraphicsCommandList* commandList, int parameterIndex, int textureIndex);

	void UpdateComputeUavShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateComputeUavShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int paramIndex, int textureIndex);
	

	void ReleaseShaderVariables();
	void ReleaseUploadBuffers();

	void SetSampler(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle);

	void LoadTextureFromDDSFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, wchar_t* filename, UINT resourceType, UINT index);
	void LoadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, void* data, UINT elements, UINT stride, DXGI_FORMAT format, UINT index);

	void CreateBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, void* data, UINT elements, UINT stride, DXGI_FORMAT format, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES state, UINT index);
	void CreateStructuredBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, void* data, UINT elements, UINT stride, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES state, UINT index);

	ID3D12Resource* CreateTexture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT index, UINT resourceType, UINT width, UINT height, UINT elements, UINT mips, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, D3D12_CLEAR_VALUE* clearValue);

	void SetRootParameterIndex(int index, UINT rootParameterIndex);

	DXGI_FORMAT GetBufferFormat(int index) const;
	int GetBufferElements(int index) const;

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int index);
	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUnorderedAccessViewDesc(int index);

private:
	int m_nReferences = 0;
	char m_pstrTextureName[64] = {};

	UINT m_nTextureType = 0;

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
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MATERIAL_ALBEDO_MAP				0x01
#define MATERIAL_SPECULAR_MAP			0x02
#define MATERIAL_NORMAL_MAP				0x04
#define MATERIAL_METALLIC_MAP			0x08
#define MATERIAL_EMISSION_MAP			0x10
#define MATERIAL_DETAIL_ALBEDO_MAP		0x20
#define MATERIAL_DETAIL_NORMAL_MAP		0x40

class CGameObject;

struct Material_Info
{
	XMFLOAT4 gAlbedoColor;   

	float gRoughness; 
	float gMetallic;  
	float gSpecular_intensity;  
	float gEmissive_intensity;
};

class CMaterial
{
public:
	CMaterial(int nTextures);
	CMaterial(const CMaterial& other);
	virtual ~CMaterial();

public:
	// 거칠기 (0 = 매끄러움, 1 = 거침)
	// 금속성 (0 = 비금속, 1 = 금속)
	// 반사 계수 (Specular Intensity)

	XMFLOAT4 m_cAlbedo = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 m_cEmissive = { 0.0f, 0.0f, 0.0f, 0.0f };

	float m_fRoughness = 0.0f;
	float m_fMetallic = 0.0f; 
	float m_fSpecular = 1.0f; 


public:
	// not use
	XMFLOAT4 m_xmf4SpecularColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	float m_fGlossiness = 0.0f;
	float m_fGlossyReflection = 0.0f;

public:
	// Don't apply Shared_ptr
	CShader* m_pShader = NULL;


	UINT							m_nType = 0x00; // Texture Map Type

	int 							m_nTextures = 0;
	_TCHAR							(*m_ppstrTextureNames)[64] = NULL;
	CTexture						**m_ppTextures = NULL; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

	void LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR *pwstrTextureName, CTexture **ppTexture, CGameObject *pParent, FILE *pInFile, CShader *pShader);

	void SetShader(CShader* pShader);
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void SetTexture(CTexture* pTexture, UINT nTexture = 0);

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseUploadBuffers();


public:
	static CShader					*m_pStandardShader;
	static CShader					*m_pSkinnedAnimationShader;

	static void CMaterial::PrepareShaders(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);

	void SetStandardShader() { CMaterial::SetShader(m_pStandardShader); }
	void SetSkinnedAnimationShader() { CMaterial::SetShader(m_pSkinnedAnimationShader); }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct CALLBACKKEY
{
   float  							m_fTime = 0.0f;
   void  							*m_pCallbackData = NULL;
};

#define _WITH_ANIMATION_INTERPOLATION

class CAnimationCallbackHandler
{
public:
	CAnimationCallbackHandler() { }
	~CAnimationCallbackHandler() { }

public:
   virtual void HandleCallback(void *pCallbackData, float fTrackPosition) { }
};

class CRootMotionCallbackHandler : public CAnimationCallbackHandler
{
public:
	CRootMotionCallbackHandler() { }
	~CRootMotionCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition);
};


//#define _WITH_ANIMATION_SRT

class CAnimationSet
{
public:
	CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char *pstrName);
	~CAnimationSet();

public:
	char							m_pstrAnimationSetName[64];

	float							m_fLength = 0.0f;
	int								m_nFramesPerSecond = 0; //m_fTicksPerSecond

	int								m_nKeyFrames = 0;
	float							*m_pfKeyFrameTimes = NULL;
	XMFLOAT4X4						**m_ppxmf4x4KeyFrameTransforms = NULL;

#ifdef _WITH_ANIMATION_SRT
	int								m_nKeyFrameScales = 0;
	float							*m_pfKeyFrameScaleTimes = NULL;
	XMFLOAT3						**m_ppxmf3KeyFrameScales = NULL;
	int								m_nKeyFrameRotations = 0;
	float							*m_pfKeyFrameRotationTimes = NULL;
	XMFLOAT4						**m_ppxmf4KeyFrameRotations = NULL;
	int								m_nKeyFrameTranslations = 0;
	float							*m_pfKeyFrameTranslationTimes = NULL;
	XMFLOAT3						**m_ppxmf3KeyFrameTranslations = NULL;
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
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	int								m_nAnimationSets = 0;
	CAnimationSet					**m_pAnimationSet_list = NULL;

	std::vector<int> m_vecUpperBodyBoneIndices;  // 상체
	std::vector<int> m_vecLowerBodyBoneIndices;  // 하체

	int								m_nBoneFrames = 0; 
	std::vector< CGameObject*>	m_ppBoneFrameCaches;
	void Bone_Info();
	std::string CAnimationSets::GetBoneName(int index);
	void ClassifyBones();
};

class CAnimationTrack
{
public:
	CAnimationTrack() { }
	~CAnimationTrack();

public:
    BOOL 							m_bEnable = true;
    float 							m_fSpeed = 1.0f;
    float 							m_fPosition = -ANIMATION_CALLBACK_EPSILON;
	float 							m_fWeight = 1.0f;
	bool m_bFinished{ false };

	int 							m_nAnimationSet = 0; //AnimationSet Index

	int 							m_nType = ANIMATION_TYPE_LOOP; //Once, Loop, PingPong

	int 							m_nCallbackKeys = 0;
	CALLBACKKEY*					m_pCallbackKeys = NULL;

	CAnimationCallbackHandler*		m_pAnimationCallbackHandler = NULL;

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
	CLoadedModelInfo() { }
	~CLoadedModelInfo();

    std::shared_ptr<CGameObject>						m_pModelRootObject = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh					**m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	CAnimationSets					*m_pAnimationSets = NULL;

public:
	void PrepareSkinning();
};

class CAnimationController 
{
public:
	CAnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nAnimationTracks, CLoadedModelInfo *pModel);
	~CAnimationController();

public:
    float 							m_fTime = 0.0f;

    int 							m_nAnimationTracks = 0;
    CAnimationTrack 				*m_pAnimationTracks = NULL;

	CAnimationSets					*m_pAnimationSets = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh					**m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	ID3D12Resource					**m_ppd3dcbSkinningBoneTransforms = NULL; //[SkinnedMeshes]
	XMFLOAT4X4						**m_ppcbxmf4x4MappedSkinningBoneTransforms = NULL; //[SkinnedMeshes]

public:
	void Bone_Info();

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);

	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);

	void SetCallbackKeys(int nAnimationTrack, int nCallbackKeys);
	void SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fTime, void *pData);
	void SetAnimationCallbackHandler(int nAnimationTrack, CAnimationCallbackHandler *pCallbackHandler);

	void AdvanceTime(float fElapsedTime, CGameObject *pRootGameObject);
	void AdvanceTime2(float fElapsedTime, CGameObject *pRootGameObject);

public:
	bool							m_bRootMotion = false;
	std::shared_ptr<CGameObject>				m_pModelRootObject = NULL;

	std::shared_ptr<CGameObject>				m_pRootMotionObject = NULL;
	XMFLOAT3						m_xmf3FirstRootMotionPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	void SetRootMotion(bool bRootMotion) { m_bRootMotion = bRootMotion; }

	virtual void OnRootMotion(CGameObject* pRootGameObject) { }
	virtual void OnAnimationIK(CGameObject* pRootGameObject) { }
};

//==================================================================================

class CHeightMapTerrain;

class CGameObject
{
private:
	std::shared_ptr<CGameObject> m_pChild = nullptr;     // 자식 노드
	std::shared_ptr<CGameObject> m_pSibling = nullptr;   // 형제 노드

	bool Active = true;

public:
	CGameObject* m_pParent = NULL; // 부모 ptr은 shared_ptr X, 순환 참조 발생 방지

	char							m_pstrFrameName[64];

	CMesh* m_pMesh = NULL;
	CAnimationController* m_pSkinnedAnimationController = NULL;

	std::vector<std::shared_ptr<CMaterial>>  Material_list;

	XMFLOAT4X4				m_xmf4x4Parent{};
	XMFLOAT4X4				m_xmf4x4World{};

	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;

	int n_Animation = 0;

public:
	CGameObject(const std::string_view& name = "No_name");
	CGameObject(int nMaterials, const std::string_view& name = "No_name");

	CGameObject(const CGameObject& other);
	CGameObject& operator=(const CGameObject& other);

    virtual ~CGameObject();



	std::shared_ptr<CGameObject> Get_Child();
	std::shared_ptr<CGameObject> Get_Sibling();

	void Set_Active(bool active, bool bIsRoot = true);
	bool Get_Active() { return Active; }

	void SetMesh(CMesh *pMesh);
	void SetShader(CShader *pShader);
	void SetShader(int nMaterial, CShader *pShader);
	void SetMaterial(int nMaterial, CMaterial *pMaterial);

	void Set_Child(std::shared_ptr<CGameObject> pChild);
	
	void Obj_Info(int depth=0);
	void Set_Name(std::string_view name);

	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }

	virtual void OnPrepareAnimate() { }
	virtual void Animate(float fTimeElapsed);

	virtual bool IsVisible(CCamera* pCamera);
	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	virtual void OnLateUpdate() { }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	XMFLOAT3 GetToParentPosition();
	XMFLOAT3 Get_World_Position();
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
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);

	CGameObject *GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent=NULL);

	CGameObject* FindFrame(char *pstrFrameName);

	CTexture *FindReplicatedTexture(_TCHAR *pstrTextureName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

public:
	void FindAndSetSkinnedMesh(CSkinnedMesh **ppSkinnedMeshes, int *pnSkinnedMesh);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	void SetRootMotion(bool bRootMotion) 
	{ 
		if (m_pSkinnedAnimationController) 
			m_pSkinnedAnimationController->SetRootMotion(bRootMotion); 
	}

	void LoadMaterialsFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pParent, FILE *pInFile, CShader *pShader);

	static void LoadAnimationFromFile(FILE *pInFile, CLoadedModelInfo *pLoadedModel);
	static CGameObject *LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CGameObject *pParent, FILE *pInFile, CShader *pShader, int *pnSkinnedMeshes);
	static CLoadedModelInfo *LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char *pstrFileName,  CShader *pShader);

	static CGameObject* Load_Scene_HierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader);
	static CLoadedModelInfo* Load_Scene_File(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader);

	static void PrintFrameInfo(CGameObject* pGameObject, CGameObject *pParent);
	
	virtual std::string  Get_Mesh_Name();


	virtual BoundingOrientedBox* Get_Collider();	
	virtual void Add_Collider(float cube_length);
	virtual void Set_Collider(BoundingOrientedBox* ptr = NULL);


	CAnimationController* GetSkinnedAnimationController() { return m_pSkinnedAnimationController; }

	public:
	// Using CHeightMapTerrain
	virtual int Get_Tile(float x, float z) { return -1; };
	virtual void Get_Active_TileNum_List(std::vector<int>& tile_list) {};
	virtual void Check_Culling(CCamera* pCamera) {};

};

//==================================================================================


class CHeightMapTerrain : public CGameObject
{
private:
	static CTexture* pTerrainBaseTexture;
	static CTexture* pTerrainDetailTexture;
	static Deferred_CTerrainShader* pTerrainShader;
	static CMaterial* pTerrainMaterial;

	static CHeightMapImage* m_pHeightMapImage;  // link height map image for each terrain tile object

private:
	int							m_nWidth;
	int							m_nLength;
	int							m_nDepth;
	XMFLOAT3					m_xmf3Scale;


	int			tile_number = 0;
	XMFLOAT2 Tile_Start_Pos{};
	XMFLOAT2 Area_LT{};
	XMFLOAT2 Area_RB{};

public:
	CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName,
		int start_x_pos, int start_z_pos, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, int Vertex_gap = 1, int nMaxDepth = 1);
	virtual ~CHeightMapTerrain();

	void Set_Tile(int n);

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

	void Reset_Obj_List_Height(std::vector<std::shared_ptr<CGameObject>> obj_list);
	void Reset_Obj_List_Up_Vector(std::vector<std::shared_ptr<CGameObject>> obj_list);
};


class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
};

//==================================================================================

class CAngrybotAnimationController : public CAnimationController
{
public:
	CAngrybotAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel);
	~CAngrybotAnimationController();

	virtual void OnRootMotion(CGameObject* pRootGameObject);
};

class CAngrybotObject : public CGameObject
{
public:
	CAngrybotObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks);
	virtual ~CAngrybotObject();
};

//==================================================================================

class CHumanObject : public CGameObject
{
public:
	CHumanObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CHumanObject();
};

