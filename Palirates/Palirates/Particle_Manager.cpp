#include "stdafx.h"
#include "Particle_Manager.h"
#include <sstream>

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

	m_pd3dParticleBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dParticleUploadBuffer);

	m_d3dParticleBufferView.BufferLocation = m_pd3dParticleBuffer->GetGPUVirtualAddress();
	m_d3dParticleBufferView.StrideInBytes = m_nStride;
	m_d3dParticleBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dUAVBuffer = CreateUAVBuffer(pd3dDevice, (m_nStride * m_nMaxParticles));
}

void ParticleMesh::CreateStreamOutputBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nMaxParticles)
{
	m_nMaxParticles = nMaxParticles;

	m_pd3dStreamOutputBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, (m_nStride * m_nMaxParticles), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);
	m_pd3dDrawBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, (m_nStride * m_nMaxParticles), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	UINT64 nBufferFilledSize = 0;
	m_pd3dDefaultBufferFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, &nBufferFilledSize, sizeof(UINT64), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);

	m_pd3dUploadBufferFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
	m_pd3dUploadBufferFilledSize->Map(0, NULL, (void**)&m_pnUploadBufferFilledSize);

	m_pd3dReadBackBufferFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);

}

ParticleMesh::~ParticleMesh()
{
	if (m_pd3dStreamOutputBuffer) m_pd3dStreamOutputBuffer->Release();
	if (m_pd3dDrawBuffer) m_pd3dDrawBuffer->Release();
	if (m_pd3dDefaultBufferFilledSize) m_pd3dDefaultBufferFilledSize->Release();
	if (m_pd3dUploadBufferFilledSize) m_pd3dUploadBufferFilledSize->Release();

#ifdef _WITH_QUERY_DATA_SO_STATISTICS
	if (m_pd3dSOQueryBuffer) m_pd3dSOQueryBuffer->Release();
	if (m_pd3dSOQueryHeap) m_pd3dSOQueryHeap->Release();
#else
	if (m_pd3dReadBackBufferFilledSize) m_pd3dReadBackBufferFilledSize->Release();
#endif
}

void ParticleMesh::PreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (nPipelineState == 0)
	{
		if (m_bStart)
		{
			m_bStart = false;

			m_nVertices = 1;

			m_d3dParticleBufferView.BufferLocation = m_pd3dParticleBuffer->GetGPUVirtualAddress();
			m_d3dParticleBufferView.StrideInBytes = m_nStride;
			m_d3dParticleBufferView.SizeInBytes = m_nStride * m_nVertices;
		}
		else
		{
			m_d3dParticleBufferView.BufferLocation = m_pd3dDrawBuffer->GetGPUVirtualAddress();
			m_d3dParticleBufferView.StrideInBytes = m_nStride;
			m_d3dParticleBufferView.SizeInBytes = m_nStride * m_nVertices;
		}
		m_d3dStreamOutputBufferView.BufferLocation = m_pd3dStreamOutputBuffer->GetGPUVirtualAddress();
		m_d3dStreamOutputBufferView.SizeInBytes = m_nStride * m_nMaxParticles;
		m_d3dStreamOutputBufferView.BufferFilledSizeLocation = m_pd3dDefaultBufferFilledSize->GetGPUVirtualAddress();

		*m_pnUploadBufferFilledSize = 0;

		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dDefaultBufferFilledSize, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_DEST);
		pd3dCommandList->CopyResource(m_pd3dDefaultBufferFilledSize, m_pd3dUploadBufferFilledSize);
		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dDefaultBufferFilledSize, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT);
	}
	else if (nPipelineState == 1)
	{
		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dStreamOutputBuffer, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dDrawBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_STREAM_OUT);

		::SwapResourcePointer(&m_pd3dDrawBuffer, &m_pd3dStreamOutputBuffer);

		m_d3dParticleBufferView.BufferLocation = m_pd3dDrawBuffer->GetGPUVirtualAddress();
		m_d3dParticleBufferView.StrideInBytes = m_nStride;
		m_d3dParticleBufferView.SizeInBytes = m_nStride * m_nVertices;
	}
}

void ParticleMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (nPipelineState == 0)
	{
		D3D12_STREAM_OUTPUT_BUFFER_VIEW pStreamOutputBufferViews[1] = { m_d3dStreamOutputBufferView };
		pd3dCommandList->SOSetTargets(0, 1, pStreamOutputBufferViews);

		//Stream Output to m_pd3dStreamOutputBuffer
		pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
		pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dParticleBufferView);
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);


		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dDefaultBufferFilledSize, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
		pd3dCommandList->CopyResource(m_pd3dReadBackBufferFilledSize, m_pd3dDefaultBufferFilledSize);
		::SynchronizeResourceTransition(pd3dCommandList, m_pd3dDefaultBufferFilledSize, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_STREAM_OUT);

	}
	else if (nPipelineState == 1)
	{
		pd3dCommandList->SOSetTargets(0, 1, NULL);

		//Render m_pd3dDrawBuffer 
		pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
		pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dParticleBufferView);
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);

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
		UINT64* pnReadBackBufferFilledSize = NULL;
		m_pd3dReadBackBufferFilledSize->Map(0, NULL, (void**)&pnReadBackBufferFilledSize);
		m_nVertices = UINT(*pnReadBackBufferFilledSize) / m_nStride;
		m_pd3dReadBackBufferFilledSize->Unmap(0, NULL);

		m_nCurrentParticles = m_nVertices;
#ifdef _WITH_DEBUG_STREAM_OUTPUT_VERTICES
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Stream Output Vertices = %d\n"), m_nVertices);
		OutputDebugString(pstrDebug);
#endif
		if ((m_nVertices == 0) || (m_nVertices >= MAX_PARTICLES))
			m_bStart = true;
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

	HRESULT hr = pd3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pd3dBuffer));

	if (FAILED(hr))
	{
		HRESULT removeReason = pd3dDevice->GetDeviceRemovedReason();
		std::stringstream ss;
		ss << "CreateCommittedResource failed with HRESULT: 0x" << std::hex << hr;
		ss << ", Device Removed Reason: 0x" << std::hex << removeReason;
		DebugOutput(ss.str());		
		return nullptr;
	}

	return pd3dBuffer;
}
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
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dStreamOutputBuffer, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dUAVBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

	// Draw buffer에서 UAV 버퍼로 복사
	pd3dCommandList->CopyResource(particle_mesh->m_pd3dUAVBuffer, particle_mesh->m_pd3dStreamOutputBuffer);

	// UAV 버퍼를 UNORDERED_ACCESS 상태로, Draw buffer는 COPY_DEST 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dUAVBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dStreamOutputBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

	// Draw buffer에 대해 UAV 뷰를 설정하고, Compute Shader 실행
	pd3dCommandList->SetComputeRootUnorderedAccessView(1, particle_mesh->m_pd3dStreamOutputBuffer->GetGPUVirtualAddress());
	pd3dCommandList->Dispatch(1, 1, 1);

	// UAV 버퍼를 COPY_SOURCE 상태로 전환하고, Draw buffer를 COPY_DEST 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dUAVBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	// UAV 버퍼에서 Draw buffer로 복사
	pd3dCommandList->CopyResource(particle_mesh->m_pd3dStreamOutputBuffer, particle_mesh->m_pd3dUAVBuffer);

	// Draw buffer를 다시 STREAM_OUT 상태로, UAV 버퍼는 UNORDERED_ACCESS 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dStreamOutputBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT);
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dUAVBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

}

void ParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int progress)
{
	OnPrepareRender();

	if (progress == 0)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (particle_mesh)
		{
			particle_mesh->PreRender(pd3dCommandList, 0); //Stream Output
			particle_mesh->Render(pd3dCommandList, 0); //Stream Output
			particle_mesh->PostRender(pd3dCommandList, 0); //Stream Output

		}

	}
	else if (progress == 1)
	{
		if (Material_list.size())
		{
			if (particle_mesh)
				particle_mesh->PreRender(pd3dCommandList, 1); //Draw

			// shape_mesh에 적용할 material
			for (std::shared_ptr<CMaterial> material_ptr : Material_list)
			{
				if (material_ptr)
				{
					CShader* pShader = material_ptr->m_pShader;
					if (pShader)
						pShader->OnPrepareRender(pd3dCommandList, 1); // == SetPipelineState
					material_ptr->UpdateShaderVariable(pd3dCommandList);
				}
			}



			if (shape_mesh)
				shape_mesh->Instancing_Render(pd3dCommandList, particle_mesh->m_d3dParticleBufferView, particle_mesh->Get_Num()); //Draw


		}
	} 
}




void ParticleObject::OnPostRender()
{
	if (particle_mesh)
		particle_mesh->OnPostRender(0); //Read Stream Output Buffer Filled Size
}

//==================================================

ParticleShader::ParticleShader()
{
}

ParticleShader::~ParticleShader()
{
	Release_Compute_ShaderVariables();
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE ParticleShader::GetPrimitiveTopologyType(int nPipelineState)
{
	if (nPipelineState == 0)
		return(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	else if (nPipelineState == 1)
		return (D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
}

UINT ParticleShader::GetNumRenderTargets(int nPipelineState)
{
	return((nPipelineState == 0) ? 0 : 1);
}

DXGI_FORMAT ParticleShader::GetRTVFormat(int nPipelineState, int nRenderTarget)
{
	return((nPipelineState == 0) ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R8G8B8A8_UNORM);
}

DXGI_FORMAT ParticleShader::GetDSVFormat(int nPipelineState)
{
	return(DXGI_FORMAT_D24_UNORM_S8_UINT);
}

D3D12_SHADER_BYTECODE ParticleShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Particle.hlsl", "VSParticleStreamOutput", "vs_5_1", ppd3dShaderBlob));
	else
		return(CShader::CompileShaderFromFile(L"Particle.hlsl", "VSParticleDraw", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE ParticleShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Particle.hlsl", "GSParticleStreamOutput", "gs_5_1", ppd3dShaderBlob));
	else
		return(CShader::CompileShaderFromFile(L"Particle.hlsl", "GSParticleDraw", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE ParticleShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CreatePixelShader(ppd3dShaderBlob, 0));
	else
		return(CShader::CompileShaderFromFile(L"Particle.hlsl", "PSParticleDraw", "ps_5_1", ppd3dShaderBlob));
}


D3D12_BLEND_DESC ParticleShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

D3D12_DEPTH_STENCIL_DESC ParticleShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_INPUT_LAYOUT_DESC ParticleShader::CreateInputLayout(int nPipelineState)
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	if (nPipelineState == 0)
	{
		UINT nInputElementDescs = 4;
		D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

		pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[1] = { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[2] = { "LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[3] = { "PARTICLETYPE", 0, DXGI_FORMAT_R32_UINT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
		d3dInputLayoutDesc.NumElements = nInputElementDescs;
	}
	else if (nPipelineState == 1)
	{
		UINT nInputElementDescs = 6;
		D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

		pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[1] = { "COLOR",	 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		// 파티클 인스턴스 데이터
		pd3dInputElementDescs[2] = { "WORLD_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	2, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[3] = { "VELOCITY",				0, DXGI_FORMAT_R32G32B32_FLOAT,	2, 12, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[4] = { "LIFETIME",				0, DXGI_FORMAT_R32_FLOAT,				2, 24, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[5] = { "PARTICLETYPE",		0, DXGI_FORMAT_R32_UINT,				2, 28, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

		d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
		d3dInputLayoutDesc.NumElements = nInputElementDescs;
	}
	return(d3dInputLayoutDesc);
}

D3D12_STREAM_OUTPUT_DESC ParticleShader::CreateStreamOuputState(int nPipelineState)
{
	D3D12_STREAM_OUTPUT_DESC d3dStreamOutputDesc;
	::ZeroMemory(&d3dStreamOutputDesc, sizeof(D3D12_STREAM_OUTPUT_DESC));

	if (nPipelineState == 0)
	{
		UINT nStreamOutputDecls = 4;
		D3D12_SO_DECLARATION_ENTRY* pd3dStreamOutputDecls = new D3D12_SO_DECLARATION_ENTRY[nStreamOutputDecls];
		pd3dStreamOutputDecls[0] = { 0, "POSITION", 0, 0, 3, 0 };
		pd3dStreamOutputDecls[1] = { 0, "VELOCITY", 0, 0, 3, 0 };
		pd3dStreamOutputDecls[2] = { 0, "LIFETIME", 0, 0, 1, 0 };
		pd3dStreamOutputDecls[3] = { 0, "PARTICLETYPE", 0, 0, 1, 0 };

		UINT* pBufferStrides = new UINT[1];
		pBufferStrides[0] = sizeof(ParticleVertex);

		d3dStreamOutputDesc.NumEntries = nStreamOutputDecls;
		d3dStreamOutputDesc.pSODeclaration = pd3dStreamOutputDecls;
		d3dStreamOutputDesc.NumStrides = 1;
		d3dStreamOutputDesc.pBufferStrides = pBufferStrides;
		d3dStreamOutputDesc.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;
	}

	return(d3dStreamOutputDesc);
}


D3D12_RASTERIZER_DESC ParticleShader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

void ParticleShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL, * pd3dGeometryShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob, nPipelineState);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob, nPipelineState);
	if (nPipelineState == 0)
	{
		d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryShaderBlob, nPipelineState);
		d3dPipelineStateDesc.StreamOutput = CreateStreamOuputState(nPipelineState);
	}
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState(nPipelineState);
	d3dPipelineStateDesc.BlendState = CreateBlendState(nPipelineState);
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState(nPipelineState);
	d3dPipelineStateDesc.InputLayout = CreateInputLayout(nPipelineState);
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = GetPrimitiveTopologyType(nPipelineState);
	d3dPipelineStateDesc.NumRenderTargets = GetNumRenderTargets(nPipelineState);
	d3dPipelineStateDesc.RTVFormats[0] = GetRTVFormat(nPipelineState, 0);
	d3dPipelineStateDesc.DSVFormat = GetDSVFormat(nPipelineState);
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;



	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[nPipelineState]);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void ParticleShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 2;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	m_ncomputePipelineStates = 1;
	m_ppd3dcomputePipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0); //Stream Output Pipeline State
	CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 1); //Draw Pipeline State

	m_pd3dComputeRootSignature = CreateComputeRootSignature(pd3dDevice);
	CreateComputePipelineState(pd3dDevice, m_pd3dComputeRootSignature);

	Create_Compute_ShaderVariables(pd3dDevice, pd3dCommandList);
}

D3D12_SHADER_BYTECODE ParticleShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return CShader::CompileShaderFromFile(L"Particle_CS.hlsl", "CSMain", "cs_5_1", ppd3dShaderBlob);
}

ID3D12RootSignature* ParticleShader::CreateComputeRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dComputeRootSignature = NULL;

	// 루트 파라미터 설정
	D3D12_ROOT_PARAMETER pd3dRootParameters[2];
	{
		// 상수 버퍼 (CBV) 정의
		pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[0].Descriptor.ShaderRegister = 0;  // b0
		pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// UAV 정의
		pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		pd3dRootParameters[1].Descriptor.ShaderRegister = 0;  // u0
		pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[1];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	HRESULT b = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	if (FAILED(b))
	{
		if (pd3dErrorBlob)
		{
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
			pd3dErrorBlob->Release();
		}
		return nullptr;
	}
	b = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dComputeRootSignature);
	if (FAILED(b))
	{
		if (pd3dErrorBlob)
		{
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
			pd3dErrorBlob->Release();
		}
	}

	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dComputeRootSignature);
}

void ParticleShader::CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dComputeRootSignature, int nPipelineState)
{
	ID3DBlob* pd3dComputeShaderBlob = NULL;

	// 계산 파이프라인 상태 구성 구조체
	D3D12_COMPUTE_PIPELINE_STATE_DESC d3dComputePipelineStateDesc;
	::ZeroMemory(&d3dComputePipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	d3dComputePipelineStateDesc.pRootSignature = pd3dComputeRootSignature;
	d3dComputePipelineStateDesc.CS = CreateComputeShader(&pd3dComputeShaderBlob, nPipelineState);
	d3dComputePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	// 계산 파이프라인 상태 객체 생성
	HRESULT hResult = pd3dDevice->CreateComputePipelineState(&d3dComputePipelineStateDesc, IID_PPV_ARGS(&m_ppd3dcomputePipelineStates[nPipelineState]));

	if (pd3dComputeShaderBlob) pd3dComputeShaderBlob->Release();
}

void ParticleShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{


	//=================================================

	//CScene::CreateShaderResourceViews(pd3dDevice, pParticleTexture, 0, PARAMETER_DEFAULT_TEXTURE);
	//CScene::CreateShaderResourceViews(pd3dDevice, m_pRandowmValueTexture, 0, 14);
}

void ParticleShader::Set_Compute_Pipeline(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetComputeRootSignature(m_pd3dComputeRootSignature);
	pd3dCommandList->SetPipelineState(m_ppd3dcomputePipelineStates[0]);
}

void ParticleShader::Create_Compute_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_Particle_Update_Info) + 255) & ~255); //256의 배수
	Particle_Update_Info = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	Particle_Update_Info->Map(0, NULL, (void**)&Mapped_Particle_Update_Info);
}

void ParticleShader::Update_Compute_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, UINT particle_count, float fTimeElapsed)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = Particle_Update_Info->GetGPUVirtualAddress();
	Mapped_Particle_Update_Info->Particle_N = particle_count;
	Mapped_Particle_Update_Info->ElapsedTime = fTimeElapsed;

	pd3dCommandList->SetComputeRootConstantBufferView(0, d3dGpuVirtualAddress);
}

void ParticleShader::Release_Compute_ShaderVariables()
{
	if (Particle_Update_Info) Particle_Update_Info->Unmap(0, NULL);
	if (Particle_Update_Info) Particle_Update_Info->Release();
}

//===================================================================

Particle_Manager::Particle_Manager(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	srand((unsigned)time(NULL));

	XMFLOAT4* pxmf4RandomValues = new XMFLOAT4[1024];
	for (int i = 0; i < 1024; i++)
	{
		pxmf4RandomValues[i].x = float((rand() % 10000) - 5000) / 5000.0f;
		pxmf4RandomValues[i].y = float((rand() % 10000) - 5000) / 5000.0f;
		pxmf4RandomValues[i].z = float((rand() % 10000) - 5000) / 5000.0f;
		pxmf4RandomValues[i].w = float((rand() % 10000) - 5000) / 5000.0f;
	}

	m_pRandowmValueTexture = new CTexture(1, RESOURCE_BUFFER, 0, 1);
	m_pRandowmValueTexture->CreateBuffer(pd3dDevice, pd3dCommandList, pxmf4RandomValues, 1024, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, m_pRandowmValueTexture, 0, ROOT_PARAMETER_RANDOM_VALUE_SRV_INDEX);
	
}

Particle_Manager::~Particle_Manager()
{

}

void Particle_Manager::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	//CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	//pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"texture/RoundSoftParticle.dds", RESOURCE_TEXTURE2D, 0);
	//particle_Material->SetTexture(pParticleTexture);
	
	CMesh* new_shape_mesh = NULL; // -> 인스턴싱 그리기가 가능해야 함
	Particle_Shape_Mesh* sphere_shape_mesh = new Sphere_Shape_Mesh(pd3dDevice, pd3dCommandList, 20.0f);
	Particle_Shape_Mesh* cube_shape_mesh = new Cube_Shape_Mesh(pd3dDevice, pd3dCommandList);

	//===================================================================
	ParticleShader* test_shader = new ParticleShader();
	test_shader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//===================================================================
	particle_shader_map[Particle_Type::sample_1] = test_shader;
	particle_shader_map[Particle_Type::sample_2] = NULL;
	particle_shader_map[Particle_Type::sample_3] = NULL;
	//===================================================================
	Particle_Info test_info;
	test_info.type = Particle_Type::sample_1;
	test_info.pos = XMFLOAT3(10.0f, 10.0f, 10.0f);
	test_info.velocity = XMFLOAT3(0.0f, 1.0f, 0.0f);
	test_info.acceleration = XMFLOAT3(0.0f, 1.0f, 0.0f);
	test_info.size = XMFLOAT2(10.0f, 10.0f);
	test_info.color = XMFLOAT3(1.0f, 0.0f, 0.0f);
	//===================================================================

	Add_Particle(pd3dDevice, pd3dCommandList, cube_shape_mesh, test_info);
	//===================================================================
}

void Particle_Manager::AnimateObjects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	for (auto& [type, shader_ptr] : particle_shader_map)
	{
		if (!shader_ptr)
			continue;

		shader_ptr->Set_Compute_Pipeline(pd3dCommandList);

		for (std::shared_ptr<ParticleObject> particle_obj : particle_object_list_map[type])
		{
			int particle_num = particle_obj->Get_Particle_Num();

			shader_ptr->Update_Compute_ShaderVariables(pd3dCommandList, particle_num, fTimeElapsed); // CS에 파티클 정보 버퍼 업데이트 및 연결

			particle_obj->Animate(pd3dCommandList);
		}
	}
}

void Particle_Manager::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int N, Particle_Type type)
{
	if (!particle_shader_map[type])
		return;

	particle_shader_map[type]->Setting_Render(pd3dCommandList, N);

	for (std::shared_ptr<ParticleObject> particle_obj : particle_object_list_map[type])
		particle_obj->Render(pd3dCommandList, pCamera, N);

}

void Particle_Manager::Render_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int N)
{
	if (N == 0 && m_pRandowmValueTexture)
		m_pRandowmValueTexture->UpdateShaderVariables(pd3dCommandList);

	Render(pd3dCommandList, pCamera, N, Particle_Type::sample_1);
	Render(pd3dCommandList, pCamera, N, Particle_Type::sample_2);
	Render(pd3dCommandList, pCamera, N, Particle_Type::sample_3);
}

void Particle_Manager::OnPostRender(Particle_Type type)
{
	for (std::shared_ptr<ParticleObject> particle_obj : particle_object_list_map[type])
		particle_obj->OnPostRender();
}

void Particle_Manager::OnPostRender_All()
{
	OnPostRender(Particle_Type::sample_1);
	OnPostRender(Particle_Type::sample_2);
	OnPostRender(Particle_Type::sample_3);
}

void Particle_Manager::Add_Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMesh* particle_shape_mesh, Particle_Info particle_info)
{
	std::shared_ptr<ParticleObject> new_particle_obj = make_shared<ParticleObject>();
	ParticleMesh* new_particle_mesh = new ParticleMesh(pd3dDevice, pd3dCommandList, particle_info.pos, particle_info.velocity, 0.0f, particle_info.acceleration, particle_info.color, particle_info.size, particle_info.max_particles);

	new_particle_obj->Set_Shape(particle_shape_mesh);
	new_particle_obj->Set_Particle_Mesh(new_particle_mesh);
	new_particle_obj->SetMesh(NULL);

	particle_object_list_map[particle_info.type].push_back(new_particle_obj);
	
}
