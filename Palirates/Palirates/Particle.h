#pragma once

#include "stdafx.h"
#include "Object.h"
#include "Mesh.h"


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

#define _WITH_QUERY_DATA_SO_STATISTICS
#define PARTICLE_TYPE_EMITTER		0
#define PARTICLE_TYPE_SHELL			1
#define PARTICLE_TYPE_FLARE01		2
#define PARTICLE_TYPE_FLARE02		3
#define PARTICLE_TYPE_FLARE03		4

#define MAX_PARTICLES				90000

class ParticleMesh : public CMesh
{
public:
	ParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles);
	virtual ~ParticleMesh();

	bool								b_reset = true;
	UINT								m_nStride = 0;
	UINT								m_nMaxParticles = MAX_PARTICLES;
	UINT								m_nCurrentParticles = 0;

	ID3D12Resource* CS_UAV_Buffer = NULL;

	ID3D12Resource* Particle_Init_Buffer = NULL;
	ID3D12Resource* Particle_Draw_Buffer = NULL;
	ID3D12Resource* ParticleUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		Particle_Info_Buffer_View;

	ID3D12Resource* StreamOutputBuffer = NULL;
	D3D12_STREAM_OUTPUT_BUFFER_VIEW		StreamOutputBuffer_View;


	ID3D12Resource* Default_BufferFilled_Size = NULL;
	ID3D12Resource* Upload_BufferFilled_Size = NULL;
	UINT64* Upload_BufferFilled_Size_N = NULL;


#ifdef _WITH_QUERY_DATA_SO_STATISTICS
	ID3D12QueryHeap* m_pd3dSOQueryHeap = NULL;
	ID3D12Resource* m_pd3dSOQueryBuffer = NULL;
	D3D12_QUERY_DATA_SO_STATISTICS* m_pd3dSOQueryDataStatistics = NULL;
#else
	ID3D12Resource* ReadBack_BufferFilled_Size = NULL;
#endif


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
	ParticleMesh* particle_mesh = NULL;
	CMaterial* particle_Material = NULL;

public:
	ParticleObject();
	virtual ~ParticleObject();

	void ReleaseUploadBuffers();

	void Set_Shape(Particle_Shape_Mesh* mesh_ptr) { shape_mesh = mesh_ptr; }
	void Set_Particle_Mesh(ParticleMesh* new_particle_mesh = NULL) { particle_mesh = new_particle_mesh; }
	virtual void SetMesh(CMesh* pMesh = NULL) { m_pMesh = NULL; }



	virtual void Animate(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int progress_n = 0);
	virtual void OnPostRender();

	UINT Get_Particle_Num() { return particle_mesh->Get_Num(); }


};