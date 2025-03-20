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

    void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);

public:

    static void Init(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
    {
        if (!instance)
            instance = new CDescriptor_Heap();
        if (!instance->isInitialized) 
        {
            instance->isInitialized = true;
            instance->CreateCbvSrvDescriptorHeaps(pd3dDevice, nConstantBufferViews, nShaderResourceViews);
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
	
    static void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
    static void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex, UINT nRootParameterStartIndex);
    static void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex);
    static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat);
     


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

//class CDescriptor_Heap
//{
//	public:
//		CDescriptor_Heap();
//		~CDescriptor_Heap();
//	
//		ID3D12DescriptorHeap* CbvSrvUavDescriptorHeap = NULL;
//	
//		D3D12_CPU_DESCRIPTOR_HANDLE			CbvCPUDescriptorStartHandle;
//		D3D12_GPU_DESCRIPTOR_HANDLE			CbvGPUDescriptorStartHandle;
//		D3D12_CPU_DESCRIPTOR_HANDLE			SrvCPUDescriptorStartHandle;
//		D3D12_GPU_DESCRIPTOR_HANDLE			SrvGPUDescriptorStartHandle;
//		D3D12_CPU_DESCRIPTOR_HANDLE			UavCPUDescriptorStartHandle;
//		D3D12_GPU_DESCRIPTOR_HANDLE			UavGPUDescriptorStartHandle;
//	
//		D3D12_CPU_DESCRIPTOR_HANDLE			CbvCPUDescriptorNextHandle;
//		D3D12_GPU_DESCRIPTOR_HANDLE			CbvGPUDescriptorNextHandle;
//		D3D12_CPU_DESCRIPTOR_HANDLE			SrvCPUDescriptorNextHandle;
//		D3D12_GPU_DESCRIPTOR_HANDLE			SrvGPUDescriptorNextHandle;
//		D3D12_CPU_DESCRIPTOR_HANDLE			UavCPUDescriptorNextHandle;
//		D3D12_GPU_DESCRIPTOR_HANDLE			UavGPUDescriptorNextHandle;
//	
//		static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(CbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
//		static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(CbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }
//	
//	
//	
//		static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(CbvCPUDescriptorStartHandle); }
//		static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(CbvGPUDescriptorStartHandle); }
//		static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(SrvCPUDescriptorStartHandle); }
//		static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(SrvGPUDescriptorStartHandle); }
//		static D3D12_CPU_DESCRIPTOR_HANDLE GetUavCPUDescriptorStartHandle() { return(UavCPUDescriptorStartHandle); }
//		static D3D12_GPU_DESCRIPTOR_HANDLE GetUavGPUDescriptorStartHandle() { return(UavGPUDescriptorStartHandle); }
//	
//		static void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);
//		static void CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
//		static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dConstantBuffer, UINT nStride);
//		static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferView(ID3D12Device* pd3dDevice, D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress, UINT nStride);
//	
//		static void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
//		static void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex, UINT nRootParameterStartIndex);
//		static void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex);
//		static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat);
//	
//
//};
//
