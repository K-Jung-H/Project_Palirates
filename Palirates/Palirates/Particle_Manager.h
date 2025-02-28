#pragma once
#include "Scene.h"
#include "Shader.h"
#include "stdafx.h"
#include "Mesh.h"
#include "Particle.h"


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


	void Add_Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, Particle_Shape_Mesh* particle_shape_mesh, Particle_Info particle_info);

};

