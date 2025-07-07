#include "stdafx.h"
#include "ServerAnimLoader.h"
#include "GameObject.h" 

using namespace DirectX;


int ReadIntegerFromFile(FILE* pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
	return(nValue);
}

float ReadFloatFromFile(FILE* pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
	return(fValue);
}

BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

void CStandardMesh::LoadMeshFromFile(FILE* pInFile)
{
	char  pstrToken[64]{};
	int   count = 0;

	/* 式式 ④渦 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式 */
	fread(&m_nVertices, sizeof(int), 1, pInFile);
	ReadStringFromFile(pInFile, m_pstrMeshName);

	/* 式式 饜贖 だ諒 瑞Щ 式式式式式式式式式式式式式式式式式式式式式 */
	while (true)
	{
		ReadStringFromFile(pInFile, pstrToken);

		/* 1. AABB Bounds ---------------------------------------------------- */
		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}

		/* 2. Positions (в熱) ---------------------------------------------- */
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			fread(&count, sizeof(int), 1, pInFile);
			if (count > 0)
			{
				m_pxmf3Positions = new XMFLOAT3[count];
				fread(m_pxmf3Positions, sizeof(XMFLOAT3), count, pInFile);
			}
		}

		/* 3. 碳в蹂 綰煙 : Colors / UV / Normals / Tangents / BiTangents ---- */
		else if (!strcmp(pstrToken, "<Colors>:") ||
			!strcmp(pstrToken, "<TextureCoords0>:") ||
			!strcmp(pstrToken, "<TextureCoords1>:") ||
			!strcmp(pstrToken, "<Normals>:") ||
			!strcmp(pstrToken, "<Tangents>:") ||
			!strcmp(pstrToken, "<BiTangents>:"))
		{
			fread(&count, sizeof(int), 1, pInFile);

			size_t elemSize = (!strcmp(pstrToken, "<Colors>:")) ? sizeof(XMFLOAT4) :
				(!strcmp(pstrToken, "<TextureCoords0>:")) ||
				(!strcmp(pstrToken, "<TextureCoords1>:")) ? sizeof(XMFLOAT2) :
				sizeof(XMFLOAT3);

			if (count > 0) fseek(pInFile, elemSize * count, SEEK_CUR);
		}

		/* 4. 檣策蝶(瞪羹 鳴陝⑽ 檣策蝶) ------------------------------------- */
		else if (!strcmp(pstrToken, "<Indices>:"))
		{
			fread(&count, sizeof(int), 1, pInFile);
			if (count > 0) fseek(pInFile, sizeof(UINT) * count, SEEK_CUR);
		}

		/* 5. SubMesh滌 檣策蝶 ---------------------------------------------- */
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			fread(&m_nSubMeshes, sizeof(int), 1, pInFile);
			if (m_nSubMeshes > 0)
			{
				m_pnSubSetIndices = new int[m_nSubMeshes];
				m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

				for (int i = 0; i < m_nSubMeshes; ++i)
				{
					ReadStringFromFile(pInFile, pstrToken);   // "<SubMesh>:"
					fread(&count, sizeof(int), 1, pInFile);   // (unused) sub-mesh ID
					fread(&m_pnSubSetIndices[i], sizeof(int), 1, pInFile);

					int idxCnt = m_pnSubSetIndices[i];
					if (idxCnt > 0)
					{
						m_ppnSubSetIndices[i] = new UINT[idxCnt];
						fread(m_ppnSubSetIndices[i], sizeof(UINT), idxCnt, pInFile);
					}
				}
			}
		}

		/* 6. 謙猿 ----------------------------------------------------------- */
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::LoadSkinInfoFromFile(FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for (; ; )
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

			//bounding_box = new BoundingOrientedBox(m_xmf3AABBCenter, m_xmf3AABBExtents, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f });
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
					m_ppSkinningBoneFrameCaches[i] = nullptr;
					//m_nSkinningBoneIndex[i] = i;
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
			}
		}
		else if (!strcmp(pstrToken, "</SkinningInfo>"))
		{
			break;
		}
	}
}

void CLoadedModelInfo::PrepareSkinning()
{
	int nSkinnedMesh = 0;
	m_ppSkinnedMeshes.clear();
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes);

	for (int i = 0; i < m_nSkinnedMeshes; i++)
		m_ppSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
}