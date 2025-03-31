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
#define MAX_PARTICLES				90000

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

	// GPU에서 카운터 값을 복사하는 버퍼
	ID3D12Resource* m_pCounterReadbackBuffer = nullptr;


	// 버퍼 뷰
	D3D12_VERTEX_BUFFER_VIEW m_RenderInstanceVBV = {};

	// 기타
	UINT m_nMaxParticles = MAX_PARTICLES;
	UINT m_nStride = sizeof(Particle_Info);

public:


	// 버퍼 생성 및 해제
	void CreateBuffers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	// 입자 개수 읽기 버퍼
	ID3D12Resource* CreateCounterBuffer(ID3D12Device* pd3dDevice);

	ID3D12Resource* CreateReadbackBuffer(ID3D12Device* pd3dDevice, UINT byteSize = sizeof(UINT));

	void UpdateBuffers(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseBuffers();

	// 렌더링용 VBV 업데이트
	void UpdateRenderInstanceVBV();


	void Post_Update(ID3D12GraphicsCommandList* pd3dCommandList);


	UINT Get_Particle_Max_Num() const { return m_nMaxParticles; }

	void Copy_CounterBuffer_Particle_Info(ID3D12GraphicsCommandList* pd3dCommandList);
	void Copy_CounterBuffer_FreeList(ID3D12GraphicsCommandList* pd3dCommandList);
	void Copy_CounterBuffer_Render_Instance(ID3D12GraphicsCommandList* pd3dCommandList);

	UINT Readback_CounterBuffer_Particle_Info_List();
	UINT Readback_CounterBuffer_FreeList();
	UINT Readback_CounterBuffer_Render_Instance();

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
	Particle* particle_obj = NULL;
	CMaterial* particle_Material = NULL;
	UINT particle_N = 0;

public:
	ParticleObject();
	virtual ~ParticleObject();

	void ReleaseUploadBuffers();

	void Set_Shape(Particle_Shape_Mesh* mesh_ptr) { shape_mesh = mesh_ptr; }
	void Set_Particle_OBJ(Particle* new_particle_obj = NULL) { particle_obj = new_particle_obj; }
	virtual void SetMesh(CMesh* pMesh = NULL) { m_pMesh = NULL; }

	virtual void Update_Compute_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Animate(ID3D12GraphicsCommandList* pd3dCommandList) {};
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int progress_n = 0);
	virtual void OnPostRender();

	UINT Get_Particle_Num() { return particle_N; }
	UINT Test_Func(ID3D12GraphicsCommandList* pd3dCommandList)
	{
		particle_obj->Copy_CounterBuffer(pd3dCommandList);
		return particle_obj->Readback_CounterBuffer();
	}

};