#include "stdafx.h"
#include "ServerAnimLoader.h"
#include "GameObject.h" 

using namespace DirectX;


BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';
	return(nStrLength);
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

void CLoadedModelInfo::PrepareSkinning()
{
	int nSkinnedMesh = 0;
	m_ppSkinnedMeshes.clear();
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes);

	for (int i = 0; i < m_nSkinnedMeshes; i++)
		m_ppSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
}