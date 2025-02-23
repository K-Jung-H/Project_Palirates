#pragma once
#include "stdafx.h"
#include "Shader.h"
#include "Mesh.h"

class ParticleVertex;
class ParticleMesh;
class ParticleObject;

//==============================================================================
class ParticleVertex
{
public:
	XMFLOAT3						m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float							m_fLifetime = 0.0f;
	UINT							m_nType = 0;

public:
	ParticleVertex() { }
	~ParticleVertex() { }
};
//==============================================================================

#define PARTICLE_TYPE_EMITTER		0
#define PARTICLE_TYPE_SHELL			1
#define PARTICLE_TYPE_FLARE01		2
#define PARTICLE_TYPE_FLARE02		3
#define PARTICLE_TYPE_FLARE03		4

#define MAX_PARTICLES				9000

class ParticleMesh : public CMesh
{
public:
	ParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles);
	virtual ~ParticleMesh();

	bool								m_bStart = true;
	UINT								m_nStride = 0;
	UINT								m_nMaxParticles = MAX_PARTICLES;
	UINT								m_nCurrentParticles = 0;
	ID3D12Resource* m_pd3dUAVBuffer = NULL;

	ID3D12Resource* m_pd3dParticleBuffer = NULL;
	ID3D12Resource* m_pd3dParticleUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dParticleBufferView;

	ID3D12Resource* m_pd3dStreamOutputBuffer = NULL;
	ID3D12Resource* m_pd3dDrawBuffer = NULL;

	ID3D12Resource* m_pd3dDefaultBufferFilledSize = NULL;
	ID3D12Resource* m_pd3dUploadBufferFilledSize = NULL;
	UINT64* m_pnUploadBufferFilledSize = NULL;

	ID3D12Resource* m_pd3dReadBackBufferFilledSize = NULL;

	D3D12_STREAM_OUTPUT_BUFFER_VIEW		m_d3dStreamOutputBufferView;

	virtual void CreateVertexBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size);
	virtual void CreateStreamOutputBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nMaxParticles);

	virtual void PreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState);
	virtual void PostRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState);

	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num) {}
	virtual void OnPostRender(int nPipelineState);

	ID3D12Resource* CreateUAVBuffer(ID3D12Device* pd3dDevice, size_t bufferSize);

	UINT Get_Num() { return m_nVertices; }

};

//==============================================================================

class Particle_Shape_Mesh : public CStandardMesh
{
protected:
	ID3D12Resource* m_pd3dColorBuffer = NULL;
	ID3D12Resource* m_pd3dColorUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dColorBufferView;

public:
	Particle_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~Particle_Shape_Mesh();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) {};
	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num) {}
};

class Sphere_Shape_Mesh : public Particle_Shape_Mesh
{
public:
	Sphere_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fRadius = 2.0f, int nSlices = 20, int nStacks = 20);
	virtual ~Sphere_Shape_Mesh();

	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num);
};
//==============================================================================

class ParticleObject : public CGameObject
{
private:
	CMesh* shape_mesh = NULL;
	ParticleMesh* particle_mesh = NULL;
	CMaterial* particle_Material = NULL;

public:
	ParticleObject();
	virtual ~ParticleObject();
	
	void ReleaseUploadBuffers();

	void Set_Shape(CMesh* mesh_ptr) { shape_mesh = mesh_ptr; }
	void Set_Particle_Mesh(ParticleMesh* new_particle_mesh = NULL) { particle_mesh = new_particle_mesh; }
	virtual void SetMesh(CMesh* pMesh = NULL) { m_pMesh = NULL; }


	
	virtual void Animate(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int progress_n = 0);
	virtual void OnPostRender();

	UINT Get_Particle_Num() { return particle_mesh->Get_Num(); }


};


//==============================================================================

// CS에 전달할 정보
struct CB_Particle_Update_Info
{
	float ElapsedTime;
	int Particle_N;
};


class ParticleShader : public CShader
{
public:
	ID3D12Resource* Particle_Update_Info = NULL;
	CB_Particle_Update_Info* Mapped_Particle_Update_Info = NULL;


	int									m_ncomputePipelineStates = 0;
	ID3D12PipelineState** m_ppd3dcomputePipelineStates = NULL;
	ID3D12RootSignature* m_pd3dComputeRootSignature = NULL;

public:
	ParticleShader();
	virtual ~ParticleShader();

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(int nPipelineState);
	virtual UINT GetNumRenderTargets(int nPipelineState);
	virtual DXGI_FORMAT GetRTVFormat(int nPipelineState, int nRenderTarget);
	virtual DXGI_FORMAT GetDSVFormat(int nPipelineState);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOuputState(int nPipelineState);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);



	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState = 0);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState);

	D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	ID3D12RootSignature* CreateComputeRootSignature(ID3D12Device* pd3dDevice);
	void CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dComputeRootSignature, int nPipelineState = 0);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext);
	void Set_Compute_Pipeline(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Create_Compute_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Update_Compute_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, UINT particle_count, float fTimeElapsed);
	virtual void Release_Compute_ShaderVariables();
};

//==============================================================================

enum class Particle_Type
{
	sample_1,
	sample_2,
	sample_3,
	etc
};

struct Particle_Info
{
	Particle_Type type = Particle_Type::etc;
	XMFLOAT3 pos{};
	XMFLOAT3 velocity{};
	XMFLOAT3 acceleration{};
	XMFLOAT3 color{};
	XMFLOAT2 size{};
	UINT max_particles = MAX_PARTICLES;

};

class Particle_Manager
{
private:
	std::unordered_map<Particle_Type, ParticleShader*> particle_shader_map;
	std::unordered_map<Particle_Type, std::vector<std::shared_ptr<ParticleObject>>> particle_object_list_map;

	CTexture* m_pRandowmValueTexture = NULL;

public:
	Particle_Manager(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~Particle_Manager();

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	void AnimateObjects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int N, Particle_Type type);
	void Render_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int N);
	void OnPostRender(Particle_Type type);
	void OnPostRender_All();


	void Add_Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMesh* particle_shape_mesh, Particle_Info particle_info);

};

