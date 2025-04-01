//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include <iostream>
#include <sstream>
#include <iomanip>


CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < N_SwapChainBuffers; i++) 
		ptr_SwapChainBackBuffer_List[i] = NULL;
	SwapChainBuffer_Index = 0;

	p_CommandQueue = NULL;

	Compute_CommandAllocator = NULL;
	Compute_CommandList = NULL;

	Render_CommandAllocator = NULL;
	Render_CommandList = NULL;


	ptr_Rtv_DescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < N_SwapChainBuffers; i++) 
		m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pPlayer = NULL;

	_tcscpy_s(m_pszFrameRate, _T("Palirates - ("));

	isRunning = false;
}

CGameFramework::~CGameFramework()
{
	Disconnect();
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	CoInitialize(NULL);

	CDescriptor_Heap::Init(m_pd3dDevice, 0, 70, 10);
	//CDescriptor_Heap::CreateCbvSrvDescriptorHeaps(m_pd3dDevice, 0, 70);
	

	scene_manager = new Scene_Manager(N_SwapChainBuffers, m_pd3dDevice, p_CommandQueue, ptr_SwapChainBackBuffer_List, m_nWndClientWidth, m_nWndClientHeight);

	


	Build_Scenes();

	return(true);
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1 **)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = N_SwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(p_CommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);
#endif
	SwapChainBuffer_Index = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void **)&m_pdxgiFactory);

	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice))) break;
	}

	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void **)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice);
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);
	for (UINT i = 0; i < N_SwapChainBuffers; i++) 
		m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvUavDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	if (pd3dAdapter) 
		pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&p_CommandQueue);

	//==============================================================================================
	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&Compute_CommandAllocator);
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Compute_CommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&Compute_CommandList);
	hResult = Compute_CommandList->Close();

	//==============================================================================================
	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&Render_CommandAllocator);
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Render_CommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&Render_CommandList);
	hResult = Render_CommandList->Close();

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&Post_CommandAllocator);
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Post_CommandAllocator, nullptr, __uuidof(ID3D12GraphicsCommandList), (void**)&Post_CommandList);
	hResult = Post_CommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = N_SwapChainBuffers + 5;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&ptr_Rtv_DescriptorHeap);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = ptr_Rtv_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < N_SwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&ptr_SwapChainBackBuffer_List[i]);
		m_pd3dDevice->CreateRenderTargetView(ptr_SwapChainBackBuffer_List[i], &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle); 
		SwapChainBack_Buffer_RTV_CPUHandle_list[i] = d3dRtvCPUDescriptorHandle;
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
	
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	DsvDescriptorCPUHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, DsvDescriptorCPUHandle);
}

void CGameFramework::ChangeSwapChainState()
{
//	WaitForGpuComplete();
	WaitForGpuComplete(GPU_Stage::Render);

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < N_SwapChainBuffers; i++)
		if (ptr_SwapChainBackBuffer_List[i])
			ptr_SwapChainBackBuffer_List[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(N_SwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	SwapChainBuffer_Index = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CScene* main_scene = scene_manager->Get_Active_Scene_Ptr();

	if (main_scene) main_scene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			::SetCapture(hWnd);
			::GetCursorPos(&m_ptOldCursorPos);
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			::ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			break;
		default:
			break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CScene* main_scene = scene_manager->Get_Active_Scene_Ptr();

	if (main_scene) main_scene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					::PostQuitMessage(0);
					break;

				case VK_SPACE:
					//scene_manager->Load_Scene("Scene_2");
					//Object_Manager::Reserve_Update();
					break;

				case VK_RETURN:
					break;
				case VK_F1:
				case VK_F2:
				case VK_F3:
					m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
					break;
				case VK_F9:
					ChangeSwapChainState();
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
				m_GameTimer.Stop();
			else
				m_GameTimer.Start();
			break;
		}
		case WM_SIZE:
			break;
		case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
            break;

        case WM_KEYDOWN:
        case WM_KEYUP:
		case WM_CHAR:
			OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
	}
	return(0);
}

void CGameFramework::OnDestroy()
{
	Release_Scenes();

	delete MRT_shader;

	::CloseHandle(m_hFenceEvent);

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < N_SwapChainBuffers; i++) 
		if (ptr_SwapChainBackBuffer_List[i]) 
			ptr_SwapChainBackBuffer_List[i]->Release();
	
	if (ptr_Rtv_DescriptorHeap) 
		ptr_Rtv_DescriptorHeap->Release();

	if (p_CommandQueue) p_CommandQueue->Release();
	
	if (Compute_CommandAllocator) Compute_CommandAllocator->Release();
	if (Compute_CommandList) Compute_CommandList->Release();

	if (Render_CommandAllocator) Render_CommandAllocator->Release();
	if (Render_CommandList) Render_CommandList->Release();


	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
    if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

#ifdef WRITE_TEXT_UI
	delete text_ui_renderer;
#endif;

#if defined(_DEBUG)
	IDXGIDebug1	*pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void **)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

#define _WITH_TERRAIN_PLAYER

void CGameFramework::CreateShaderVariables()
{
	UINT ncbElementBytes = ((sizeof(CB_FRAMEWORK_INFO) + 255) & ~255); //256의 배수
	FrameworkInfo = ::CreateBufferResource(m_pd3dDevice, Active_CommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	FrameworkInfo->Map(0, NULL, (void**)&MappedFrameworkInfo);

}

void CGameFramework::UpdateShaderVariables()
{
	MappedFrameworkInfo->m_fCurrentTime = m_GameTimer.GetTotalTime();
	MappedFrameworkInfo->m_fElapsedTime = m_GameTimer.GetTimeElapsed();


	MappedFrameworkInfo->m_fSecondsPerFirework = 1.0f;
	MappedFrameworkInfo->m_nFlareParticlesToEmit = 10;
	MappedFrameworkInfo->m_xmf3Gravity = XMFLOAT3(0.0f, -9.8f, 0.0f);
	MappedFrameworkInfo->m_nMaxFlareType2Particles = 10;

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = FrameworkInfo->GetGPUVirtualAddress();
	Active_CommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_FRAME_CBV_INDEX, d3dGpuVirtualAddress);

}

void CGameFramework::ReleaseShaderVariables()
{
	if (FrameworkInfo)
	{
		FrameworkInfo->Unmap(0, NULL);
		FrameworkInfo->Release();
	}
}


void CGameFramework::Build_Scenes()
{
	BeginGPUStage(GPU_Stage::Render);

	CreateShaderVariables();

	//==========================================
	// Multi - Render Target Shader
	MRT_shader = new G_BufferMerger_Shader();
	MRT_shader->CreateShader(m_pd3dDevice, NULL, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = ptr_Rtv_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * N_SwapChainBuffers);

	MRT_shader->CreateResourcesAndRtvsSrvs(m_pd3dDevice, Active_CommandList, RTV_Format_Num, RenderTarget_Config::RTV_FORMATS, d3dRtvCPUDescriptorHandle); 


	D3D12_GPU_DESCRIPTOR_HANDLE d3dDsvGPUDescriptorHandle = CDescriptor_Heap::CreateShaderResourceView(m_pd3dDevice, m_pd3dDepthStencilBuffer, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);


	
	scene_manager->Set_Shader(MRT_shader);

	// 후처리 담당 전용 루트 시그니처 만들기 static으로 선언해서 부모객체에 박아놓기.
	Post_ComputeShader::CreateComputeRootSignature(m_pd3dDevice);
	
	for (UINT i = 0; i < N_SwapChainBuffers; ++i)
	{
		Post_ComputeShader::CreateBackBufferSRV(m_pd3dDevice, ptr_SwapChainBackBuffer_List[i], i, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	post_shader = new CEdgeDetectCSShader();
	post_shader->CreateShader(m_pd3dDevice);
	
	fullscreen_shader = new CTextureToFullScreenShader();
	fullscreen_shader->CreateShader(m_pd3dDevice);
	//==========================================



	//========================================================
	std::shared_ptr<CScene> Scene_1 = std::make_shared<CScene>();
	scene_manager->Register_Scene("Scene_1", Scene_1);
	scene_manager->Build_Scene("Scene_1", m_pd3dDevice, Active_CommandList);

	//std::shared_ptr<Test_Scene> Scene_2 = std::make_shared<Test_Scene>();
	//scene_manager->Register_Scene("Scene_2", Scene_2);
	//scene_manager->Build_Scene("Scene_2", m_pd3dDevice, Active_CommandList);


	CScene* test_scene_ptr = scene_manager->Load_Scene("Scene_1").get();
	CTerrainPlayer* pPlayer = new CTerrainPlayer(m_pd3dDevice, Active_CommandList, test_scene_ptr->GetGraphicsRootSignature(), test_scene_ptr->m_pTerrain.get());

	m_pPlayer = pPlayer;
	scene_manager->Set_Scene_Player("Scene_1", m_pPlayer);
//	scene_manager->Set_Scene_Player("Scene_2", m_pPlayer);

	m_pCamera = m_pPlayer->GetCamera();

	//========================================================

	EndGPUStage(GPU_Stage::Render);
	WaitForGpuComplete(GPU_Stage::Render);

	scene_manager->ReleaseUploadBuffers();

	if (m_pPlayer)
		m_pPlayer->ReleaseUploadBuffers();

	m_GameTimer.Reset();

}

void CGameFramework::Release_Scenes()
{
	//if (m_pPlayer) 
	//	delete m_pPlayer;

	delete scene_manager;
	
	ReleaseShaderVariables();
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;

	CScene* main_scene = scene_manager->Get_Active_Scene_Ptr();

	if (GetKeyboardState(pKeysBuffer) && main_scene)
		bProcessedByScene = main_scene->ProcessInput(pKeysBuffer);

	if (!bProcessedByScene)
	{
		DWORD dwDirection = 0;

		if ((pKeysBuffer[VK_UP] & 0xF0) || (pKeysBuffer[0x57] & 0xF0))
			dwDirection |= DIR_FORWARD;   // 방향키 위 또는 W
		if ((pKeysBuffer[VK_DOWN] & 0xF0) || (pKeysBuffer[0x53] & 0xF0))
			dwDirection |= DIR_BACKWARD;  // 방향키 아래 또는 S
		if ((pKeysBuffer[VK_LEFT] & 0xF0) || (pKeysBuffer[0x41] & 0xF0))
			dwDirection |= DIR_LEFT;      // 방향키 왼쪽 또는 A
		if ((pKeysBuffer[VK_RIGHT] & 0xF0) || (pKeysBuffer[0x44] & 0xF0))
			dwDirection |= DIR_RIGHT;     // 방향키 오른쪽 또는 D
		if ((pKeysBuffer[VK_PRIOR] & 0xF0) || (pKeysBuffer[0x51] & 0xF0))
			dwDirection |= DIR_UP;        // Page Up 또는 Q
		if ((pKeysBuffer[VK_NEXT] & 0xF0) || (pKeysBuffer[0x45] & 0xF0))
			dwDirection |= DIR_DOWN;      // Page Down 또는 E

		m_pPlayer->GetStateMachine()->handleEvent(pKeysBuffer);

		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				if (pKeysBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			if (dwDirection) 
				m_pPlayer->Move(dwDirection, 10* 12.25f, true);
		}
	}
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
	m_pPlayer->GetStateMachine()->update(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::Update_Scene()
{
	HRESULT hResult;
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	scene_manager->Update_Active_Objects(m_pd3dDevice, Active_CommandList, fTimeElapsed);

	//==============================================================

	scene_manager->Update_Active_Particles(Active_CommandList, fTimeElapsed);

	//===============================================================

	m_pPlayer->Animate(fTimeElapsed);

}

void CGameFramework::After_Update_Scene()
{
	scene_manager->After_Update_Active_Particles(Active_CommandList);
}

void CGameFramework::BeginGPUStage(GPU_Stage stage)
{
	SafeSyncStage(stage); // 이전 작업 완료 보장

	switch (stage)
	{
	case GPU_Stage::Compute:
		Active_CommandAllocator = Compute_CommandAllocator;
		Active_CommandList = Compute_CommandList;
		break;
	case GPU_Stage::Render:
		Active_CommandAllocator = Render_CommandAllocator;
		Active_CommandList = Render_CommandList;
		break;
	case GPU_Stage::Post:
		Active_CommandAllocator = Post_CommandAllocator;
		Active_CommandList = Post_CommandList;
		break;
	}

	Active_CommandAllocator->Reset();
	Active_CommandList->Reset(Active_CommandAllocator, nullptr);
}

void CGameFramework::EndGPUStage(GPU_Stage stage, bool wait )
{
	Active_CommandList->Close();
	ID3D12CommandList* cmdLists[] = { Active_CommandList };
	p_CommandQueue->ExecuteCommandLists(1, cmdLists);

	SignalFence(stage, true);

	if (wait)
		WaitForGpuComplete(stage);
}

HRESULT CGameFramework::SignalFence(GPU_Stage stage, bool shouldAdvanceFence)
{
	UINT bufferIndex = SwapChainBuffer_Index;
	UINT64* fenceValue = nullptr;

	switch (stage)
	{
	case GPU_Stage::Compute:
		fenceValue = &m_ComputeFenceValues[bufferIndex];
		break;
	case GPU_Stage::Render:
		fenceValue = &m_RenderFenceValues[bufferIndex];
		break;
	case GPU_Stage::Post:
		fenceValue = &m_PostFenceValues[bufferIndex];
		break;
	default:
		return E_FAIL;
	}

	if (shouldAdvanceFence)
		++(*fenceValue);

	return p_CommandQueue->Signal(m_pd3dFence, *fenceValue);
}

void CGameFramework::WaitForGpuComplete(GPU_Stage stage)
{
	UINT bufferIndex = SwapChainBuffer_Index;
	UINT64 fenceValue = 0;

	switch (stage)
	{
	case GPU_Stage::Compute:
		fenceValue = m_ComputeFenceValues[bufferIndex];
		break;
	case GPU_Stage::Render:
		fenceValue = m_RenderFenceValues[bufferIndex];
		break;
	case GPU_Stage::Post:
		fenceValue = m_PostFenceValues[bufferIndex];
		break;
	default:
		return;
	}

	if (fenceValue == 0) return;

	if (m_pd3dFence->GetCompletedValue() < fenceValue)
	{
		m_pd3dFence->SetEventOnCompletion(fenceValue, m_hFenceEvent);
		WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::SafeSyncStage(GPU_Stage stage)
{
	SignalFence(stage, false); // fence 증가 없이 현재 값으로만 signal
	WaitForGpuComplete(stage);
}

UINT64 CGameFramework::GetFenceValue(GPU_Stage stage, UINT bufferIndex) const
{
	switch (stage)
	{
	case GPU_Stage::Compute: 
		return m_ComputeFenceValues[bufferIndex];

	case GPU_Stage::Render:  
		return m_RenderFenceValues[bufferIndex];

	case GPU_Stage::Post:    
		return m_PostFenceValues[bufferIndex];

	default: 
		return 0;
	}
}


void CGameFramework::MoveToNextFrame()
{
	SwapChainBuffer_Index = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	// Present 후, 다음 프레임을 위한 Render Fence 증가 및 signal
	UINT64& renderFence = m_RenderFenceValues[SwapChainBuffer_Index];
	HRESULT hResult = p_CommandQueue->Signal(m_pd3dFence, ++renderFence);

	if (m_pd3dFence->GetCompletedValue() < renderFence)
	{
		m_pd3dFence->SetEventOnCompletion(renderFence, m_hFenceEvent);
		WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::PrepareStage(GPU_Stage stage)
{
	CDescriptor_Heap::SetDescriptorHeaps(Active_CommandList, 1);

	if (stage == GPU_Stage::Render || stage == GPU_Stage::Post)
	{
		if (m_pCamera)
			m_pCamera->SetViewportsAndScissorRects(Active_CommandList);
	}
}

//#define _WITH_PLAYER_TOP

void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(100.0f);
	ProcessInput();

	// ====================== [1] Compute: Update Scene ======================
	BeginGPUStage(GPU_Stage::Compute);
	PrepareStage(GPU_Stage::Compute);
	{
		Update_Scene();
	}
	EndGPUStage(GPU_Stage::Compute);

	// 다음 Compute 단계와의 동기화 보장
	SafeSyncStage(GPU_Stage::Compute);

	// ====================== [2] Compute: After Update ======================
	BeginGPUStage(GPU_Stage::Compute);
	PrepareStage(GPU_Stage::Compute);
	{
		After_Update_Scene();
	}
	EndGPUStage(GPU_Stage::Compute);

	// ====================== [3] UI (CPU-only) ======================
#ifdef WRITE_TEXT_UI
	scene_manager->Update_UI();
#endif

	// ====================== [4] Render Phase ======================
	BeginGPUStage(GPU_Stage::Render);
	PrepareStage(GPU_Stage::Render);
	{
		auto dsvHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		Active_CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		scene_manager->Prepare_MRT_G_Buffer(Active_CommandList, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], &DsvDescriptorCPUHandle);

		scene_manager->Prepare_Render_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);
		UpdateShaderVariables();
		scene_manager->Render_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);

		if (m_pPlayer) m_pPlayer->Render(Active_CommandList, m_pCamera);

		SynchronizeResourceTransition(Active_CommandList, ptr_SwapChainBackBuffer_List[SwapChainBuffer_Index], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		Active_CommandList->OMSetRenderTargets(1, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], TRUE, nullptr);

		scene_manager->Prepare_Deffered_Render_Scene(Active_CommandList);
		scene_manager->Deffered_Render_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);
	}
	EndGPUStage(GPU_Stage::Render);

	// ====================== [5] Post Process Phase ======================
	BeginGPUStage(GPU_Stage::Post);
	PrepareStage(GPU_Stage::Post);
	{
		Active_CommandList->OMSetRenderTargets(1, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], TRUE, nullptr);

		post_shader->OnPrepareRender(Active_CommandList);
		post_shader->Set_BackBuffer_SRV(Active_CommandList, SwapChainBuffer_Index);
		post_shader->Set_RootSignature_SRV(Active_CommandList, 1, MRT_shader->GetTexture()[0].GetGraphicsSrvGpuDescriptorHandle(4));
		post_shader->Dispatch(Active_CommandList);

		SynchronizeResourceTransition(Active_CommandList, post_shader->GetOutputTextureResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		fullscreen_shader->OnPrepareRender(Active_CommandList);
		fullscreen_shader->Set_SRV_ScreenTexture(Active_CommandList, post_shader->GetOutputTextureSRV());
		fullscreen_shader->Render(Active_CommandList);

		SynchronizeResourceTransition(Active_CommandList, post_shader->GetOutputTextureResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		scene_manager->Post_Update_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);
		m_pPlayer->Record_Last_Pos();

#ifndef WRITE_TEXT_UI
		SynchronizeResourceTransition(Active_CommandList, ptr_SwapChainBackBuffer_List[SwapChainBuffer_Index],
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
#endif
	}
	EndGPUStage(GPU_Stage::Post);

	// ====================== [6] Text UI Rendering ======================
#ifdef WRITE_TEXT_UI
	scene_manager->Render_Scene_UI(SwapChainBuffer_Index);
#endif

	// ====================== [7] Present ======================
#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters = {};
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	// ====================== [8] Frame Sync ======================
	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 13, 37);
	SetWindowText(m_hWnd, m_pszFrameRate);
}




//==============서버================
void CGameFramework::ConnectToServer(const std::string& ip, int port)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "[ERROR] Winsock 초기화 실패" << std::endl;
		return;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		std::cerr << "[ERROR] 소켓 생성 실패: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) != 1)
	{
		std::cerr << "[ERROR] IP 주소 변환 실패: " << ip << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}
	if (connect(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "[ERROR] 서버 연결 실패: " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	std::cout << "[INFO] 서버 연결 성공 (IP: " << ip << ", 포트: " << port << ")" << std::endl;

	isRunning = true;
	networkThread = std::thread(&CGameFramework::NetworkLoop, this);
}

void CGameFramework::SendPacket()
{
	std::lock_guard<std::mutex> lock(networkMutex);

	if (!serverSocket) return;

	CPlayer* player = GetPlayer();
	if (!player) return;

	DirectX::XMFLOAT3 pos = player->GetPosition();
	int state = player->GetState();

	std::ostringstream packetStream;
	packetStream << "MOVE," << player->GetID() << ","
		<< std::fixed << std::setprecision(2) << pos.x << ","
		<< pos.y << "," << pos.z << ","
		<< state;

	std::string packet = packetStream.str();

	int bytesSent = send(serverSocket, packet.c_str(), packet.size(), 0);
	if (bytesSent == SOCKET_ERROR)
	{
		std::cerr << "[ERROR] send() 실패: " << WSAGetLastError() << std::endl;
	}
	else
	{
		std::cout << "[INFO] 서버로 패킷 전송 완료: " << packet << std::endl;
	}
}

std::string CGameFramework::ReceiveData()
{
	char buffer[1024];
	int bytesReceived = recv(serverSocket, buffer, sizeof(buffer), 0);

	if (bytesReceived > 0)
	{
		std::string receivedData(buffer, bytesReceived);

		int playerId;
		float px, py, pz;
		int state;

		if (sscanf_s(receivedData.c_str(), "PLAYER_UPDATE,%d,%f,%f,%f,%d",
			&playerId, &px, &py, &pz, &state) == 5)
		{
			Scene_Manager& sceneManager = GetSceneManager();
			CPlayer* player = sceneManager.GetPlayerById(playerId);
			if (player)
			{
				player->SetPosition(DirectX::XMFLOAT3(px, py, pz));
				player->SetState(state);
			}
		}
		return receivedData;
	}

	return "";
}

void CGameFramework::Disconnect()
{
	isRunning = false;
	if (networkThread.joinable())
		networkThread.join();
	closesocket(serverSocket);
	WSACleanup();

}

void CGameFramework::NetworkLoop()
{
	while (isRunning)
	{
		SendPacket();
		std::this_thread::sleep_for(std::chrono::milliseconds(33));

		char buffer[1024];
		int bytesReceived = recv(serverSocket, buffer, sizeof(buffer), 0);

		if (bytesReceived > 0)
		{
			buffer[bytesReceived] = '\0'; 
			std::string receivedData(buffer);
			std::cout << "[INFO] 서버로부터 수신된 데이터: " << receivedData << std::endl;
		}
		else if (bytesReceived == 0)
		{
			std::cerr << "[INFO] 서버와의 연결 종료" << std::endl;
			isRunning = false;
			break;
		}
		else
		{
			std::cerr << "[ERROR] recv() 실패: " << WSAGetLastError() << std::endl;
		}
	}
}
