#pragma once
#include "Object.h"
#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <stdexcept>

class CDescriptor_Heap
{
private:
    static CDescriptor_Heap* instance;
    ID3D12DescriptorHeap* CbvSrvUavDescriptorHeap = nullptr;
    bool isInitialized = false;

    D3D12_CPU_DESCRIPTOR_HANDLE         CbvCPUDescriptorStartHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE         CbvGPUDescriptorStartHandle{};
    D3D12_CPU_DESCRIPTOR_HANDLE         SrvCPUDescriptorStartHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE         SrvGPUDescriptorStartHandle{};
    D3D12_CPU_DESCRIPTOR_HANDLE         UavCPUDescriptorStartHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE         UavGPUDescriptorStartHandle{};

    D3D12_CPU_DESCRIPTOR_HANDLE		    CbvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			CbvGPUDescriptorNextHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			SrvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			SrvGPUDescriptorNextHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			UavCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			UavGPUDescriptorNextHandle;

    CDescriptor_Heap() {}

    void CreateCbvSrvUavDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews);

public:

    static void Init(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews)
    {
        if (!instance)
            instance = new CDescriptor_Heap();
        if (!instance->isInitialized) 
        {
            instance->isInitialized = true;
            instance->CreateCbvSrvUavDescriptorHeaps(pd3dDevice, nConstantBufferViews, nShaderResourceViews, nUnorderedAccessViews);
        }
    }

    static CDescriptor_Heap* Get_Instance()
    {
        if (!instance || !instance->isInitialized)
            throw std::runtime_error("CDescriptor_Heap::Init() must be called first.");
        return instance;
    }

    static void Destroy_Instance()
    {
        if (instance)
        {
            delete instance;
            instance = nullptr;
        }
    }

    static void SetDescriptorHeaps(ID3D12GraphicsCommandList* pd3dCommandList, UINT NumDescriptorHeaps);

    static ID3D12DescriptorHeap* Get_Heap_Ptr() { return Get_Instance()->CbvSrvUavDescriptorHeap; }
    static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return Get_Instance()->CbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); }
    static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return Get_Instance()->CbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart(); }

    static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return Get_Instance()->CbvCPUDescriptorStartHandle; }
    static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return Get_Instance()->CbvGPUDescriptorStartHandle; }
    static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return Get_Instance()->SrvCPUDescriptorStartHandle; }
    static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return Get_Instance()->SrvGPUDescriptorStartHandle; }
    static D3D12_CPU_DESCRIPTOR_HANDLE GetUavCPUDescriptorStartHandle() { return Get_Instance()->UavCPUDescriptorStartHandle; }
    static D3D12_GPU_DESCRIPTOR_HANDLE GetUavGPUDescriptorStartHandle() { return Get_Instance()->UavGPUDescriptorStartHandle; }

    static void CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
    static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dConstantBuffer, UINT nStride);
    static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferView(ID3D12Device* pd3dDevice, D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress, UINT nStride);
	
    static void CreateGraphicsShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
    static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat);

    //    Uitility
    //    static void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex, UINT nRootParameterStartIndex);
    //    static void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex);
     
    // 디스크립터 힙에 Compute SRV 생성 후 CTexture에 GPU 핸들 저장
    static void CreateComputeShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex);

    // 리소스와 포맷 배열로부터 Compute SRV 생성, 힙 오프셋 갱신
    static void CreateComputeShaderResourceViews(ID3D12Device* pd3dDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats);

    // 디스크립터 힙에 Compute UAV 생성 후 CTexture에 GPU 핸들 저장
    // static void CreateComputeUnorderedAccessViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex);
    static void CreateComputeUnorderedAccessViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterIndex, UINT nHandleStartIndex);

    // 지정 범위의 Compute SRV 생성, CTexture에 GPU 핸들 등록
    static void CreateComputeShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nTextureIndex, UINT nHandleIndex, UINT nDescriptorHeapIndex, UINT nDescriptors);

    // 지정 범위의 Compute UAV 생성, CTexture에 GPU 핸들 등록
    static void CreateComputeUnorderedAccessView(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nTextureIndex, UINT nHandleIndex, UINT nDescriptorHeapIndex, UINT nDescriptors);




    ~CDescriptor_Heap()
    {
        if (CbvSrvUavDescriptorHeap)
        {
            CbvSrvUavDescriptorHeap->Release();
            CbvSrvUavDescriptorHeap = nullptr;
        }
    }

    CDescriptor_Heap(const CDescriptor_Heap&) = delete;
    CDescriptor_Heap& operator=(const CDescriptor_Heap&) = delete;
    CDescriptor_Heap(CDescriptor_Heap&&) = delete;
    CDescriptor_Heap& operator=(CDescriptor_Heap&&) = delete;
};
