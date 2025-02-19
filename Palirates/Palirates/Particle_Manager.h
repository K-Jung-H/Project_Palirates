#pragma once
#include "stdafx.h"
#include "Shader.h"
#include "Mesh.h"

class CParticleVertex;
class CParticleMesh;
class ParticleObject;

//==============================================================================
class CParticleVertex
{
public:
	XMFLOAT3						m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float							m_fLifetime = 0.0f;
	UINT							m_nType = 0;

public:
	CParticleVertex() { }
	~CParticleVertex() { }
};
//==============================================================================
#define MAX_PARTICLES				9000

class CParticleMesh : public CMesh
{
public:
	CParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles);
	virtual ~CParticleMesh();

	bool								m_bStart = true;
	UINT								m_nStride = 0;
	UINT								m_nMaxParticles = MAX_PARTICLES;

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

	virtual void OnPostRender(int nPipelineState);

	ID3D12Resource* CreateUAVBuffer(ID3D12Device* pd3dDevice, size_t bufferSize);
};

//==============================================================================
class CParticleObject : public CGameObject
{
private:
	CMesh* particle_shape_mesh = NULL;
public:
	CParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CParticleObject();

	void ReleaseUploadBuffers();

	virtual void SetShape(CMesh* mesh_ptr) { particle_shape_mesh = mesh_ptr; }
	virtual void Animate(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int N = 0);
	virtual void OnPostRender();
	UINT Get_Num() { return ((CParticleMesh*)m_pMesh)->GetNum(); }
};

//==============================================================================
struct CB_Particle_Update_Info
{
	float gfElapsedTime;
	int Particle_N;
};


class CParticleShader : public CShader
{
public:
	std::vector<ParticleObject*> particle_list;


	// Particle_Object가 갖도록 하기
	CMesh* particle_shape_mesh = NULL;
	CParticleMesh* particle_mesh = NULL;
	CTexture* m_pRandowmValueTexture = NULL;
	CMaterial* particle_Material;


	ID3D12Resource* m_pd3dcbParticlenfo = NULL;
	CB_Particle_Update_Info* m_pcbMappedParticleInfo = NULL;


	int									m_ncomputePipelineStates = 0;
	ID3D12PipelineState** m_ppd3dcomputePipelineStates = NULL;
	ID3D12RootSignature* m_pd3dComputeRootSignature = NULL;

public:
	CParticleShader();
	virtual ~CParticleShader();

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(int nPipelineState);
	virtual UINT GetNumRenderTargets(int nPipelineState);
	virtual DXGI_FORMAT GetRTVFormat(int nPipelineState, int nRenderTarget);
	virtual DXGI_FORMAT GetDSVFormat(int nPipelineState);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOuputState(int nPipelineState);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState = 0);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState);

	D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	ID3D12RootSignature* CreateComputeRootSignature(ID3D12Device* pd3dDevice);
	void CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dComputeRootSignature, int nPipelineState = 0);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext);

	virtual void AnimateObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int N);
	void OnPostRender();

	void Add_Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 pos, XMFLOAT3 velocity, XMFLOAT3 acceleration, XMFLOAT3 color, XMFLOAT2 size, UINT max_particles);
};

//==============================================================================
class Particle_Manager
{
	std::vector<CParticleShader*> particle_shader_list; // 파티클 움직임, GS, SO 타입에 따라 사용해야 할 셰이더가 달라질 것
};

