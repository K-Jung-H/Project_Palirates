#include "stdafx.h"
#include "Mesh.h"
#include "Object.h"

bool IsPointInTriangle(float x, float z, const XMFLOAT3& v0, const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	// 검사할 점
	XMFLOAT2 p(x, z);
	XMFLOAT2 a(v0.x, v0.z);
	XMFLOAT2 b(v1.x, v1.z);
	XMFLOAT2 c(v2.x, v2.z);

	// 벡터 외적 계산
	float cross1 = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
	float cross2 = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x);
	float cross3 = (a.x - c.x) * (p.y - c.y) - (a.y - c.y) * (p.x - c.x);

	// 외적 값의 부호가 모두 같거나, 절대값 비교를 통해 방향성을 무시
	return ((cross1 >= 0 && cross2 >= 0 && cross3 >= 0) ||
		(cross1 <= 0 && cross2 <= 0 && cross3 <= 0));
}




//==============================================================================

CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

CMesh::~CMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();

	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
			if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
		}
		if (m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;
		if (m_pd3dSubSetIndexBufferViews) delete[] m_pd3dSubSetIndexBufferViews;

		if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
		if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;
	}

	if (m_pxmf3Positions) delete[] m_pxmf3Positions;

	// 메모리 디버그 - 제거되는 메시 이름 확인
	if(m_pstrMeshName != nullptr && std::strlen(m_pstrMeshName) == 0)
		DebugOutput("\nDelete Mesh: No name");
	else
		DebugOutput("\nDelete Mesh: ",m_pstrMeshName);
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;

	if ((m_nSubMeshes > 0) && m_ppd3dSubSetIndexUploadBuffers)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexUploadBuffers) delete[] m_ppd3dSubSetIndexUploadBuffers;
		m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
}

void CMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
}

void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet)
{
	UpdateShaderVariables(pd3dCommandList);

	OnPreRender(pd3dCommandList, NULL);

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void CMesh::OnPostRender(ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
}

void CMesh::Set_BoundingBox(BoundingOrientedBox* new_obb_ptr)
{
	if (new_obb_ptr == NULL)
		bounding_box = new BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	else
	{
		if (bounding_box == nullptr)		
			bounding_box = new BoundingOrientedBox();
		*bounding_box = *new_obb_ptr;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE *pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y)*m_nWidth)] = pHeightMapPixels[x + (y*m_nWidth)];
		}
	}

	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels != NULL) 
		delete[] m_pHeightMapPixels;

	m_pHeightMapPixels = NULL;
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z*m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z*m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1)*m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1)*m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, int Vertex_gap, void *pContext) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;	
	Vertex_Gap = (Vertex_gap != 0) ? Vertex_gap : 1;


	int adjustedWidth = (m_nWidth + Vertex_Gap - 1) / Vertex_Gap;
	int adjustedLength = (m_nLength + Vertex_Gap - 1) / Vertex_Gap;
	m_nVertices = adjustedWidth * adjustedLength;

	
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf4Colors = new XMFLOAT4[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
	m_pxmf2TextureCoords1 = new XMFLOAT2[m_nVertices];

	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	int cxHeightMap = pHeightMapImage->GetHeightMapWidth();
	int czHeightMap = pHeightMapImage->GetHeightMapLength();

	m_xmArea_LT = XMFLOAT2(xStart * m_xmf3Scale.x, zStart * m_xmf3Scale.z);
	m_xmArea_RB = XMFLOAT2((xStart + (nWidth - 1)) * m_xmf3Scale.x, (zStart + (nLength - 1)) * m_xmf3Scale.z);


	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;


	for (int i = 0, z = zStart; z < (zStart + m_nLength); z += Vertex_Gap)
	{
		for (int x = xStart; x < (xStart + m_nWidth); x += Vertex_Gap, i++)
		{
			fHeight = OnGetHeight(x, z, pContext);

			if (fHeight == -100.0f)
			{
				bool bFoundValidHeight = false;
				for (int offset = 1; !bFoundValidHeight; offset++)
				{
					std::vector<std::pair<int, int>> offsets = { {x - offset, z}, {x, z - offset}, {x - offset, z - offset} };
					for (const std::pair<int, int>& pos : offsets)
					{
						fHeight = OnGetHeight(pos.first, pos.second, pContext);
						if (fHeight != -100.0f)
						{
							bFoundValidHeight = true;
							break;
						}
					}
				}

				if (!bFoundValidHeight)
					fHeight = 0.0f;
			}

			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;

			m_pxmf3Positions[i] = XMFLOAT3((x * m_xmf3Scale.x), fHeight, (z * m_xmf3Scale.z));
			m_pxmf4Colors[i] = Vector4::Add(OnGetColor(x, z, pContext), xmf4Color);

			m_pxmf2TextureCoords0[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			m_pxmf2TextureCoords1[i] = XMFLOAT2(float(x) / float(m_xmf3Scale.x * 0.5f), float(z) / float(m_xmf3Scale.z * 0.5f));
		}

	}


	m_nSubMeshes = 1;
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

	m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
	m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

	m_pnSubSetIndices[0] = (adjustedWidth * 2 * (adjustedLength - 1)) + (adjustedLength - 2);
	m_ppnSubSetIndices[0] = new UINT[m_pnSubSetIndices[0]];


	for (int j = 0, z = 0; z <adjustedLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < adjustedWidth; x++)
			{
				if ((x == 0) && (z > 0))
					m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * adjustedWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * adjustedWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)((x + (z * adjustedWidth)) + adjustedWidth);
			}
		}
		else
		{
			for (int x = adjustedWidth - 1; x >= 0; x--)
			{
				if (x == (adjustedWidth - 1))
					m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * adjustedWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * adjustedWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)((x + (z * adjustedWidth)) + adjustedWidth);
			}
		}
	}

	m_ppd3dSubSetIndexBuffers[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[0]);

	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dColorBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4Colors, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColorUploadBuffer);

	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord1UploadBuffer);

	m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	XMFLOAT3 center = 
	{
		(m_xmArea_LT.x + m_xmArea_RB.x) * 0.5f,
		(fMinHeight + fMaxHeight) * 0.5f,
		(m_xmArea_LT.y + m_xmArea_RB.y) * 0.5f
	};

	XMFLOAT3 extents = 
	{
		(m_xmArea_RB.x - m_xmArea_LT.x) * 0.5f,
		(fMaxHeight - fMinHeight) * 0.5f,
		(m_xmArea_RB.y - m_xmArea_LT.y) * 0.5f
	};

	bounding_box = new BoundingOrientedBox(center, extents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pd3dTextureCoord1Buffer) m_pd3dTextureCoord1Buffer->Release();

	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	if (m_pxmf2TextureCoords1) delete[] m_pxmf2TextureCoords1;
}

void CHeightMapGridMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dColorUploadBuffer) m_pd3dColorUploadBuffer->Release();
	m_pd3dColorUploadBuffer = NULL;

	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;

	if (m_pd3dTextureCoord1UploadBuffer) m_pd3dTextureCoord1UploadBuffer->Release();
	m_pd3dTextureCoord1UploadBuffer = NULL;
}


float CHeightMapGridMesh::OnGetHeight(int x, int z, void *pContext)
{
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;

	int Hight_Map_Width = pHeightMapImage->GetHeightMapWidth();
	int Hight_Map_Length = pHeightMapImage->GetHeightMapLength();

	// x, z 범위 확인
	if (x < 0 || x >= Hight_Map_Width || z < 0 || z >= Hight_Map_Length)
	{
		std::string errorMessage = "\nOut of bounds: x=" + std::to_string(x) + ", z=" + std::to_string(z) 
			+ " (Width=" + std::to_string(Hight_Map_Width) + ", Length=" + std::to_string(Hight_Map_Length) + ")";

		DebugOutput(errorMessage);
		return -100.0f;
	}

	BYTE *pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();


	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z*nWidth)] * xmf3Scale.y;
	return(fHeight);
}

XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void *pContext)
{
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);
	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1), xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);
	return(xmf4Color);
}

void CHeightMapGridMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[4] = { m_d3dPositionBufferView, m_d3dColorBufferView, m_d3dTextureCoord0BufferView, m_d3dTextureCoord1BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 4, pVertexBufferViews);
}

float CHeightMapGridMesh::Get_Height(float x, float z)
{
	// 모든 삼각형을 순회
	for (UINT i = 0; i < m_pnSubSetIndices[0] - 2; ++i)
	{
		// 삼각형 스트립에서 연속된 인덱스를 사용하여 삼각형 생성
		UINT index0 = m_ppnSubSetIndices[0][i];
		UINT index1 = m_ppnSubSetIndices[0][i + 1];
		UINT index2 = m_ppnSubSetIndices[0][i + 2];

		// 삼각형의 정점 좌표 가져오기
		XMFLOAT3 v0 = m_pxmf3Positions[index0];
		XMFLOAT3 v1 = m_pxmf3Positions[index1];
		XMFLOAT3 v2 = m_pxmf3Positions[index2];

		// 동일한 정점이 있으면 해당 삼각형을 건너뛰기
		if (std::tie(v0.x, v0.y, v0.z) == std::tie(v1.x, v1.y, v1.z) ||
			std::tie(v1.x, v1.y, v1.z) == std::tie(v2.x, v2.y, v2.z) ||
			std::tie(v0.x, v0.y, v0.z) == std::tie(v2.x, v2.y, v2.z))
		{
			continue;
		}

		// 입력 좌표 (x, z)가 삼각형 내부에 포함되는지 확인
		if (IsPointInTriangle(x, z, v0, v1, v2))
		{
#ifdef DEBUG_MESSAGE
#ifdef	DEBUG_MESSAGE_HEIGHT_POLYGON_INFO

			string message = "Index: " + std::to_string(i) + " - p1: (x = " + std::to_string((int)v0.x) + ", z = " + std::to_string((int)v0.z) + ") ";
			message += "p2: (x = " + std::to_string((int)v1.x) + ", z = " + std::to_string((int)v1.z) + ") ";
			message += "p3: (x = " + std::to_string((int)v2.x) + ", z = " + std::to_string((int)v2.z) + ")\n";
			DebugOutput(message);
#endif
#endif

			// 삼각형 높이 반환
			return Get_PolygonHeight(x, z, v0, v1, v2);
		}
	}

	// 포함된 폴리곤을 찾지 못한 경우 지정된 높이 반환
	return -1.0f;
}

float  CHeightMapGridMesh::Get_PolygonHeight(float x, float z, XMFLOAT3 v0, XMFLOAT3 v1, XMFLOAT3 v2)

{
	// 삼각형의 두 개의 변을 계산
	XMFLOAT3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
	XMFLOAT3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

	// 삼각형의 법선 벡터 계산 (외적)
	XMFLOAT3 normal;
	normal.x = edge1.y * edge2.z - edge1.z * edge2.y;
	normal.y = edge1.z * edge2.x - edge1.x * edge2.z;
	normal.z = edge1.x * edge2.y - edge1.y * edge2.x;

	// 평면 방정식의 D 값 계산 (D = - (A*x + B*y + C*z))
	float D = -(normal.x * v0.x + normal.y * v0.y + normal.z * v0.z);

	// y = (-Ax - Cz - D) / B를 이용하여 높이(y) 계산
	if (normal.y != 0.0f)
	{
		return (-normal.x * x - normal.z * z - D) / normal.y;
	}

	return v0.y; // 법선이 y축과 평행한 경우 예외 처리
}


XMFLOAT3 CHeightMapGridMesh::Get_Normal(float x, float z)
{
	// 모든 삼각형을 순회
	for (UINT i = 0; i < m_pnSubSetIndices[0] - 2; ++i)
	{
		// 삼각형 스트립에서 연속된 인덱스를 사용하여 삼각형 생성
		UINT index0 = m_ppnSubSetIndices[0][i];
		UINT index1 = m_ppnSubSetIndices[0][i + 1];
		UINT index2 = m_ppnSubSetIndices[0][i + 2];

		// 삼각형의 정점 좌표 가져오기
		XMFLOAT3 v0 = m_pxmf3Positions[index0];
		XMFLOAT3 v1 = m_pxmf3Positions[index1];
		XMFLOAT3 v2 = m_pxmf3Positions[index2];

		 // 동일한 정점이 있으면 해당 삼각형을 건너뛰기
		if (std::tie(v0.x, v0.y, v0.z) == std::tie(v1.x, v1.y, v1.z) ||
			std::tie(v1.x, v1.y, v1.z) == std::tie(v2.x, v2.y, v2.z) ||
			std::tie(v0.x, v0.y, v0.z) == std::tie(v2.x, v2.y, v2.z))
		{
			continue; 
		}

		// 입력 좌표 (x, z)가 삼각형 내부에 포함되는지 확인
		if (IsPointInTriangle(x, z, v0, v1, v2))
		{
#ifdef DEBUG_MESSAGE
#ifdef	DEBUG_MESSAGE_NORMAL_POLYGON_INFO
			string message = "Index: " + std::to_string(i) + " - p1: (x = " + std::to_string((int)v0.x) + ", z = " + std::to_string((int)v0.z) + ") ";
			message += "p2: (x = " + std::to_string((int)v1.x) + ", z = " + std::to_string((int)v1.z) + ") ";
			message += "p3: (x = " + std::to_string((int)v2.x) + ", z = " + std::to_string((int)v2.z) + ")\n";
			DebugOutput(message);
#endif
#endif
			bool is_Reversed = (i % 2 == 1);  
			return Get_PolygonNormal(v0, v1, v2, is_Reversed);
		}
	}

	// 포함된 폴리곤을 찾지 못한 경우 지정된 법선 반환
	return XMFLOAT3(0.0f, -1.0f, 0.0f);
}

XMFLOAT3 CHeightMapGridMesh::Get_PolygonNormal(XMFLOAT3 v0, XMFLOAT3 v1, XMFLOAT3 v2, bool is_Reversed)
{
	XMFLOAT3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
	XMFLOAT3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

	XMFLOAT3 normal;
	normal.x = edge1.y * edge2.z - edge1.z * edge2.y;
	normal.y = edge1.z * edge2.x - edge1.x * edge2.z;
	normal.z = edge1.x * edge2.y - edge1.y * edge2.x;

	float length = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	if (length > 0.0f)
	{
		normal.x /= length;
		normal.y /= length;
		normal.z /= length;
	}

	// 삼각형이 뒤집혀 있으면 노멀 반전
	if (is_Reversed)
	{
		normal.x = -normal.x;
		normal.y = -normal.y;
		normal.z = -normal.z;
	}

	return normal;
}

BoundingOrientedBox* CHeightMapGridMesh::Get_BoundingBox() 
{
	return bounding_box;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CSkyBoxMesh::CSkyBoxMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	// Front Quad (quads point inward)
	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[2] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[3] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[5] = XMFLOAT3(+fx, -fx, +fx);
	// Back Quad										
	m_pxmf3Positions[6] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[7] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[8] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[9] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[10] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[11] = XMFLOAT3(-fx, -fx, -fx);
	// Left Quad										
	m_pxmf3Positions[12] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[13] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[14] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[15] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[16] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[17] = XMFLOAT3(-fx, -fx, +fx);
	// Right Quad										
	m_pxmf3Positions[18] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[19] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[20] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[21] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[22] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[23] = XMFLOAT3(+fx, -fx, -fx);
	// Top Quad											
	m_pxmf3Positions[24] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[25] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[26] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[27] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[28] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[29] = XMFLOAT3(+fx, +fx, +fx);
	// Bottom Quad										
	m_pxmf3Positions[30] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[31] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[32] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[33] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[34] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[35] = XMFLOAT3(+fx, -fx, -fx);

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

CSkyBoxMesh::~CSkyBoxMesh()
{
}


CubeMesh::CubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	// 정점 개수 설정
	m_nVertices = 8;
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	// 큐브의 8개 정점 위치 초기화
	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fy, -fz); // v0
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fy, -fz);  // v1
	m_pxmf3Positions[2] = XMFLOAT3(+fx, +fy, +fz);   // v2
	m_pxmf3Positions[3] = XMFLOAT3(-fx, +fy, +fz);    // v3
	m_pxmf3Positions[4] = XMFLOAT3(-fx, -fy, -fz);   // v4
	m_pxmf3Positions[5] = XMFLOAT3(+fx, -fy, -fz);    // v5
	m_pxmf3Positions[6] = XMFLOAT3(+fx, -fy, +fz);     // v6
	m_pxmf3Positions[7] = XMFLOAT3(-fx, -fy, +fz);      // v7

	// 버퍼 생성
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	// 서브메쉬 개수 설정
	m_nSubMeshes = 1;

	// 인덱스 개수 설정
	int nSubMeshIndices = 36;
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

	m_pnSubSetIndices[0] = nSubMeshIndices;
	m_ppnSubSetIndices[0] = new UINT[nSubMeshIndices];

	// 큐브의 인덱스 배열 초기화
	UINT* indices = m_ppnSubSetIndices[0];

	// 각 면을 구성하는 인덱스 설정
	// 면 1: v0, v1, v2, v3
	indices[0] = 3; indices[1] = 1; indices[2] = 0;
	indices[3] = 2; indices[4] = 1; indices[5] = 3;

	// 면 2: v4, v5, v6, v7
	indices[6] = 0; indices[7] = 5; indices[8] = 4;
	indices[9] = 1; indices[10] = 5; indices[11] = 0;

	// 면 3: v0, v4, v6, v2
	indices[12] = 3; indices[13] = 4; indices[14] = 7;
	indices[15] = 0; indices[16] = 4; indices[17] = 3;

	// 면 4: v1, v3, v7, v5
	indices[18] = 1; indices[19] = 6; indices[20] = 5;
	indices[21] = 2; indices[22] = 6; indices[23] = 1;

	// 면 5: v0, v1, v5, v4
	indices[24] = 2; indices[25] = 7; indices[26] = 6;
	indices[27] = 3; indices[28] = 7; indices[29] = 2;

	// 면 6: v2, v6, v7, v3
	indices[30] = 6; indices[31] = 4; indices[32] = 5;
	indices[33] = 7; indices[34] = 4; indices[35] = 6;

	// 인덱스 버퍼 생성
	m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
	m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];

	m_ppd3dSubSetIndexBuffers[0] = CreateBufferResource(
		pd3dDevice, pd3dCommandList, indices, sizeof(UINT) * nSubMeshIndices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[0]
	);

	// 인덱스 버퍼 뷰 설정
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * nSubMeshIndices;
}


CubeMesh::~CubeMesh()
{
}

void CubeMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViews = (D3D12_VERTEX_BUFFER_VIEW*)pContext;
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews); // 2개 버퍼를 설정

}

void CubeMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num)
{
	UpdateShaderVariables(pd3dCommandList);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[] = { m_d3dPositionBufferView, d3dInstancingBufferView };
	OnPreRender(pd3dCommandList, pVertexBufferViews); 

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if (m_nSubMeshes > 0)
	{

		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[0]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[0], instance_num, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, instance_num, m_nOffset, 0);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardMesh::CStandardMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}

CStandardMesh::~CStandardMesh()
{
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	if (m_pd3dTangentBuffer) m_pd3dTangentBuffer->Release();
	if (m_pd3dBiTangentBuffer) m_pd3dBiTangentBuffer->Release();

	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	if (m_pxmf3Tangents) delete[] m_pxmf3Tangents;
	if (m_pxmf3BiTangents) delete[] m_pxmf3BiTangents;
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	if (m_pxmf2TextureCoords1) delete[] m_pxmf2TextureCoords1;
}

void CStandardMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;

	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;

	if (m_pd3dTangentUploadBuffer) m_pd3dTangentUploadBuffer->Release();
	m_pd3dTangentUploadBuffer = NULL;

	if (m_pd3dBiTangentUploadBuffer) m_pd3dBiTangentUploadBuffer->Release();
	m_pd3dBiTangentUploadBuffer = NULL;
}

void CStandardMesh::LoadMeshFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pInFile)
{
	char pstrToken[64] = { '\0' };
	int nPositions = 0, nColors = 0, nNormals = 0, nTangents = 0, nBiTangents = 0, nTextureCoords = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	UINT nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pInFile);

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nReads = (UINT)::fread(&nPositions, sizeof(int), 1, pInFile);
			if (nPositions > 0)
			{
				m_nType |= VERTEXT_POSITION;
				m_pxmf3Positions = new XMFLOAT3[nPositions];
				nReads = (UINT)::fread(m_pxmf3Positions, sizeof(XMFLOAT3), nPositions, pInFile);

				m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

				m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
				m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Colors>:"))
		{
			nReads = (UINT)::fread(&nColors, sizeof(int), 1, pInFile);
			if (nColors > 0)
			{
				m_nType |= VERTEXT_COLOR;
				m_pxmf4Colors = new XMFLOAT4[nColors];
				nReads = (UINT)::fread(m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords0>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD0;
				m_pxmf2TextureCoords0 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords0, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

				m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords1>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD1;
				m_pxmf2TextureCoords1 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords1, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord1UploadBuffer);

				m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&nNormals, sizeof(int), 1, pInFile);
			if (nNormals > 0)
			{
				m_nType |= VERTEXT_NORMAL;
				m_pxmf3Normals = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);

				m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

				m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
				m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			nReads = (UINT)::fread(&nTangents, sizeof(int), 1, pInFile);
			if (nTangents > 0)
			{
				m_nType |= VERTEXT_TANGENT;
				m_pxmf3Tangents = new XMFLOAT3[nTangents];
				nReads = (UINT)::fread(m_pxmf3Tangents, sizeof(XMFLOAT3), nTangents, pInFile);

				m_pd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTangentUploadBuffer);

				m_d3dTangentBufferView.BufferLocation = m_pd3dTangentBuffer->GetGPUVirtualAddress();
				m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BiTangents>:"))
		{
			nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, pInFile);
			if (nBiTangents > 0)
			{
				m_pxmf3BiTangents = new XMFLOAT3[nBiTangents];
				nReads = (UINT)::fread(m_pxmf3BiTangents, sizeof(XMFLOAT3), nBiTangents, pInFile);

				m_pd3dBiTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3BiTangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBiTangentUploadBuffer);

				m_d3dBiTangentBufferView.BufferLocation = m_pd3dBiTangentBuffer->GetGPUVirtualAddress();
				m_d3dBiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dBiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&(m_nSubMeshes), sizeof(int), 1, pInFile);
			if (m_nSubMeshes > 0)
			{
				m_pnSubSetIndices = new int[m_nSubMeshes];
				m_ppnSubSetIndices = new UINT*[m_nSubMeshes];

				m_ppd3dSubSetIndexBuffers = new ID3D12Resource*[m_nSubMeshes];
				m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource*[m_nSubMeshes];
				m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

				for (int i = 0; i < m_nSubMeshes; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						int nIndex = 0;
						nReads = (UINT)::fread(&nIndex, sizeof(int), 1, pInFile); //i
						nReads = (UINT)::fread(&(m_pnSubSetIndices[i]), sizeof(int), 1, pInFile);
						if (m_pnSubSetIndices[i] > 0)
						{
							m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
							nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices[i], pInFile);

							m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[i]);

							m_pd3dSubSetIndexBufferViews[i].BufferLocation = m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
							m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_pd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
}

void CStandardMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[5] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 5, pVertexBufferViews);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedMesh::CSkinnedMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
}

CSkinnedMesh::~CSkinnedMesh()
{
	if (m_pxmn4BoneIndices) delete[] m_pxmn4BoneIndices;
	if (m_pxmf4BoneWeights) delete[] m_pxmf4BoneWeights;

//	if (m_ppSkinningBoneFrameCaches) delete[] m_ppSkinningBoneFrameCaches;
	m_ppSkinningBoneFrameCaches.clear();
	if (m_ppstrSkinningBoneNames) delete[] m_ppstrSkinningBoneNames;

	if (m_pxmf4x4BindPoseBoneOffsets) delete[] m_pxmf4x4BindPoseBoneOffsets;
	if (m_pd3dcbBindPoseBoneOffsets) m_pd3dcbBindPoseBoneOffsets->Release();

	if (m_pd3dBoneIndexBuffer) m_pd3dBoneIndexBuffer->Release();
	if (m_pd3dBoneWeightBuffer) m_pd3dBoneWeightBuffer->Release();

	ReleaseShaderVariables();
}

void CSkinnedMesh::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CSkinnedMesh::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dcbBindPoseBoneOffsets)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneOffsetsGpuVirtualAddress = m_pd3dcbBindPoseBoneOffsets->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(PARAMETER_BONE_OFFSET, d3dcbBoneOffsetsGpuVirtualAddress); //Skinned Bone Offsets
	}

	if (m_pd3dcbSkinningBoneTransforms)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress = m_pd3dcbSkinningBoneTransforms->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(PARAMETER_BONE_TRANSFORM, d3dcbBoneTransformsGpuVirtualAddress); //Skinned Bone Transforms
	}

	// 뼈들의 변환 행렬 정보를 애니메이션 컨트롤러의 m_pcbxmf4x4MappedSkinningBoneTransforms에 전달
	for (int j = 0; j < m_nSkinningBones; j++)
	{
		XMStoreFloat4x4(&m_pcbxmf4x4MappedSkinningBoneTransforms[j], XMMatrixTranspose(XMLoadFloat4x4(&m_ppSkinningBoneFrameCaches[j]->m_xmf4x4World)));
	}

	// weak_ptr
	//for (int j = 0; j < m_nSkinningBones; j++)
	//{
	//	if (std::shared_ptr<CGameObject> sharedFrameCache = m_ppSkinningBoneFrameCaches[j].lock()) // shared_ptr로 변환 후 작업 수행
	//		XMStoreFloat4x4(&m_pcbxmf4x4MappedSkinningBoneTransforms[j],XMMatrixTranspose(XMLoadFloat4x4(&sharedFrameCache->m_xmf4x4World)));
	//	
	//	else 	// weak_ptr가 유효하지 않을 경우에 대한 처리 (예: 초기화 또는 로그 출력)
	//		XMStoreFloat4x4(&m_pcbxmf4x4MappedSkinningBoneTransforms[j], XMMatrixIdentity()); // 기본 값으로 초기화
	//}
}

void CSkinnedMesh::ReleaseShaderVariables()
{
}

void CSkinnedMesh::ReleaseUploadBuffers()
{
	CStandardMesh::ReleaseUploadBuffers();

	if (m_pd3dBoneIndexUploadBuffer) m_pd3dBoneIndexUploadBuffer->Release();
	m_pd3dBoneIndexUploadBuffer = NULL;

	if (m_pd3dBoneWeightUploadBuffer) m_pd3dBoneWeightUploadBuffer->Release();
	m_pd3dBoneWeightUploadBuffer = NULL;
}

void CSkinnedMesh::PrepareSkinning(std::shared_ptr<CGameObject> pModelRootObject)
{
	for (int j = 0; j < m_nSkinningBones; j++)
	{
		//m_ppSkinningBoneFrameCaches[j] = pModelRootObject->FindFrame(m_ppstrSkinningBoneNames[j]);
		m_ppSkinningBoneFrameCaches[j] = pModelRootObject->FindFrame(m_ppstrSkinningBoneNames[j]);
	}
}

void CSkinnedMesh::LoadSkinInfoFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	::ReadStringFromFile(pInFile, m_pstrMeshName); 

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<BonesPerVertex>:"))
		{
			m_nBonesPerVertex = ::ReadIntegerFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<BoneNames>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0) 
			{
				m_ppstrSkinningBoneNames = new char[m_nSkinningBones][64];
//				m_ppSkinningBoneFrameCaches = new CGameObject*[m_nSkinningBones];
				m_ppSkinningBoneFrameCaches.resize(m_nSkinningBones);
				for (int i = 0; i < m_nSkinningBones; i++)
				{
					::ReadStringFromFile(pInFile, m_ppstrSkinningBoneNames[i]);
#ifdef _WITH_DISPLAY_BONE_NAME
					DebugOutput("\nBone Name: ", m_ppstrSkinningBoneNames[i]);
#endif
					m_ppSkinningBoneFrameCaches[i] = nullptr;
					//m_ppSkinningBoneFrameCaches[i].reset();
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneOffsets>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_pxmf4x4BindPoseBoneOffsets = new XMFLOAT4X4[m_nSkinningBones];
				nReads = (UINT)::fread(m_pxmf4x4BindPoseBoneOffsets, sizeof(XMFLOAT4X4), m_nSkinningBones, pInFile);

				UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
				m_pd3dcbBindPoseBoneOffsets = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
				m_pd3dcbBindPoseBoneOffsets->Map(0, NULL, (void **)&m_pcbxmf4x4MappedBindPoseBoneOffsets);

				for (int i = 0; i < m_nSkinningBones; i++)
				{
					XMStoreFloat4x4(&m_pcbxmf4x4MappedBindPoseBoneOffsets[i], XMMatrixTranspose(XMLoadFloat4x4(&m_pxmf4x4BindPoseBoneOffsets[i])));
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneIndices>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmn4BoneIndices = new XMINT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmn4BoneIndices, sizeof(XMINT4), m_nVertices, pInFile);
				m_pd3dBoneIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmn4BoneIndices, sizeof(XMINT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneIndexUploadBuffer);

				m_d3dBoneIndexBufferView.BufferLocation = m_pd3dBoneIndexBuffer->GetGPUVirtualAddress();
				m_d3dBoneIndexBufferView.StrideInBytes = sizeof(XMINT4);
				m_d3dBoneIndexBufferView.SizeInBytes = sizeof(XMINT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BoneWeights>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmf4BoneWeights = new XMFLOAT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmf4BoneWeights, sizeof(XMFLOAT4), m_nVertices, pInFile);
				m_pd3dBoneWeightBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4BoneWeights, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneWeightUploadBuffer);

				m_d3dBoneWeightBufferView.BufferLocation = m_pd3dBoneWeightBuffer->GetGPUVirtualAddress();
				m_d3dBoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
				m_d3dBoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "</SkinningInfo>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[7] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView, m_d3dBoneIndexBufferView, m_d3dBoneWeightBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 7, pVertexBufferViews);
}