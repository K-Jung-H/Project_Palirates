#pragma once
#include "Scene.h"
#include "Shader.h"
#include "stdafx.h"
#include "Mesh.h"
#include "Particle.h"


struct CB_Particle_Update_Info
{
	XMFLOAT3 EmitRegionMin;
	float ElapsedTime;

	XMFLOAT3 EmitRegionMax;
	UINT Max_Particle_N;

	XMFLOAT3 Main_Direction;
	float Init_Velocity_Value;

	UINT obb_num;
	XMFLOAT3 padding0;
};

class ParticleShader : public CShader
{
protected:
	CB_Particle_Update_Info m_UpdateInfo = {};

	UINT m_cxThreadGroups;
	UINT m_cyThreadGroups;
	UINT m_czThreadGroups;

	static ID3D12RootSignature* common_ComputeRootSignature;

public:
	int									m_ncomputePipelineStates = 0;
	ID3D12PipelineState** m_ppd3dcomputePipelineStates = NULL;

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
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState);

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
		ID3D12RootSignature* CreateComputeRootSignature(ID3D12Device* pd3dDevice);
	void CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dComputeRootSignature, int nPipelineState = 0);
	void Set_Compute_Pipeline(ID3D12GraphicsCommandList* pd3dCommandList, int index);

	virtual void Update_Compute_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, CB_Particle_Update_Info* update_info);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList);
	void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups);

	static void Set_ComputeRootSignature(ID3D12GraphicsCommandList* pd3dCommandList);

};

class Spread_ParticleShader : public ParticleShader
{
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
};

class Sand_ParticleShader : public ParticleShader
{
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
};

//==============================================================================


class Particle_Manager
{
private:
	std::unordered_map<Particle_Type, ParticleShader*> particle_shader_map;
	CTexture* m_OBBBufferTexture = NULL;
	UINT OBB_num = 0;

	static constexpr UINT THREAD_COUNT = 64;
	static constexpr UINT MAX_OBBS = 5000;

	std::vector<std::shared_ptr<ParticleObject>> destroy_queue;

public:
	std::unordered_map<Particle_Type, std::vector<std::shared_ptr<ParticleObject>>> particle_object_list_map;
	Particle_Manager();
	~Particle_Manager();
	void Create_Particle_Manager(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	void Create_OBB_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const vector<GPU_OBB>& obb_container);
	void Update_OBB_Data_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, const vector<GPU_OBB>& obb_container);

	void Bind_OBB_Data_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void Release_OBB_Data_ShaderVariables();

	void AnimateObjects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);

	void Emit_Particles(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	void Update_and_Extract_Instance_Particles(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);


	void Sync_AfterAnimate(Particle_Type type);
	void Sync_AfterAnimateObjects();

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, Particle_Type type);
	void Render_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);


	std::shared_ptr<ParticleObject>  Add_Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, Particle_Shape_Mesh* particle_shape_mesh, Particle_Format particle_info);

	void Clear_CounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList);
	void Copy_CounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList);

	void Queue_Destroy(std::shared_ptr<ParticleObject> obj) { destroy_queue.push_back(obj); }
	void Process_Destroy_Queue();
};

