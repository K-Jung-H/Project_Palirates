#include "stdafx.h"
#include "ServerAnimLoader.h"
#include <cstdio>
#include <cstring>

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