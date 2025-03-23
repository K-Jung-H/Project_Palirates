#include "stdafx.h"
#include "Descriptor_Heap.h"


CDescriptor_Heap* CDescriptor_Heap::instance = NULL;

void CDescriptor_Heap::SetDescriptorHeaps(ID3D12GraphicsCommandList* pd3dCommandList, UINT NumDescriptorHeaps)
{
    CDescriptor_Heap* instance = Get_Instance();
    pd3dCommandList->SetDescriptorHeaps(NumDescriptorHeaps, &instance->CbvSrvUavDescriptorHeap);
}

// Creates the descriptor heap for CBV, SRV, and UAV
// void CDescriptor_Heap::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
void CDescriptor_Heap::CreateCbvSrvUavDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews)
{
    CDescriptor_Heap* instance = Get_Instance();

    if (instance->CbvSrvUavDescriptorHeap)
        return;

    D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc{};
    d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs + UAVs
    d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    d3dDescriptorHeapDesc.NodeMask = 0;

    pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&instance->CbvSrvUavDescriptorHeap));

    instance->CbvCPUDescriptorStartHandle = instance->CbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    instance->CbvGPUDescriptorStartHandle = instance->CbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

    instance->SrvCPUDescriptorStartHandle.ptr = instance->CbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
    instance->SrvGPUDescriptorStartHandle.ptr = instance->CbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);

    instance->UavCPUDescriptorStartHandle.ptr = instance->SrvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nShaderResourceViews);
    instance->UavGPUDescriptorStartHandle.ptr = instance->SrvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nShaderResourceViews);

    instance->CbvCPUDescriptorNextHandle = instance->CbvCPUDescriptorStartHandle;
    instance->CbvGPUDescriptorNextHandle = instance->CbvGPUDescriptorStartHandle;

    instance->SrvCPUDescriptorNextHandle = instance->SrvCPUDescriptorStartHandle;
    instance->SrvGPUDescriptorNextHandle = instance->SrvGPUDescriptorStartHandle;

    instance->UavCPUDescriptorNextHandle = instance->UavCPUDescriptorStartHandle;
    instance->UavGPUDescriptorNextHandle = instance->UavGPUDescriptorStartHandle;



}

// Creates multiple constant buffer views
void CDescriptor_Heap::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
    CDescriptor_Heap* instance = Get_Instance();
    D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
    D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc{};
    d3dCBVDesc.SizeInBytes = nStride;

    for (int j = 0; j < nConstantBufferViews; j++)
    {
        d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
        pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, instance->CbvCPUDescriptorNextHandle);

        instance->CbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
        instance->CbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
    }
}

// Creates a single constant buffer view and returns its GPU handle
D3D12_GPU_DESCRIPTOR_HANDLE CDescriptor_Heap::CreateConstantBufferView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dConstantBuffer, UINT nStride)
{
    CDescriptor_Heap* instance = Get_Instance();
    D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc{};
    d3dCBVDesc.SizeInBytes = nStride;
    d3dCBVDesc.BufferLocation = pd3dConstantBuffer->GetGPUVirtualAddress();

    pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, instance->CbvCPUDescriptorNextHandle);

    D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = instance->CbvGPUDescriptorNextHandle;
    instance->CbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
    instance->CbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;

    return d3dCbvGPUDescriptorHandle;
}

// Creates a constant buffer view using a GPU virtual address
D3D12_GPU_DESCRIPTOR_HANDLE CDescriptor_Heap::CreateConstantBufferView(ID3D12Device* pd3dDevice, D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress, UINT nStride)
{
    CDescriptor_Heap* instance = Get_Instance();
    D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc{};
    d3dCBVDesc.SizeInBytes = nStride;
    d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress;

    pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, instance->CbvCPUDescriptorNextHandle);

    D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = instance->CbvGPUDescriptorNextHandle;
    instance->CbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
    instance->CbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;

    return d3dCbvGPUDescriptorHandle;
}


// Creates shader resource views for a texture
// Creates a more than one shader resource view
// Creates SRVs and binds to the root signature. [Root Signature Binding: O]
void CDescriptor_Heap::CreateGraphicsShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
    CDescriptor_Heap* instance = Get_Instance();

    instance->SrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
    instance->SrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

    int nTextures = pTexture->GetTextures();

    // 디버그 로그: root parameter 개수 확인
    int nRootParams = pTexture->GetGraphicsSrvRootParameters();
    assert(nRootParams > 0);

    if (nRootParams <= 0)
    {
        OutputDebugStringA("❗ Error: No graphics root parameters defined in texture!\n");
        __debugbreak();
    }

    for (int i = 0; i < nTextures; i++)
    {
        ID3D12Resource* pShaderResource = pTexture->GetResource(i);
        D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
        pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, instance->SrvCPUDescriptorNextHandle);
        instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;

        pTexture->SetGraphicsSrvGpuDescriptorHandle(i, instance->SrvGPUDescriptorNextHandle);
        instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
    }

    // 루트 파라미터 인덱스 (고정)
    // 루트 시그니처 상의 슬롯
    // GPU 핸들 시작 인덱스
    // 연속된 디스크립터 개수
    pTexture->SetGraphicsSrvRootParameter(0, nRootParameterStartIndex, 0, nTextures);
}



// Creates a single shader resource view
// Creates SRVs and binds to the root signature. [Root Signature Binding: X]
D3D12_GPU_DESCRIPTOR_HANDLE CDescriptor_Heap::CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat)
{
    CDescriptor_Heap* instance = Get_Instance();
    D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc{};
    d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    d3dShaderResourceViewDesc.Format = dxgiSrvFormat;
    d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
    d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
    d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = instance->SrvGPUDescriptorNextHandle;
    pd3dDevice->CreateShaderResourceView(pd3dResource, &d3dShaderResourceViewDesc, instance->SrvCPUDescriptorNextHandle);

    instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
    instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;

    return d3dSrvGPUDescriptorHandle;
}


void CDescriptor_Heap::CreateComputeShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex)
{
    CDescriptor_Heap* instance = Get_Instance();

    instance->SrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
    instance->SrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

    int nTextures = pTexture->GetTextures();
    for (int i = 0; i < nTextures; ++i)
    {
        ID3D12Resource* pResource = pTexture->GetResource(i);
        if (pResource)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = pTexture->GetShaderResourceViewDesc(i);
            pd3dDevice->CreateShaderResourceView(pResource, &desc, instance->SrvCPUDescriptorNextHandle);

            pTexture->SetComputeSrvGpuDescriptorHandle(i, instance->SrvGPUDescriptorNextHandle);

            instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
            instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
        }
    }
}

void CDescriptor_Heap::CreateComputeShaderResourceViews(ID3D12Device* pd3dDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats)
{
    CDescriptor_Heap* instance = Get_Instance();

    for (int i = 0; i < nResources; ++i)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Format = pdxgiSrvFormats[i];
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = 1;
        desc.Texture2D.MostDetailedMip = 0;
        desc.Texture2D.PlaneSlice = 0;
        desc.Texture2D.ResourceMinLODClamp = 0.0f;

        pd3dDevice->CreateShaderResourceView(ppd3dResources[i], &desc, instance->SrvCPUDescriptorNextHandle);

        instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
        instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
    }
}

void CDescriptor_Heap::CreateComputeUnorderedAccessViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex)
{
    CDescriptor_Heap* instance = Get_Instance();

    instance->UavCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
    instance->UavGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

    int nTextures = pTexture->GetTextures();
    for (int i = 0; i < nTextures; ++i)
    {
        ID3D12Resource* pResource = pTexture->GetResource(i);
        if (pResource)
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc = pTexture->GetUnorderedAccessViewDesc(i);
            pd3dDevice->CreateUnorderedAccessView(pResource, nullptr, &desc, instance->UavCPUDescriptorNextHandle);

            pTexture->SetComputeUavGpuDescriptorHandle(i, instance->UavGPUDescriptorNextHandle);

            instance->UavCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
            instance->UavGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
        }
    }
}

void CDescriptor_Heap::CreateComputeShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nTextureIndex, UINT nHandleIndex, UINT nDescriptorHeapIndex, UINT nDescriptors)
{
    CDescriptor_Heap* instance = Get_Instance();

    instance->SrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
    instance->SrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

    for (UINT i = 0; i < nDescriptors; ++i)
    {
        ID3D12Resource* pResource = pTexture->GetResource(nTextureIndex + i);
        if (pResource)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = pTexture->GetShaderResourceViewDesc(nTextureIndex + i);
            pd3dDevice->CreateShaderResourceView(pResource, &desc, instance->SrvCPUDescriptorNextHandle);

            pTexture->SetComputeSrvGpuDescriptorHandle(nHandleIndex + i, instance->SrvGPUDescriptorNextHandle);

            instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
            instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
        }
    }
}

void CDescriptor_Heap::CreateComputeUnorderedAccessView(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nTextureIndex, UINT nHandleIndex, UINT nDescriptorHeapIndex, UINT nDescriptors)
{
    CDescriptor_Heap* instance = Get_Instance();

    instance->UavCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
    instance->UavGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

    for (UINT i = 0; i < nDescriptors; ++i)
    {
        ID3D12Resource* pResource = pTexture->GetResource(nTextureIndex + i);
        if (pResource)
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc = pTexture->GetUnorderedAccessViewDesc(nTextureIndex + i);
            pd3dDevice->CreateUnorderedAccessView(pResource, nullptr, &desc, instance->UavCPUDescriptorNextHandle);

            pTexture->SetComputeUavGpuDescriptorHandle(nHandleIndex + i, instance->UavGPUDescriptorNextHandle);

            instance->UavCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
            instance->UavGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
        }
    }
}


//===========================================================
// 유틸리티 함수 - 힙 손상 위험 있음 - 사용 전에 검토 필요

//// Creates a single shader resource view for a specific texture index
//void CDescriptor_Heap::CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex, UINT nRootParameterStartIndex)
//{
//    CDescriptor_Heap* instance = Get_Instance();
//
//    ID3D12Resource* pShaderResource = pTexture->GetResource(nIndex);
//    D3D12_GPU_DESCRIPTOR_HANDLE d3dGpuDescriptorHandle = pTexture->GetGpuDescriptorHandle(nIndex);
//
//    if (pShaderResource && !d3dGpuDescriptorHandle.ptr)
//    {
//        D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nIndex);
//        pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, instance->SrvCPUDescriptorNextHandle);
//
//        instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
//        pTexture->SetGraphicsSrvGpuDescriptorHandle(nIndex, instance->SrvGPUDescriptorNextHandle);
//        instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
//
//        //pTexture->SetRootParameterIndex(nIndex, nRootParameterStartIndex + nIndex);
//    }
//}
//
//// Creates a single shader resource view without a root parameter index
//void CDescriptor_Heap::CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex)
//{
//    CDescriptor_Heap* instance = Get_Instance();
//
//    ID3D12Resource* pShaderResource = pTexture->GetResource(nIndex);
//    D3D12_GPU_DESCRIPTOR_HANDLE d3dGpuDescriptorHandle = pTexture->GetGpuDescriptorHandle(nIndex);
//
//    if (pShaderResource && !d3dGpuDescriptorHandle.ptr)
//    {
//        D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nIndex);
//        pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, instance->SrvCPUDescriptorNextHandle);
//
//        instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
//        pTexture->SetGraphicsSrvGpuDescriptorHandle(nIndex, instance->SrvGPUDescriptorNextHandle);
//        instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
//    }
//}