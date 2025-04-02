#pragma once

#include "stdafx.h"
#include "Object.h"
#include "Mesh.h"
#include "Descriptor_Heap.h"

//==============================================================================

struct Render_Instance
{
	XMFLOAT3 Position;
	XMFLOAT3 Velocity;
	XMFLOAT4 Color;
};

struct Particle_Info
{
	XMFLOAT3 Position;
	float    Lifetime;

	XMFLOAT3 Velocity;
	float    MaxLifetime;

	XMFLOAT3 Acceleration;
	float    Padding1;

	XMFLOAT3 Color;
	float    Padding2;

	XMFLOAT2 Size;
	UINT     Type;
	UINT     Active;  // 0: 죽은 입자
};

//==============================================================================
#define MAX_PARTICLES				9000

enum P_BufferType
{
	BUFFER_COUNTER = 0,
	BUFFER_READBACK = 1,
	BUFFER_COUNTER_RESET = 2
};

class Particle
{
public:
	Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nMaxParticles = MAX_PARTICLES);
	virtual ~Particle();

private:
	// 주요 리소스
	// RWStructuredBuffer<Particle_Info> 0
	// Append/ConsumeStructuredBuffer<uint> 1
	// RWStructuredBuffer<RenderInstance> 2
	CTexture* particle_buffer_texture = NULL;

	ID3D12Resource* Particle_Info_List_counterBuffer = NULL;
	ID3D12Resource* Particle_Info_List_readbackBuffer = NULL;

	ID3D12Resource* FreeList_counterBuffer = NULL;
	ID3D12Resource* FreeList_readbackBuffer = NULL;


	ID3D12Resource* Render_Instance_counterBuffer = NULL;
	ID3D12Resource* Render_Instance_readbackBuffer = NULL;

	ID3D12Resource* CounterResetBuffer = NULL;

	// 버퍼 뷰
	D3D12_VERTEX_BUFFER_VIEW m_RenderInstanceVBV = {};

	// 기타
	UINT m_nMaxParticles = MAX_PARTICLES;
	UINT m_nStride = sizeof(Particle_Info);

public:
	UINT N_Particle_Info_List = 0;
	UINT N_FreeList = 0;
	UINT N_Render_Instance = 0;


public:


	// 버퍼 생성 및 해제
	void Create_Resource_Buffers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	ID3D12Resource* CreateBuffer(ID3D12Device* pd3dDevice, P_BufferType type, UINT byteSize = sizeof(UINT), UINT initialValue = 0);
	void UpdateBuffers(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseBuffers();

	// 렌더링용 VBV 업데이트
	void UpdateRenderInstanceVBV();

	UINT Get_Particle_Max_Num() const { return m_nMaxParticles; }

	void Copy_CounterBuffer_Particle_Info(ID3D12GraphicsCommandList* pd3dCommandList);
	void Copy_CounterBuffer_FreeList(ID3D12GraphicsCommandList* pd3dCommandList);
	void Copy_CounterBuffer_Render_Instance(ID3D12GraphicsCommandList* pd3dCommandList);

	void Copy_CounterBuffer_All(ID3D12GraphicsCommandList* pd3dCommandList)
	{
		Copy_CounterBuffer_Particle_Info(pd3dCommandList);
		Copy_CounterBuffer_FreeList(pd3dCommandList);
		Copy_CounterBuffer_Render_Instance(pd3dCommandList);

	}

	UINT Readback_CounterBuffer_Particle_Info_List();
	UINT Readback_CounterBuffer_FreeList();
	UINT Readback_CounterBuffer_Render_Instance();

	void Readback_All()
	{
		Readback_CounterBuffer_Particle_Info_List();
		Readback_CounterBuffer_FreeList();
		Readback_CounterBuffer_Render_Instance();
	}

	void ResetCounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* counterBuffer);
	void Reset_Instance_CounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList);
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

class Cube_Shape_Mesh : public Particle_Shape_Mesh
{
public:
	Cube_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fSize = 2.0f);
	virtual ~Cube_Shape_Mesh();

	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num);
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
	Particle_Shape_Mesh* shape_mesh = NULL;
	Particle* particle_data = NULL;
	CMaterial* particle_Material = NULL;
	UINT particle_N = 0;

public:
	ParticleObject();
	virtual ~ParticleObject();

	void ReleaseUploadBuffers();

	void Set_Shape(Particle_Shape_Mesh* mesh_ptr) { shape_mesh = mesh_ptr; }
	void Set_Particle_OBJ(Particle* new_particle_obj = NULL) { particle_data = new_particle_obj; }
	virtual void SetMesh(CMesh* pMesh = NULL) { m_pMesh = NULL; }

	virtual void Update_Compute_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Animate(ID3D12GraphicsCommandList* pd3dCommandList) {};

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int progress_n = 0);

	Particle* Get_Particle_Data() { return particle_data; }
	UINT Get_Particle_Num() { return particle_N; }
	UINT Get_Particle_Max_Num() { return particle_data->Get_Particle_Max_Num(); }
	UINT Get_Size_FreeList() { return particle_data->N_FreeList; }
	UINT Get_Size_Particle_Info_List() { return particle_data->N_Particle_Info_List; }
	UINT Get_Size_Render_Instance() { return particle_data->N_Render_Instance; }
};