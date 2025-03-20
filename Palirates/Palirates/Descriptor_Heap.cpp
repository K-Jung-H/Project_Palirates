#include "stdafx.h"
#include "Descriptor_Heap.h"


CDescriptor_Heap* CDescriptor_Heap::instance = NULL;

void CDescriptor_Heap::SetDescriptorHeaps(ID3D12GraphicsCommandList* pd3dCommandList, UINT NumDescriptorHeaps)
{
    CDescriptor_Heap* instance = Get_Instance();
    pd3dCommandList->SetDescriptorHeaps(NumDescriptorHeaps, &instance->CbvSrvUavDescriptorHeap);
}

// Creates the descriptor heap for CBV, SRV, and UAV
void CDescriptor_Heap::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
    CDescriptor_Heap* instance = Get_Instance();

    if (instance->CbvSrvUavDescriptorHeap) return;

    D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc{};
    d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews;
    d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    d3dDescriptorHeapDesc.NodeMask = 0;

    pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&instance->CbvSrvUavDescriptorHeap));

    instance->CbvCPUDescriptorStartHandle = instance->CbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    instance->CbvGPUDescriptorStartHandle = instance->CbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

    instance->SrvCPUDescriptorStartHandle.ptr = instance->CbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
    instance->SrvGPUDescriptorStartHandle.ptr = instance->CbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

    instance->CbvCPUDescriptorNextHandle = instance->CbvCPUDescriptorStartHandle;
    instance->CbvGPUDescriptorNextHandle = instance->CbvGPUDescriptorStartHandle;
    instance->SrvCPUDescriptorNextHandle = instance->SrvCPUDescriptorStartHandle;
    instance->SrvGPUDescriptorNextHandle = instance->SrvGPUDescriptorStartHandle;
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

        instance->CbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
        instance->CbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
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
    instance->CbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
    instance->CbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

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
    instance->CbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
    instance->CbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

    return d3dCbvGPUDescriptorHandle;
}

// Creates shader resource views for a texture
void CDescriptor_Heap::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
    CDescriptor_Heap* instance = Get_Instance();

    instance->SrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
    instance->SrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

    int nTextures = pTexture->GetTextures();
    for (int i = 0; i < nTextures; i++)
    {
        ID3D12Resource* pShaderResource = pTexture->GetResource(i);
        D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
        pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, instance->SrvCPUDescriptorNextHandle);
        instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

        pTexture->SetGpuDescriptorHandle(i, instance->SrvGPUDescriptorNextHandle);
        instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
    }

    int nRootParameters = pTexture->GetRootParameters();
    for (int i = 0; i < nRootParameters; i++) 
        pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);

}

// Creates a single shader resource view for a specific texture index
void CDescriptor_Heap::CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex, UINT nRootParameterStartIndex)
{
    CDescriptor_Heap* instance = Get_Instance();

    ID3D12Resource* pShaderResource = pTexture->GetResource(nIndex);
    D3D12_GPU_DESCRIPTOR_HANDLE d3dGpuDescriptorHandle = pTexture->GetGpuDescriptorHandle(nIndex);

    if (pShaderResource && !d3dGpuDescriptorHandle.ptr)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nIndex);
        pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, instance->SrvCPUDescriptorNextHandle);

        instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
        pTexture->SetGpuDescriptorHandle(nIndex, instance->SrvGPUDescriptorNextHandle);
        instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

        pTexture->SetRootParameterIndex(nIndex, nRootParameterStartIndex + nIndex);
    }
}

// Creates a single shader resource view without a root parameter index
void CDescriptor_Heap::CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex)
{
    CDescriptor_Heap* instance = Get_Instance();

    ID3D12Resource* pShaderResource = pTexture->GetResource(nIndex);
    D3D12_GPU_DESCRIPTOR_HANDLE d3dGpuDescriptorHandle = pTexture->GetGpuDescriptorHandle(nIndex);

    if (pShaderResource && !d3dGpuDescriptorHandle.ptr)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nIndex);
        pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, instance->SrvCPUDescriptorNextHandle);

        instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
        pTexture->SetGpuDescriptorHandle(nIndex, instance->SrvGPUDescriptorNextHandle);
        instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
    }
}


// Creates a single shader resource view
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

    instance->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
    instance->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

    return d3dSrvGPUDescriptorHandle;
}








//void CDescriptor_Heap::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
//{
//	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
//	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
//	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//	d3dDescriptorHeapDesc.NodeMask = 0;
//	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&CbvSrvUavDescriptorHeap);
//
//	m_pDescriptorHeap->CbvCPUDescriptorStartHandle = m_pDescriptorHeap->CbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//	m_pDescriptorHeap->CbvGPUDescriptorStartHandle = m_pDescriptorHeap->CbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
//	m_pDescriptorHeap->SrvCPUDescriptorStartHandle.ptr = m_pDescriptorHeap->CbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
//	m_pDescriptorHeap->SrvGPUDescriptorStartHandle.ptr = m_pDescriptorHeap->CbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
//
//	m_pDescriptorHeap->CbvCPUDescriptorNextHandle = m_pDescriptorHeap->CbvCPUDescriptorStartHandle;
//	m_pDescriptorHeap->CbvGPUDescriptorNextHandle = m_pDescriptorHeap->CbvGPUDescriptorStartHandle;
//	m_pDescriptorHeap->SrvCPUDescriptorNextHandle = m_pDescriptorHeap->SrvCPUDescriptorStartHandle;
//	m_pDescriptorHeap->SrvGPUDescriptorNextHandle = m_pDescriptorHeap->SrvGPUDescriptorStartHandle;
//}
//
//void CDescriptor_Heap::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
//{
//	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
//	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
//	d3dCBVDesc.SizeInBytes = nStride;
//	for (int j = 0; j < nConstantBufferViews; j++)
//	{
//		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
//		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->CbvCPUDescriptorNextHandle);
//		m_pDescriptorHeap->CbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//		m_pDescriptorHeap->CbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//	}
//}
//
//D3D12_GPU_DESCRIPTOR_HANDLE CDescriptor_Heap::CreateConstantBufferView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dConstantBuffer, UINT nStride)
//{
//	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
//	d3dCBVDesc.SizeInBytes = nStride;
//	d3dCBVDesc.BufferLocation = pd3dConstantBuffer->GetGPUVirtualAddress();
//	pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->CbvCPUDescriptorNextHandle);
//	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_pDescriptorHeap->CbvGPUDescriptorNextHandle;
//	m_pDescriptorHeap->CbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//	m_pDescriptorHeap->CbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//
//	return(d3dCbvGPUDescriptorHandle);
//}
//
//D3D12_GPU_DESCRIPTOR_HANDLE CDescriptor_Heap::CreateConstantBufferView(ID3D12Device* pd3dDevice, D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress, UINT nStride)
//{
//	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
//	d3dCBVDesc.SizeInBytes = nStride;
//	d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress;
//	pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->CbvCPUDescriptorNextHandle);
//	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_pDescriptorHeap->CbvGPUDescriptorNextHandle;
//	m_pDescriptorHeap->CbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//	m_pDescriptorHeap->CbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//
//	return(d3dCbvGPUDescriptorHandle);
//}
//
//void CDescriptor_Heap::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
//{
//	m_pDescriptorHeap->SrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
//	m_pDescriptorHeap->SrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
//
//	int nTextures = pTexture->GetTextures();
//	for (int i = 0; i < nTextures; i++)
//	{
//		ID3D12Resource* pShaderResource = pTexture->GetResource(i);
//		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
//		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_pDescriptorHeap->SrvCPUDescriptorNextHandle);
//		m_pDescriptorHeap->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//
//		pTexture->SetGpuDescriptorHandle(i, m_pDescriptorHeap->SrvGPUDescriptorNextHandle);
//		m_pDescriptorHeap->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//	}
//	int nRootParameters = pTexture->GetRootParameters();
//	for (int i = 0; i < nRootParameters; i++) pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);
//}
//
//void CDescriptor_Heap::CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex, UINT nRootParameterStartIndex)
//{
//	ID3D12Resource* pShaderResource = pTexture->GetResource(nIndex);
//	D3D12_GPU_DESCRIPTOR_HANDLE d3dGpuDescriptorHandle = pTexture->GetGpuDescriptorHandle(nIndex);
//	if (pShaderResource && !d3dGpuDescriptorHandle.ptr)
//	{
//		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nIndex);
//		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_pDescriptorHeap->SrvCPUDescriptorNextHandle);
//		m_pDescriptorHeap->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//
//		pTexture->SetGpuDescriptorHandle(nIndex, m_pDescriptorHeap->SrvGPUDescriptorNextHandle);
//		m_pDescriptorHeap->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//
//		pTexture->SetRootParameterIndex(nIndex, nRootParameterStartIndex + nIndex);
//	}
//}
//
//void CDescriptor_Heap::CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex)
//{
//	ID3D12Resource* pShaderResource = pTexture->GetResource(nIndex);
//	D3D12_GPU_DESCRIPTOR_HANDLE d3dGpuDescriptorHandle = pTexture->GetGpuDescriptorHandle(nIndex);
//	if (pShaderResource && !d3dGpuDescriptorHandle.ptr)
//	{
//		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nIndex);
//		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_pDescriptorHeap->SrvCPUDescriptorNextHandle);
//		m_pDescriptorHeap->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//
//		pTexture->SetGpuDescriptorHandle(nIndex, m_pDescriptorHeap->SrvGPUDescriptorNextHandle);
//		m_pDescriptorHeap->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//	}
//}
//
//D3D12_GPU_DESCRIPTOR_HANDLE CDescriptor_Heap::CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat)
//{
//	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
//	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	d3dShaderResourceViewDesc.Format = dxgiSrvFormat;
//	d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//	d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
//	d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
//	d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
//	d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
//	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_pDescriptorHeap->SrvGPUDescriptorNextHandle;
//	pd3dDevice->CreateShaderResourceView(pd3dResource, &d3dShaderResourceViewDesc, m_pDescriptorHeap->SrvCPUDescriptorNextHandle);
//	m_pDescriptorHeap->SrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//	m_pDescriptorHeap->SrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
//
//	return(d3dSrvGPUDescriptorHandle);
//}