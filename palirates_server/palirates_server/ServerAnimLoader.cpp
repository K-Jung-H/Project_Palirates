#include "stdafx.h"
#include "ServerAnimLoader.h"
#include "GameObject.h" 

using namespace DirectX;


BYTE ReadStringSafeFromFile(FILE* pInFile, char* pstrToken, size_t bufferSize)
{
	BYTE nStrLength = 0;
	fread(&nStrLength, sizeof(BYTE), 1, pInFile);

	size_t readLen = (nStrLength >= bufferSize) ? bufferSize - 1 : nStrLength;
	fread(pstrToken, sizeof(char), readLen, pInFile);
	pstrToken[readLen] = '\0';

	if (nStrLength > readLen) {
		fseek(pInFile, nStrLength - readLen, SEEK_CUR);
	}

	return nStrLength;
}

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

void CSkinnedMesh::LoadSkinInfoFromFile(FILE* fp)
{
    char token[64]{};

    while (true)
    {
        /* --- 토큰 읽기 --------------------------------------- */
        if (ReadStringSafeFromFile(fp, token, sizeof token) == 0)
            break;                                      // EOF → 종료

        /* BonesPerVertex : 서버에서는 사용 X ------------------- */
        if (!strcmp(token, "<BonesPerVertex>:"))
        {
            int dummy = ReadIntegerFromFile(fp);
            (void)dummy;                                // 경고 억제
        }

        /* BoneNames ------------------------------------------ */
        else if (!strcmp(token, "<BoneNames>:"))
        {
            m_nSkinningBones = ReadIntegerFromFile(fp);

            m_boneNames.resize(m_nSkinningBones);
            m_boneFrameCaches.resize(m_nSkinningBones, nullptr);

            for (int i = 0; i < m_nSkinningBones; ++i)
            {
                char boneName[64]{};
                ReadStringSafeFromFile(fp, boneName, sizeof boneName);
                m_boneNames[i] = boneName;
            }
        }

        /* Bind-Pose 오프셋 행렬 -------------------------------- */
        else if (!strcmp(token, "<BoneOffsets>:"))
        {
            int count = ReadIntegerFromFile(fp);
            m_bindPoseOffsets.resize(count);

            fread(m_bindPoseOffsets.data(),
                sizeof(DirectX::XMFLOAT4X4),
                count, fp);
        }

        /* Bounds : 6-float (min/max) 건너뛰기 ------------------ */
        else if (!strcmp(token, "<Bounds>:"))
        {
            fseek(fp, sizeof(float) * 6, SEEK_CUR);
        }

        /* BoneIndices : (XMINT4 * vertex) 스킵 ---------------- */
        else if (!strcmp(token, "<BoneIndices>:"))
        {
            int verts = ReadIntegerFromFile(fp);
            fseek(fp, sizeof(DirectX::XMINT4) * verts, SEEK_CUR);
        }

        /* BoneWeights : (XMFLOAT4 * vertex) 스킵 -------------- */
        else if (!strcmp(token, "<BoneWeights>:"))
        {
            int verts = ReadIntegerFromFile(fp);
            fseek(fp, sizeof(DirectX::XMFLOAT4) * verts, SEEK_CUR);
        }

        /* 스키닝 블록 종료 ------------------------------------ */
        else if (!strcmp(token, "</SkinningInfo>"))
        {
            break;
        }

        /* 예기치 않은 토큰 → 포맷 오류 방지 ------------------- */
        else
        {
            std::cout << "[SkinLoader] Unknown token : " << token << '\n';
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