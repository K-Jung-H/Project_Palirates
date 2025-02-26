#include "stdafx.h"
#include "Particle.h"

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
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_ppd3dSubSetIndexUploadBuffers[0]);

	// 서브메쉬 인덱스 버퍼 뷰 설정
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * nSubMeshIndices;

	//===========================================================
	// Position Buffer 생성
	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	//===========================================================
	// Color Buffer 생성
	m_pd3dColorBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4Colors, sizeof(XMFLOAT4) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColorUploadBuffer);

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

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { m_d3dPositionBufferView, m_d3dColorBufferView, d3dInstancingBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);

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
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_ppd3dSubSetIndexUploadBuffers[0]);

	// 서브메쉬 인덱스 버퍼 뷰 설정
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * nSubMeshIndices;

	//===========================================================
	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	//===========================================================
	m_pd3dColorBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4Colors, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColorUploadBuffer);

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


ParticleMesh::ParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles) : CMesh(pd3dDevice, pd3dCommandList)
{
	CreateVertexBuffer(pd3dDevice, pd3dCommandList, xmf3Position, xmf3Velocity, fLifetime, xmf3Acceleration, xmf3Color, xmf2Size);
	CreateStreamOutputBuffer(pd3dDevice, pd3dCommandList, nMaxParticles);
}

void ParticleMesh::CreateVertexBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size)
{
	m_nVertices = 1;
	m_nStride = sizeof(ParticleVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

	ParticleVertex pVertices[1];

	pVertices[0].m_xmf3Position = xmf3Position;
	pVertices[0].m_xmf3Velocity = xmf3Velocity;
	pVertices[0].m_fLifetime = fLifetime;
	pVertices[0].m_nType = PARTICLE_TYPE_EMITTER;

	Particle_Init_Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &ParticleUploadBuffer);

	Particle_Info_Buffer_View.BufferLocation = Particle_Init_Buffer->GetGPUVirtualAddress();
	Particle_Info_Buffer_View.StrideInBytes = m_nStride;
	Particle_Info_Buffer_View.SizeInBytes = m_nStride * m_nVertices;

	CS_UAV_Buffer = CreateUAVBuffer(pd3dDevice, (m_nStride * m_nMaxParticles));
	::SynchronizeResourceTransition(pd3dCommandList, CS_UAV_Buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

}

void ParticleMesh::CreateStreamOutputBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nMaxParticles)
{
	m_nMaxParticles = nMaxParticles;

	StreamOutputBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, (m_nStride * m_nMaxParticles), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);
	Particle_Draw_Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, (m_nStride * m_nMaxParticles), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	UINT64 nBufferFilledSize = 0;
	Default_BufferFilled_Size = ::CreateBufferResource(pd3dDevice, pd3dCommandList, &nBufferFilledSize, sizeof(UINT64), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);

	Upload_BufferFilled_Size = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
	Upload_BufferFilled_Size->Map(0, NULL, (void**)&Upload_BufferFilled_Size_N);

#ifdef _WITH_QUERY_DATA_SO_STATISTICS
	D3D12_QUERY_HEAP_DESC d3dQueryHeapDesc = { };
	d3dQueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_SO_STATISTICS;
	d3dQueryHeapDesc.Count = 1;
	d3dQueryHeapDesc.NodeMask = 0;
	pd3dDevice->CreateQueryHeap(&d3dQueryHeapDesc, __uuidof(ID3D12QueryHeap), (void**)&m_pd3dSOQueryHeap);

	m_pd3dSOQueryBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(D3D12_QUERY_DATA_SO_STATISTICS), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
#else
	ReadBack_BufferFilled_Size = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
#endif
}

ParticleMesh::~ParticleMesh()
{
	if (StreamOutputBuffer) StreamOutputBuffer->Release();
	if (Particle_Draw_Buffer) Particle_Draw_Buffer->Release();
	if (Default_BufferFilled_Size) Default_BufferFilled_Size->Release();
	if (Upload_BufferFilled_Size) Upload_BufferFilled_Size->Release();

#ifdef _WITH_QUERY_DATA_SO_STATISTICS
	if (m_pd3dSOQueryBuffer) m_pd3dSOQueryBuffer->Release();
	if (m_pd3dSOQueryHeap) m_pd3dSOQueryHeap->Release();
#else
	if (ReadBack_BufferFilled_Size) ReadBack_BufferFilled_Size->Release();
#endif
}

void ParticleMesh::PreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (nPipelineState == 0)
	{
		if (b_reset)
		{
			b_reset = false;

			m_nVertices = 1;

			Particle_Info_Buffer_View.BufferLocation = Particle_Init_Buffer->GetGPUVirtualAddress();
			Particle_Info_Buffer_View.StrideInBytes = m_nStride;
			Particle_Info_Buffer_View.SizeInBytes = m_nStride * m_nVertices;
		}
		else
		{
			Particle_Info_Buffer_View.BufferLocation = Particle_Draw_Buffer->GetGPUVirtualAddress();
			Particle_Info_Buffer_View.StrideInBytes = m_nStride;
			Particle_Info_Buffer_View.SizeInBytes = m_nStride * m_nVertices;
		}


		StreamOutputBuffer_View.BufferLocation = StreamOutputBuffer->GetGPUVirtualAddress();
		StreamOutputBuffer_View.SizeInBytes = m_nStride * m_nMaxParticles;
		StreamOutputBuffer_View.BufferFilledSizeLocation = Default_BufferFilled_Size->GetGPUVirtualAddress();

		*Upload_BufferFilled_Size_N = 0;
		//*Upload_BufferFilled_Size_N = m_nStride * m_nVertices;

		::SynchronizeResourceTransition(pd3dCommandList, Default_BufferFilled_Size, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_DEST);
		pd3dCommandList->CopyResource(Default_BufferFilled_Size, Upload_BufferFilled_Size);
		::SynchronizeResourceTransition(pd3dCommandList, Default_BufferFilled_Size, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT);
	}
	else if (nPipelineState == 1)
	{
		::SynchronizeResourceTransition(pd3dCommandList, Particle_Draw_Buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		::SynchronizeResourceTransition(pd3dCommandList, StreamOutputBuffer, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
		pd3dCommandList->CopyResource(Particle_Draw_Buffer, StreamOutputBuffer);
		::SynchronizeResourceTransition(pd3dCommandList, StreamOutputBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_STREAM_OUT);
		::SynchronizeResourceTransition(pd3dCommandList, Particle_Draw_Buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);


		Particle_Info_Buffer_View.BufferLocation = Particle_Draw_Buffer->GetGPUVirtualAddress();
		Particle_Info_Buffer_View.StrideInBytes = m_nStride;
		Particle_Info_Buffer_View.SizeInBytes = m_nStride * m_nVertices;


	}
}

void ParticleMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (nPipelineState == 0)
	{
		D3D12_STREAM_OUTPUT_BUFFER_VIEW pStreamOutputBufferViews[1] = { StreamOutputBuffer_View };
		pd3dCommandList->SOSetTargets(0, 1, pStreamOutputBufferViews);

#ifdef _WITH_QUERY_DATA_SO_STATISTICS
		pd3dCommandList->BeginQuery(m_pd3dSOQueryHeap, D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0);
#endif
		pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
		pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &Particle_Info_Buffer_View);
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);



#ifdef _WITH_QUERY_DATA_SO_STATISTICS
		pd3dCommandList->EndQuery(m_pd3dSOQueryHeap, D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0);
		pd3dCommandList->ResolveQueryData(m_pd3dSOQueryHeap, D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0, 1, m_pd3dSOQueryBuffer, 0);

#else
		::SynchronizeResourceTransition(pd3dCommandList, Default_BufferFilled_Size, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
		pd3dCommandList->CopyResource(ReadBack_BufferFilled_Size, Default_BufferFilled_Size);
		::SynchronizeResourceTransition(pd3dCommandList, Default_BufferFilled_Size, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_STREAM_OUT);

#endif

	}
}


void ParticleMesh::PostRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
}

#define _WITH_DEBUG_STREAM_OUTPUT_VERTICES

void ParticleMesh::OnPostRender(int nPipelineState)
{
	if (nPipelineState == 0)
	{
#ifdef _WITH_QUERY_DATA_SO_STATISTICS
		D3D12_RANGE d3dReadRange = { 0, 0 };
		UINT8* pBufferDataBegin = NULL;
		m_pd3dSOQueryBuffer->Map(0, &d3dReadRange, (void**)&m_pd3dSOQueryDataStatistics);
		if (m_pd3dSOQueryDataStatistics)
			m_nVertices = (UINT)m_pd3dSOQueryDataStatistics->NumPrimitivesWritten;
		m_pd3dSOQueryBuffer->Unmap(0, NULL);
#else
		UINT64* pnReadBackBufferFilledSize = NULL;
		ReadBack_BufferFilled_Size->Map(0, NULL, (void**)&pnReadBackBufferFilledSize);
		m_nVertices = UINT(*pnReadBackBufferFilledSize) / m_nStride;
		ReadBack_BufferFilled_Size->Unmap(0, NULL);
#endif


		m_nCurrentParticles = m_nVertices;
#ifdef _WITH_DEBUG_STREAM_OUTPUT_VERTICES
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Stream Output Vertices = %d\n"), m_nVertices);
		OutputDebugString(pstrDebug);
#endif
		if ((m_nVertices == 0) || (m_nVertices >= MAX_PARTICLES))
			b_reset = true;
	}
}

ID3D12Resource* ParticleMesh::CreateUAVBuffer(ID3D12Device* pd3dDevice, size_t bufferSize)
{
	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	bufferDesc.Width = bufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // UAV을 사용할 수 있도록 설정

	// Create the buffer with D3D12_HEAP_TYPE_DEFAULT and UAV state
	ID3D12Resource* pd3dBuffer = nullptr;
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	HRESULT hr = pd3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pd3dBuffer));

	if (FAILED(hr))
	{
		HRESULT removeReason = pd3dDevice->GetDeviceRemovedReason();
		return nullptr;
	}

	return pd3dBuffer;
}
//==============================================================================


ParticleObject::ParticleObject() : CGameObject(1)
{
}

ParticleObject::~ParticleObject()
{
}

void ParticleObject::ReleaseUploadBuffers()
{
	CGameObject::ReleaseUploadBuffers();
}

void ParticleObject::Animate(ID3D12GraphicsCommandList* pd3dCommandList)
{
	// Draw buffer를 COPY_SOURCE 상태로 전환하고, UAV 버퍼를 COPY_DEST 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->Particle_Draw_Buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_SOURCE);

	// Draw buffer에서 UAV 버퍼로 복사
	pd3dCommandList->CopyResource(particle_mesh->CS_UAV_Buffer, particle_mesh->Particle_Draw_Buffer);

	// UAV 버퍼를 UNORDERED_ACCESS 상태로, Draw buffer는 COPY_DEST 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->CS_UAV_Buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// Draw buffer에 대해 UAV 뷰를 설정하고, Compute Shader 실행
	pd3dCommandList->SetComputeRootUnorderedAccessView(1, particle_mesh->CS_UAV_Buffer->GetGPUVirtualAddress());
	pd3dCommandList->Dispatch(1, 1, 1);

	// UAV 버퍼를 COPY_SOURCE 상태로 전환하고, Draw buffer를 COPY_DEST 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->Particle_Draw_Buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->CS_UAV_Buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	// UAV 버퍼에서 Draw buffer로 복사
	pd3dCommandList->CopyResource(particle_mesh->Particle_Draw_Buffer, particle_mesh->CS_UAV_Buffer);

	// Draw buffer를 다시 CONSTANT_BUFFER 상태로, UAV 버퍼는 COPY_DEST 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->Particle_Draw_Buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->CS_UAV_Buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
}

void ParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int progress)
{

	if (progress == 0)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (particle_mesh)
		{
			particle_mesh->PreRender(pd3dCommandList, 0); //Stream Output
			particle_mesh->Render(pd3dCommandList, 0); //Stream Output
		}

	}
	else if (progress == 1)
	{
		if (particle_mesh)
			particle_mesh->PreRender(pd3dCommandList, 1); //Draw

		if (shape_mesh)
			shape_mesh->Instancing_Render(pd3dCommandList, particle_mesh->Particle_Info_Buffer_View, particle_mesh->Get_Num()); //Draw

	}
}




void ParticleObject::OnPostRender()
{
	if (particle_mesh)
		particle_mesh->OnPostRender(0); //Read Stream Output Buffer Filled Size
}
