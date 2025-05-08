#include "stdafx.h"
#include "Particle.h"
#include "Particle_Manager.h"

//==============================================================================

Particle_Shape_Mesh::Particle_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
}
Particle_Shape_Mesh::~Particle_Shape_Mesh()
{
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
}

//==============================================================================

Cube_Shape_Mesh::Cube_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fSize)
	: Particle_Shape_Mesh(pd3dDevice, pd3dCommandList)
{
	XMFLOAT4 color1 = { 0.8f, 0.2f, 0.2f, 1.0f }; // 예시 색상
	XMFLOAT4 color2 = { 0.2f, 0.8f, 0.2f, 1.0f };

	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Cube는 8개의 정점 (8개의 꼭짓점)과 6개의 면을 가짐
	m_nVertices = 8;
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf4Colors = new XMFLOAT4[m_nVertices];

	// 큐브 정점 위치 정의
	float halfSize = fSize / 2.0f;

	m_pxmf3Positions[0] = XMFLOAT3(-halfSize, -halfSize, -halfSize);
	m_pxmf3Positions[1] = XMFLOAT3(halfSize, -halfSize, -halfSize);
	m_pxmf3Positions[2] = XMFLOAT3(halfSize, halfSize, -halfSize);
	m_pxmf3Positions[3] = XMFLOAT3(-halfSize, halfSize, -halfSize);
	m_pxmf3Positions[4] = XMFLOAT3(-halfSize, -halfSize, halfSize);
	m_pxmf3Positions[5] = XMFLOAT3(halfSize, -halfSize, halfSize);
	m_pxmf3Positions[6] = XMFLOAT3(halfSize, halfSize, halfSize);
	m_pxmf3Positions[7] = XMFLOAT3(-halfSize, halfSize, halfSize);

	// 각 정점에 색상 할당 (예시로 두 색상을 번갈아 할당)
	for (int i = 0; i < m_nVertices; ++i)
	{
		m_pxmf4Colors[i] = XMFLOAT4(rand() % 2 == 0 ? color1 : color2);
	}

	// 서브메쉬의 개수는 1개만 사용하도록 설정
	m_nSubMeshes = 1;

	// 서브메쉬 인덱스 개수 및 할당
	int nSubMeshIndices = 36; // 큐브는 6개의 면, 각 면은 2개의 삼각형, 한 면당 6개의 인덱스 => 6 * 6 = 36
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

	m_pnSubSetIndices[0] = nSubMeshIndices; // 첫 번째 서브메쉬의 인덱스 개수 설정
	m_ppnSubSetIndices[0] = new UINT[nSubMeshIndices]; // 첫 번째 서브메쉬의 인덱스 배열 할당

	int k = 0;

	// 큐브의 면에 대한 인덱스를 설정 (각 면을 2개의 삼각형으로 나눔)
	// 아래 6개의 면을 정의 (각 면은 2개의 삼각형으로 나누어 6개의 인덱스를 가짐)
	// 앞면
	m_ppnSubSetIndices[0][k++] = 0; m_ppnSubSetIndices[0][k++] = 1; m_ppnSubSetIndices[0][k++] = 2;
	m_ppnSubSetIndices[0][k++] = 0; m_ppnSubSetIndices[0][k++] = 2; m_ppnSubSetIndices[0][k++] = 3;
	// 뒷면
	m_ppnSubSetIndices[0][k++] = 4; m_ppnSubSetIndices[0][k++] = 5; m_ppnSubSetIndices[0][k++] = 6;
	m_ppnSubSetIndices[0][k++] = 4; m_ppnSubSetIndices[0][k++] = 6; m_ppnSubSetIndices[0][k++] = 7;
	// 왼쪽 면
	m_ppnSubSetIndices[0][k++] = 0; m_ppnSubSetIndices[0][k++] = 4; m_ppnSubSetIndices[0][k++] = 7;
	m_ppnSubSetIndices[0][k++] = 0; m_ppnSubSetIndices[0][k++] = 7; m_ppnSubSetIndices[0][k++] = 3;
	// 오른쪽 면
	m_ppnSubSetIndices[0][k++] = 1; m_ppnSubSetIndices[0][k++] = 5; m_ppnSubSetIndices[0][k++] = 6;
	m_ppnSubSetIndices[0][k++] = 1; m_ppnSubSetIndices[0][k++] = 6; m_ppnSubSetIndices[0][k++] = 2;
	// 위쪽 면
	m_ppnSubSetIndices[0][k++] = 2; m_ppnSubSetIndices[0][k++] = 3; m_ppnSubSetIndices[0][k++] = 7;
	m_ppnSubSetIndices[0][k++] = 2; m_ppnSubSetIndices[0][k++] = 7; m_ppnSubSetIndices[0][k++] = 6;
	// 아래쪽 면
	m_ppnSubSetIndices[0][k++] = 0; m_ppnSubSetIndices[0][k++] = 1; m_ppnSubSetIndices[0][k++] = 5;
	m_ppnSubSetIndices[0][k++] = 0; m_ppnSubSetIndices[0][k++] = 5; m_ppnSubSetIndices[0][k++] = 4;

	// 서브메쉬 인덱스 버퍼 및 업로드 버퍼 생성
	m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
	m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];

	// 첫 번째 서브메쉬의 인덱스 버퍼 생성
	m_ppd3dSubSetIndexBuffers[0] = CreateBufferResource(
		pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * nSubMeshIndices, 
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_ppd3dSubSetIndexUploadBuffers[0]);

	// 서브메쉬 인덱스 버퍼 뷰 설정
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * nSubMeshIndices;

	//===========================================================
	// Position Buffer 생성
	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	//===========================================================
	// Color Buffer 생성
	m_pd3dColorBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4Colors, sizeof(XMFLOAT4) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColorUploadBuffer);

	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
}

Cube_Shape_Mesh::~Cube_Shape_Mesh()
{
}

void Cube_Shape_Mesh::Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->SOSetTargets(0, 1, NULL);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, d3dInstancingBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);

	if (m_ppd3dSubSetIndexBuffers[0] != nullptr)
	{
		D3D12_INDEX_BUFFER_VIEW indexBufferView = m_pd3dSubSetIndexBufferViews[0];
		pd3dCommandList->IASetIndexBuffer(&indexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[0], instance_num, 0, 0, 0);
	}
	else
		pd3dCommandList->DrawInstanced(m_nVertices, instance_num, m_nOffset, 0);




}

//==============================================================================

Sphere_Shape_Mesh::Sphere_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fRadius, int nSlices, int nStacks)
	: Particle_Shape_Mesh(pd3dDevice, pd3dCommandList)
{
	XMFLOAT4 color1 = { 0.5f, 0.5f, 0.8f, 1.0f };
	XMFLOAT4 color2 = { 0.0f, 0.0f, 0.5f, 1.0f };

	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_nVertices = 2 + (nSlices * (nStacks - 1));

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf4Colors = new XMFLOAT4[m_nVertices];


	float fDeltaPhi = float(XM_PI / nStacks);
	float fDeltaTheta = float((2.0f * XM_PI) / nSlices);
	int k = 0;

	//구의 위(북극)를 나타내는 정점이다. 
	m_pxmf3Positions[k++] = XMFLOAT3(0.0f, +fRadius, 0.0f);

	float theta_i, phi_j;
	//원기둥 표면의 정점이다. 
	for (int j = 1; j < nStacks; j++)
	{
		phi_j = fDeltaPhi * j;
		for (int i = 0; i < nSlices; i++)
		{
			theta_i = fDeltaTheta * i;
			m_pxmf3Positions[k++] = XMFLOAT3(
				fRadius * sinf(phi_j) * cosf(theta_i),
				fRadius * cosf(phi_j),
				fRadius * sinf(phi_j) * sinf(theta_i));
		}
	}
	//구의 아래(남극)를 나타내는 정점이다. 
	m_pxmf3Positions[k] = XMFLOAT3(0.0f, -fRadius, 0.0f);

	for (int i = 0; i < m_nVertices; ++i)
	{
		m_pxmf4Colors[i] = XMFLOAT4(rand() % 2 == 0 ? color1 : color2);
	}

	// 서브메쉬의 개수는 1개만 사용하도록 설정
	m_nSubMeshes = 1;

	// 서브메쉬 인덱스 개수 및 할당
	int nSubMeshIndices = (nSlices * 3) * 2 + (nSlices * (nStacks - 2) * 3 * 2);	 // 단일 서브메쉬의 인덱스 개수
	m_pnSubSetIndices = new int[m_nSubMeshes];									 // 서브메쉬 인덱스 개수 저장 배열
	m_ppnSubSetIndices = new UINT * [m_nSubMeshes];							 // 서브메쉬 인덱스 배열 포인터

	m_pnSubSetIndices[0] = nSubMeshIndices; // 첫 번째 서브메쉬의 인덱스 개수 설정
	m_ppnSubSetIndices[0] = new UINT[nSubMeshIndices]; // 첫 번째 서브메쉬의 인덱스 배열 할당


	k = 0;
	//구의 위쪽 원뿔의 표면을 표현하는 삼각형들의 인덱스이다. 
	for (int i = 0; i < nSlices; i++)
	{
		m_ppnSubSetIndices[0][k++] = 0;
		m_ppnSubSetIndices[0][k++] = 1 + ((i + 1) % nSlices);
		m_ppnSubSetIndices[0][k++] = 1 + i;
	}
	//구의 원기둥의 표면을 표현하는 삼각형들의 인덱스이다. 
	for (int j = 0; j < nStacks - 2; j++)
	{
		for (int i = 0; i < nSlices; i++)
		{
			//사각형의 첫 번째 삼각형의 인덱스이다. 
			m_ppnSubSetIndices[0][k++] = 1 + (i + (j * nSlices));
			m_ppnSubSetIndices[0][k++] = 1 + (((i + 1) % nSlices) + (j * nSlices));
			m_ppnSubSetIndices[0][k++] = 1 + (i + ((j + 1) * nSlices));
			//사각형의 두 번째 삼각형의 인덱스이다. 
			m_ppnSubSetIndices[0][k++] = 1 + (i + ((j + 1) * nSlices));
			m_ppnSubSetIndices[0][k++] = 1 + (((i + 1) % nSlices) + (j * nSlices));
			m_ppnSubSetIndices[0][k++] = 1 + (((i + 1) % nSlices) + ((j + 1) * nSlices));
		}
	}
	//구의 아래쪽 원뿔의 표면을 표현하는 삼각형들의 인덱스이다. 
	for (int i = 0; i < nSlices; i++)
	{
		m_ppnSubSetIndices[0][k++] = (m_nVertices - 1);
		m_ppnSubSetIndices[0][k++] = ((m_nVertices - 1) - nSlices) + i;
		m_ppnSubSetIndices[0][k++] = ((m_nVertices - 1) - nSlices) + ((i + 1) % nSlices);
	}

	// 서브메쉬 인덱스 버퍼 및 업로드 버퍼 생성
	m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
	m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];

	// 첫 번째 서브메쉬의 인덱스 버퍼 생성
	m_ppd3dSubSetIndexBuffers[0] = CreateBufferResource(
		pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * nSubMeshIndices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_ppd3dSubSetIndexUploadBuffers[0]);

	// 서브메쉬 인덱스 버퍼 뷰 설정
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * nSubMeshIndices;

	//===========================================================
	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	//===========================================================
	m_pd3dColorBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4Colors, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColorUploadBuffer);

	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;

}

Sphere_Shape_Mesh::~Sphere_Shape_Mesh()
{
}

void Sphere_Shape_Mesh::Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->SOSetTargets(0, 1, NULL);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { m_d3dPositionBufferView, m_d3dColorBufferView, d3dInstancingBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);


	pd3dCommandList->DrawInstanced(m_nVertices, instance_num, m_nOffset, 0);

}


//==============================================================================
Particle::Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCmdList, Particle_Format particle_format)
{
	m_nMaxParticles = particle_format.max_particles;

	Create_Resource_Buffers(pd3dDevice, pd3dCmdList, particle_format);

}

Particle::~Particle()
{

	if (particle_buffer_texture)
	{
		particle_buffer_texture->ReleaseUploadBuffers();
		delete particle_buffer_texture;
		particle_buffer_texture = nullptr;
	}

	ReleaseBuffers();
}

void Particle::Create_Resource_Buffers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, Particle_Format particle_format)
{
	particle_buffer_texture = new CTexture(4, RESOURCE_STRUCTURED_BUFFER, 0, 0, 4, 0, 0, 4, 0);

	Particle_Info* particle_init_data = Init_Particle_Data(particle_format);


	particle_buffer_texture->CreateStructuredBuffer(pd3dDevice, pd3dCommandList, 0, particle_init_data, m_nMaxParticles, sizeof(Particle_Info), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	particle_buffer_texture->CreateStructuredBuffer(pd3dDevice, pd3dCommandList, 1, nullptr, m_nMaxParticles, sizeof(Render_Instance), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	particle_buffer_texture->CreateStructuredBuffer(pd3dDevice, pd3dCommandList, 2, nullptr, 4, sizeof(UINT), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	
	Particle_Info_List_counterBuffer = Create_Control_Buffer(pd3dDevice, BUFFER_COUNTER);
	Particle_Info_List_readbackBuffer = Create_Control_Buffer(pd3dDevice, BUFFER_READBACK);
	
	Render_Instance_counterBuffer = Create_Control_Buffer(pd3dDevice, BUFFER_COUNTER);
	Render_Instance_readbackBuffer = Create_Control_Buffer(pd3dDevice, BUFFER_READBACK);

	Debug_ReadBack_buffer = Create_Control_Buffer(pd3dDevice, BUFFER_READBACK, sizeof(UINT) * 4);


	CounterResetBuffer = Create_Control_Buffer(pd3dDevice, BUFFER_COUNTER_RESET, sizeof(UINT), 0);
	Debug_Reset_Buffer = Create_Control_Buffer(pd3dDevice, BUFFER_COUNTER_RESET, sizeof(UINT) * 4, 0);

	CDescriptor_Heap::CreateStructuredBufferUAV(pd3dDevice, particle_buffer_texture, 0, Particle_Info_List_counterBuffer, 2);
	CDescriptor_Heap::CreateStructuredBufferUAV(pd3dDevice, particle_buffer_texture, 1, Render_Instance_counterBuffer, 3);
	CDescriptor_Heap::CreateStructuredBufferUAV(pd3dDevice, particle_buffer_texture, 2, nullptr, 4);

	{
		ID3D12Resource* Init_Buffer = Create_Control_Buffer(pd3dDevice, BUFFER_COUNTER_RESET, sizeof(UINT), m_nMaxParticles);

		SynchronizeResourceTransition(pd3dCommandList, Particle_Info_List_counterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

		pd3dCommandList->CopyBufferRegion(Particle_Info_List_counterBuffer, 0, Init_Buffer, 0, sizeof(UINT));

		SynchronizeResourceTransition(pd3dCommandList, Particle_Info_List_counterBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	D3D12_GPU_VIRTUAL_ADDRESS RenderInstance_buffer = particle_buffer_texture->GetResource(1)->GetGPUVirtualAddress();

	m_RenderInstanceVBV.BufferLocation = RenderInstance_buffer;
	m_RenderInstanceVBV.StrideInBytes = sizeof(Render_Instance);
	m_RenderInstanceVBV.SizeInBytes = sizeof(Render_Instance) * 1;


	delete particle_init_data;

}

void Particle::UpdateBuffers(ID3D12GraphicsCommandList* pd3dCommandList)
{
	particle_buffer_texture->UpdateComputeUavShaderVariables(pd3dCommandList);
}


D3D12_VERTEX_BUFFER_VIEW Particle::Update_Render_Instance_VBV()
{
	m_RenderInstanceVBV.SizeInBytes = sizeof(Render_Instance) * N_Render_Instance;

	return m_RenderInstanceVBV;
}


void Particle::ReleaseBuffers()
{
	if (Particle_Info_List_counterBuffer) { Particle_Info_List_counterBuffer->Release(); Particle_Info_List_counterBuffer = nullptr; }
	if (Particle_Info_List_readbackBuffer) { Particle_Info_List_readbackBuffer->Release(); Particle_Info_List_readbackBuffer = nullptr; }

	if (Render_Instance_counterBuffer) { Render_Instance_counterBuffer->Release(); Render_Instance_counterBuffer = nullptr; }
	if (Render_Instance_readbackBuffer) { Render_Instance_readbackBuffer->Release(); Render_Instance_readbackBuffer = nullptr; }

	if (Debug_ReadBack_buffer) { Debug_ReadBack_buffer->Release(); Debug_ReadBack_buffer = nullptr; }

	if (CounterResetBuffer) { CounterResetBuffer->Release(); CounterResetBuffer = nullptr; }
	if (Debug_Reset_Buffer) { Debug_Reset_Buffer->Release(); Debug_Reset_Buffer = nullptr; }
}

Particle_Info* Particle::Init_Particle_Data(const Particle_Format& particle_format)
{ 
	Particle_Info* particle_info = new Particle_Info[m_nMaxParticles];
	for (UINT i = 0; i < m_nMaxParticles; ++i)
	{
		particle_info[i].Active = 0;
		particle_info[i].Type = particle_format.particle_type;

		particle_info[i].MaxLifetime = particle_format.MaxLifetime;
		particle_info[i].Lifetime = 0.0f;

		particle_info[i].Position = XMFLOAT3{};
		particle_info[i].Velocity = XMFLOAT3{};
		particle_info[i].Acceleration = particle_format.acceleration;
		particle_info[i].Rotate_Value = 0.0f;

		particle_info[i].Color = particle_format.color;
		particle_info[i].Size = particle_format.size;

		particle_info[i].EmitFaceIndex = particle_format.EmitFaceIndex;
	}

	return particle_info;
}

void Particle::Copy_CounterBuffer_Particle_Info(ID3D12GraphicsCommandList* pd3dCommandList)
{
	// 상태를 COPY_DEST로 전환
	SynchronizeResourceTransition(pd3dCommandList, Particle_Info_List_readbackBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	// 데이터를 복사
	pd3dCommandList->CopyBufferRegion(Particle_Info_List_readbackBuffer, 0, Particle_Info_List_counterBuffer, 0, sizeof(UINT));

	// 상태를 GENERIC_READ로 전환
	SynchronizeResourceTransition(pd3dCommandList, Particle_Info_List_readbackBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
}

void Particle::Copy_CounterBuffer_Render_Instance(ID3D12GraphicsCommandList* pd3dCommandList)
{
	// 상태를 COPY_DEST로 전환
	SynchronizeResourceTransition(pd3dCommandList, Render_Instance_readbackBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	// 데이터를 복사
	pd3dCommandList->CopyBufferRegion(Render_Instance_readbackBuffer, 0, Render_Instance_counterBuffer, 0, sizeof(UINT));

	// 상태를 GENERIC_READ로 전환
	SynchronizeResourceTransition(pd3dCommandList, Render_Instance_readbackBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
}

void Particle::Copy_DebugBuffer(ID3D12GraphicsCommandList* pd3dCommandList)
{
	// 상태를 COPY_DEST로 전환
	SynchronizeResourceTransition(pd3dCommandList, Debug_ReadBack_buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	pd3dCommandList->CopyBufferRegion(Debug_ReadBack_buffer, 0, particle_buffer_texture->GetResource(2), 0, sizeof(UINT) * 4);

	// 상태를 GENERIC_READ로 전환
	SynchronizeResourceTransition(pd3dCommandList, Debug_ReadBack_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
}


UINT Particle::Readback_CounterBuffer_Particle_Info_List()
{
	if (!Particle_Info_List_readbackBuffer)
		return 0;

	UINT count = 0;
	void* pData = nullptr;
	D3D12_RANGE range = { 0, sizeof(UINT) };

	if (SUCCEEDED(Particle_Info_List_readbackBuffer->Map(0, &range, &pData)) && pData)
	{
		count = *reinterpret_cast<UINT*>(pData);
		Particle_Info_List_readbackBuffer->Unmap(0, nullptr);
	}

	N_Particle_Info_List = count;
	return count;
}

UINT Particle::Readback_CounterBuffer_Render_Instance()
{
	if (!Render_Instance_readbackBuffer)
		return 0;

	UINT count = 0;
	void* pData = nullptr;
	D3D12_RANGE range = { 0, sizeof(UINT) };

	if (SUCCEEDED(Render_Instance_readbackBuffer->Map(0, &range, &pData)) && pData)
	{
		count = *reinterpret_cast<UINT*>(pData);
		Render_Instance_readbackBuffer->Unmap(0, nullptr);
	}

	N_Render_Instance = count;
	return count;
}

UINT Particle::Readback_DebugBuffer()
{
	if (!Debug_ReadBack_buffer)
		return 0;

	UINT* debugData = nullptr;
	D3D12_RANGE readRange = { 0, sizeof(UINT) * 4 }; // 4개의 uint (16바이트)

	if (SUCCEEDED(Debug_ReadBack_buffer->Map(0, &readRange, reinterpret_cast<void**>(&debugData))) && debugData)
	{
		//UINT emit_cs_called = debugData[0]; // Emit에서 InterlockedAdd된 값
		//UINT emitCount = debugData[1]; // Emit 조건 최대값 (예: Max_Particle)
		//UINT killCount = debugData[2]; // Kill 처리된 파티클 수
		//UINT renderCount = debugData[3]; // 실제 RenderInstanceBuffer에 Append된 수

		//DebugOutput("\n[GPU Debug Info]\n");
		//DebugOutput("------------------------------\n");
		//DebugOutput(" Emit_CS_Called				 : " + to_string(emit_cs_called) + "\n");
		//DebugOutput(" EmitCount	 : " + to_string(emitCount) + "\n");
		//DebugOutput(" KillCount				 : " + to_string(killCount) + "\n");
		//DebugOutput(" RenderCount		     : " + to_string(renderCount) + "\n");
		//DebugOutput("------------------------------\n");

		Debug_ReadBack_buffer->Unmap(0, nullptr);

		return 0; 
	}
	else
	{
		DebugOutput(" Failed to map debug readback buffer.\n");
	}

	return 0;
}



void Particle::ResetCounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* counterBuffer)
{
	if (!counterBuffer)
	{
		OutputDebugString(L"[ResetCounterBuffer] Null counter buffer\n");
		return;
	}

	SynchronizeResourceTransition(pd3dCommandList, counterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

	pd3dCommandList->CopyBufferRegion(counterBuffer, 0, CounterResetBuffer, 0, sizeof(UINT));

	SynchronizeResourceTransition(pd3dCommandList, counterBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void Particle::Reset_Particle_Info_List_CounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList)
{
	ResetCounterBuffer(pd3dCommandList, Particle_Info_List_counterBuffer);
}

void Particle::Reset_Instance_CounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList)
{
	ResetCounterBuffer(pd3dCommandList, Render_Instance_counterBuffer);
}

void Particle::Reset_Debug_Buffer(ID3D12GraphicsCommandList* pd3dCommandList)
{
	ID3D12Resource* Debug_buffer = particle_buffer_texture->GetResource(2);

	SynchronizeResourceTransition(pd3dCommandList, Debug_buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

	pd3dCommandList->CopyBufferRegion(Debug_buffer, 0, Debug_Reset_Buffer, 0, sizeof(UINT) * 4);

	SynchronizeResourceTransition(pd3dCommandList, Debug_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}


//==============================================================================


ParticleObject::ParticleObject() : CGameObject(1)
{
	m_pMesh = NULL;

	area_xyz = XMFLOAT3{ 1000.0f,100.0f,1000.0f };
	center = XMFLOAT3{ 0.0f,0.0f ,0.0f };
}

ParticleObject::~ParticleObject()
{
	delete particle_data;

}

void ParticleObject::ReleaseUploadBuffers()
{
	CGameObject::ReleaseUploadBuffers();
}

void ParticleObject::Init_Info(Particle_Format particle_info)
{
	Set_Center(particle_info.center);
	Set_Area(particle_info.area_xyz);
	Set_Main_Direction(particle_info.main_direction);
	
	Init_Velocity_Value = particle_info.init_velocity_value;
 }

void ParticleObject::Set_Main_Direction(const XMFLOAT3& input)
{
	XMVECTOR dirVec = XMLoadFloat3(&input);
	if (XMVector3Equal(dirVec, XMVectorZero()))
	{
		direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
		return;
	}

	dirVec = XMVector3Normalize(dirVec);
	XMStoreFloat3(&direction, dirVec);
}

XMFLOAT3 ParticleObject::Get_Main_Direction()
{
	XMVECTOR dirVec = XMLoadFloat3(&direction);

	if (XMVector3Equal(dirVec, XMVectorZero()))
		return XMFLOAT3(0.0f, 1.0f, 0.0f);
	
	XMVECTOR normalizedDir = XMVector3Normalize(dirVec);
	XMFLOAT3 result;
	XMStoreFloat3(&result, normalizedDir);

	return result;
}

void ParticleObject::Update_Compute_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	particle_data->UpdateBuffers(pd3dCommandList);
}

void ParticleObject::Animate(ID3D12GraphicsCommandList* pd3dCommandList)
{
}


void ParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	// 메시기반 인스턴싱 하기

	D3D12_VERTEX_BUFFER_VIEW Particle_Instancing_BufferView = particle_data->Update_Render_Instance_VBV();
	UINT instance_num = particle_data->N_Render_Instance;

	if (instance_num == 0)
		return;

	 
	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);


	if (shape_mesh)
		shape_mesh->Instancing_Render(pd3dCommandList, Particle_Instancing_BufferView, instance_num); 
}

void ParticleObject::Add_Destroy_Queue() 
{
	auto particlePtr = std::dynamic_pointer_cast<ParticleObject>(shared_from_this());

	if (particlePtr)
		owner_manager->Queue_Destroy(particlePtr);
}
