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
Particle::Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCmdList, UINT nMaxParticles)
{
	m_nMaxParticles = nMaxParticles;
	m_nStride = sizeof(Particle_Info);

	CreateBuffers(pd3dDevice, pd3dCmdList);
	CreateCounterReadbackBuffer(pd3dDevice);

}

Particle::~Particle()
{
	ReleaseBuffers();
}

void Particle::CreateBuffers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	particle_buffer_texture = new CTexture(3, RESOURCE_STRUCTURED_BUFFER, 0, 0, 1, 0, 0, 3, 0);

	particle_buffer_texture->CreateStructuredBuffer(pd3dDevice, pd3dCommandList, 0, nullptr, m_nMaxParticles, sizeof(Particle_Info), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	particle_buffer_texture->CreateStructuredBuffer(pd3dDevice, pd3dCommandList, 1, nullptr, m_nMaxParticles, sizeof(UINT), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	particle_buffer_texture->CreateStructuredBuffer(pd3dDevice, pd3dCommandList, 2, nullptr, m_nMaxParticles, sizeof(RenderInstance), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	counterBuffer_1 = CreateCounterBuffer(pd3dDevice);
	counterBuffer_2 = CreateCounterBuffer(pd3dDevice);
	counterBuffer_3 = CreateCounterBuffer(pd3dDevice);

	CDescriptor_Heap::CreateStructuredBufferUAV(pd3dDevice, particle_buffer_texture, 0, counterBuffer_1, 1);
	CDescriptor_Heap::CreateStructuredBufferUAV(pd3dDevice, particle_buffer_texture, 1, counterBuffer_2, 2);
	CDescriptor_Heap::CreateStructuredBufferUAV(pd3dDevice, particle_buffer_texture, 2, counterBuffer_3, 3);

	D3D12_GPU_VIRTUAL_ADDRESS RenderInstance_buffer = particle_buffer_texture->GetResource(2)->GetGPUVirtualAddress();
	m_RenderInstanceVBV.BufferLocation = RenderInstance_buffer;
	m_RenderInstanceVBV.StrideInBytes = sizeof(RenderInstance);
	m_RenderInstanceVBV.SizeInBytes = sizeof(RenderInstance) * m_nMaxParticles;
}

void Particle::ReleaseBuffers()
{
}

ID3D12Resource* Particle::CreateCounterBuffer(ID3D12Device* pd3dDevice)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = sizeof(UINT); // 4바이트: 카운터용
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	ID3D12Resource* pCounterBuffer = nullptr;
	HRESULT hr = pd3dDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pCounterBuffer));

	if (FAILED(hr))
	{
		OutputDebugString(L"❌ Failed to create Counter Buffer\n");
		return nullptr;
	}

	return pCounterBuffer;
}

void Particle::CreateCounterReadbackBuffer(ID3D12Device* pd3dDevice)
{
	if (!m_pCounterReadbackBuffer)
	{
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_READBACK;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = sizeof(UINT);
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		pd3dDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pCounterReadbackBuffer));
	}
}




void Particle::RequestParticleCount(ID3D12GraphicsCommandList* pd3dCmdList)
{
	// 인스턴스 정보 버퍼에서 카운터 정보 복사하기
	// Dest (CPU 접근 가능한 버퍼)
	// Dest offset
	// Src: Counter buffer
	// Src offset
	pd3dCmdList->CopyBufferRegion(m_pCounterReadbackBuffer, 0, counterBuffer_3, 0, sizeof(UINT));
}

UINT Particle::Get_Particle_Num()
{
	if (!m_pCounterReadbackBuffer) 
		return 0;

	UINT count = 0;
	void* pData = nullptr;
	D3D12_RANGE range = { 0, sizeof(UINT) };

	if (SUCCEEDED(m_pCounterReadbackBuffer->Map(0, &range, &pData)) && pData)
	{
		count = *reinterpret_cast<UINT*>(pData);
		m_pCounterReadbackBuffer->Unmap(0, nullptr);
	}
	return count;
}


// 렌더링용 VBV 업데이트
void Particle::UpdateRenderInstanceVBV()
{
	D3D12_GPU_VIRTUAL_ADDRESS RenderInstance_buffer = particle_buffer_texture->GetResource(2)->GetGPUVirtualAddress();
	m_RenderInstanceVBV.BufferLocation = RenderInstance_buffer;
	m_RenderInstanceVBV.StrideInBytes = sizeof(RenderInstance);
	m_RenderInstanceVBV.SizeInBytes = sizeof(RenderInstance) * m_nMaxParticles;
}

// 인스턴싱 렌더링
void Particle::Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, int instanceCount)
{
	pd3dCommandList->IASetVertexBuffers(1, 1, &m_RenderInstanceVBV);
	pd3dCommandList->DrawInstanced(1, instanceCount, 0, 0);
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

void PrePare_Update()
{

}

void ParticleObject::Animate(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//// Draw buffer를 COPY_SOURCE 상태로 전환하고, UAV 버퍼를 COPY_DEST 상태로 전환
	//::SynchronizeResourceTransition(pd3dCommandList, particle->Particle_Draw_Buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_SOURCE);

	//// Draw buffer에서 UAV 버퍼로 복사
	//pd3dCommandList->CopyResource(particle_mesh->CS_UAV_Buffer, particle_mesh->Particle_Draw_Buffer);

	//// UAV 버퍼를 UNORDERED_ACCESS 상태로, Draw buffer는 COPY_DEST 상태로 전환
	//::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->CS_UAV_Buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	//// Draw buffer에 대해 UAV 뷰를 설정하고, Compute Shader 실행
	//pd3dCommandList->SetComputeRootUnorderedAccessView(1, particle_mesh->CS_UAV_Buffer->GetGPUVirtualAddress());

	//int cs_threadGroup_Size = 256;
	//int dispatch_Size = (Get_Particle_Num() + cs_threadGroup_Size - 1) / cs_threadGroup_Size;


	//pd3dCommandList->Dispatch(dispatch_Size, 1, 1);

	//// UAV 버퍼를 COPY_SOURCE 상태로 전환하고, Draw buffer를 COPY_DEST 상태로 전환
	//::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->Particle_Draw_Buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	//::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->CS_UAV_Buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	//// UAV 버퍼에서 Draw buffer로 복사
	//pd3dCommandList->CopyResource(particle_mesh->Particle_Draw_Buffer, particle_mesh->CS_UAV_Buffer);

	//// Draw buffer를 다시 CONSTANT_BUFFER 상태로, UAV 버퍼는 COPY_DEST 상태로 전환
	//::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->Particle_Draw_Buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	//::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->CS_UAV_Buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
}

void ParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int progress)
{

	//if (progress == 0)
	//{
	//	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

	//	if (particle_mesh)
	//	{
	//		particle_mesh->PreRender(pd3dCommandList, 0); //Stream Output
	//		particle_mesh->Render(pd3dCommandList, 0); //Stream Output
	//	}

	//}
	//else if (progress == 1)
	//{
	//	if (particle_mesh)
	//		particle_mesh->PreRender(pd3dCommandList, 1); //Draw

	//	if (shape_mesh)
	//		shape_mesh->Instancing_Render(pd3dCommandList, particle_mesh->Particle_Info_Buffer_View, particle_mesh->Get_Num()); //Draw

	//}
}




void ParticleObject::OnPostRender()
{
	if (particle_mesh)
		particle_mesh->OnPostRender(0); //Read Stream Output Buffer Filled Size
}
