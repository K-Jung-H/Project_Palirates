//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"
#include "GameFramework.h"
#include "Object_Manager.h"

std::unordered_map<int, RemotePlayer> remotePlayers;

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

	CDescriptor_Heap::Init(m_pd3dDevice, 0, 200, 400, 50);
	Light_Material_Manager::Initialize();

	scene_manager = new Scene_Manager(N_SwapChainBuffers, m_pd3dDevice, p_CommandQueue, ptr_SwapChainBackBuffer_List, m_nWndClientWidth, m_nWndClientHeight);

	ConnectToServer(SERVER_IP, SERVER_PORT);
	StartPingThread();
	SendPacket_String("ENTER_SCENE,Character_Select\n");

	Build_Default_Elements();
	Build_Default_Scenes();
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
	d3dDescriptorHeapDesc.NumDescriptors = N_SwapChainBuffers +5;
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
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

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

	for (UINT i = 0; i < N_SwapChainBuffers; ++i)
	{
		Post_ComputeShader::CreateBackBufferSRV(m_pd3dDevice, ptr_SwapChainBackBuffer_List[i], i, DXGI_FORMAT_R8G8B8A8_UNORM);
	}
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CScene* main_scene = scene_manager->Get_Active_Scene_Ptr();

	if (main_scene) main_scene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	if (main_scene) main_scene->UpdateUIHoverState(hWnd);

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
				case VK_SPACE:
					test_button = !test_button;
					break;

				case VK_RETURN:
					break;
				case VK_F1:
				case VK_F2:
				case VK_F3:
				{
					shared_ptr<CCamera> temp = scene_manager->Get_Active_Scene_Player()->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
					scene_manager->Get_Active_Scene_Player()->SetCamera(temp);
					scene_manager->Set_Active_Scene_Main_Camera(temp);
				}
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
		case WM_MOUSEWHEEL:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
			break;
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
			OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			if (nMessageID == WM_KEYDOWN && wParam == 'U') {
				m_pPlayer->GetStateMachine()->changeState(State::Get_Hit_F2, Key_Value::None);
			}
			break;
	}
	return(0);
}

void CGameFramework::OnDestroy()
{
	Disconnect();


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
	UINT ncbElementBytes = ((sizeof(CB_FRAMEWORK_INFO) + 255) & ~255); 
	FrameworkInfo = ::CreateBufferResource(m_pd3dDevice, Active_CommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	FrameworkInfo->Map(0, NULL, (void**)&MappedFrameworkInfo);

}

void CGameFramework::UpdateShaderVariables()
{
	MappedFrameworkInfo->m_fCurrentTime = m_GameTimer.GetTotalTime();
	MappedFrameworkInfo->m_fElapsedTime = m_GameTimer.GetTimeElapsed();

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

void CGameFramework::Build_Default_Elements()
{
	BeginGPUStage(GPU_Stage::Render);


	post_effect_manager = new Post_Effect_Manager(m_pd3dDevice);
	post_effect_manager->CreateShaderResource(m_pd3dDevice, Active_CommandList);

	CreateShaderVariables();
	//==========================================
	// Multi - Render Target Shader
	MRT_shader = new G_BufferMerger_Shader();
	MRT_shader->CreateShader(m_pd3dDevice, NULL, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = ptr_Rtv_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * N_SwapChainBuffers);

	MRT_shader->CreateResourcesAndRtvsSrvs(m_pd3dDevice, Active_CommandList, RTV_Format_Num, RenderTarget_Config::RTV_FORMATS, d3dRtvCPUDescriptorHandle);


	D3D12_GPU_DESCRIPTOR_HANDLE d3dDsvGPUDescriptorHandle = CDescriptor_Heap::CreateShaderResourceView(m_pd3dDevice, m_pd3dDepthStencilBuffer, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

	scene_manager->Set_MRT_Shader(MRT_shader);
	

	Post_ComputeShader::CreateComputeRootSignature(m_pd3dDevice);

	for (UINT i = 0; i < N_SwapChainBuffers; ++i)
	{
		Post_ComputeShader::CreateBackBufferSRV(m_pd3dDevice, ptr_SwapChainBackBuffer_List[i], i, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	EndGPUStage(GPU_Stage::Render);
	WaitForGpuComplete(GPU_Stage::Render);
}

void CGameFramework::Build_Default_Scenes()
{
	if (!scene_manager)
		return;

	Build_Scene(Scene_Type::Lobby, "Character_Select");
	Build_Scene(Scene_Type::Board, "Game_Stage_Board");
//	Build_Scene(Scene_Type::Stage_1, "Stage_1");


//	Build_Scene(Scene_Type::Test, "Test_Scene");
//	scene_manager->Set_Active_Scene("Test_Scene");

	scene_manager->Set_Active_Scene("Character_Select");
	m_pPlayer = scene_manager->Get_Active_Scene_Player();

	//========================================================

	scene_manager->ReleaseUploadBuffers();

	if (m_pPlayer)
		m_pPlayer->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}

void CGameFramework::Build_Scene(Scene_Type scene_type, string scene_name)
{
	BeginGPUStage(GPU_Stage::Compute);

	scene_manager->Build_Scene(scene_type, scene_name, m_pd3dDevice, Active_CommandList);


	EndGPUStage(GPU_Stage::Compute);
	WaitForGpuComplete(GPU_Stage::Compute);
}

bool CGameFramework::Change_Scene()
{
	shared_ptr<CScene> active_scene = scene_manager->Get_Active_Scene();
	Change_Signal c_signal;
	if (isRunning) // 멀티인 경우에만 동작
	{
		if (Change_Call_By_Server != -1)
		{
			std::cout << "[DEBUG] 서버 패킷에 의한 씬전환 분기 진입" << std::endl;
			c_signal = change_signal;

			Change_Call_By_Server = -1;
			change_signal.change = false;

		}
	}
	else // 오프라인 인 경우
	{
		c_signal  = active_scene->Get_Change_Signal();
	}

	//==================================================

	if (!c_signal.change)
		return false;


	if (scene_manager->Find_Scene(c_signal.scene_name))
	{
		scene_manager->Set_Active_Scene(c_signal.scene_name);
		m_pPlayer = scene_manager->Get_Active_Scene_Player();
		Object_Manager::Reserve_Update();
	}
	else
	{
		BeginGPUStage(GPU_Stage::Compute);
		scene_manager->Build_Scene(c_signal.type, c_signal.scene_name, m_pd3dDevice, Active_CommandList);
		EndGPUStage(GPU_Stage::Compute);
		WaitForGpuComplete(GPU_Stage::Compute);

		scene_manager->Set_Active_Scene(c_signal.scene_name);
		m_pPlayer = scene_manager->Get_Active_Scene_Player();
		Object_Manager::Reserve_Update();
	}

	return true;
}

void CGameFramework::Release_Scenes()
{
	delete scene_manager;
	
	ReleaseShaderVariables();
}

void CGameFramework::ProcessInput()
{
	static bool last_mouse_state = false;
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;

	CScene* main_scene = scene_manager->Get_Active_Scene_Ptr();

	if (GetKeyboardState(pKeysBuffer) && main_scene)
		bProcessedByScene = main_scene->ProcessInput(pKeysBuffer);

	if (!bProcessedByScene && m_pPlayer->bIsControllable)
	{
		DWORD dwDirection = 0;
		{
			current_keyboard_inputFlags = INPUT_NONE;

			if ((pKeysBuffer[VK_UP] & 0xF0) || (pKeysBuffer[0x57] & 0xF0)) // W
			{
				current_keyboard_inputFlags |= INPUT_W;
				dwDirection |= DIR_FORWARD;
			}
			if ((pKeysBuffer[VK_DOWN] & 0xF0) || (pKeysBuffer[0x53] & 0xF0)) // S
			{
				current_keyboard_inputFlags |= INPUT_S;
				dwDirection |= DIR_BACKWARD;
			}
			if ((pKeysBuffer[VK_LEFT] & 0xF0) || (pKeysBuffer[0x41] & 0xF0)) // A
			{
				current_keyboard_inputFlags |= INPUT_A;
				dwDirection |= DIR_LEFT;
			}
			if ((pKeysBuffer[VK_RIGHT] & 0xF0) || (pKeysBuffer[0x44] & 0xF0)) // D
			{
				current_keyboard_inputFlags |= INPUT_D;
				dwDirection |= DIR_RIGHT;
			}
			if ((pKeysBuffer[VK_PRIOR] & 0xF0) || (pKeysBuffer[0x51] & 0xF0)) // Q
			{
				current_keyboard_inputFlags |= INPUT_Q;
			}
			if ((pKeysBuffer[VK_NEXT] & 0xF0) || (pKeysBuffer[0x45] & 0xF0)) // E
			{
				current_keyboard_inputFlags |= INPUT_E;
			}
			if (pKeysBuffer[VK_SHIFT] & 0xF0) // Shift
			{
				current_keyboard_inputFlags |= INPUT_SHIFT;
			}
			if (pKeysBuffer[VK_RETURN] & 0xF0) // Enter
			{
				current_keyboard_inputFlags |= INPUT_ENTER;
			}
		}


		//=======================================================================


		bool isMouseButtonDown = (pKeysBuffer[VK_LBUTTON] & 0xF0) || (pKeysBuffer[VK_RBUTTON] & 0xF0);

		if (m_pPlayer && m_pPlayer->GetCamera())
		{
			m_pPlayer->GetCamera()->SetMouseButtonHeld(isMouseButtonDown);
		}


		m_pPlayer->GetStateMachine()->handleEvent(pKeysBuffer);


		bool bMouseLocked = scene_manager->Get_Active_Scene_Mouse_State();

		if (bMouseLocked != last_mouse_state)
		{
			if (bMouseLocked)
				HideCursor();
			else
				ShowCursorFix();

			last_mouse_state = bMouseLocked;
		}

		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		POINT ptCenter = { m_nWndClientWidth / 2, m_nWndClientHeight / 2 };
		ClientToScreen(m_hWnd, &ptCenter);

		if (bMouseLocked)
		{

			if (GetCursorPos(&ptCursorPos))
			{
				cxDelta = (float)(ptCursorPos.x - ptCenter.x);
				cyDelta = (float)(ptCursorPos.y - ptCenter.y);

				SetCursorPos(ptCenter.x, ptCenter.y);
			}
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
				m_pPlayer->Move(dwDirection, 300.0f * m_GameTimer.GetTimeElapsed(), true);
		}

	}

}

void CGameFramework::Animate_Scene()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	scene_manager->Animate_Active_Objects(m_pd3dDevice, Active_CommandList, fTimeElapsed);

	//===============================================================


	if (m_pPlayer)
		m_pPlayer->Animate(fTimeElapsed);

		static bool dead = false;
		// Weapon Drop EX
		if (!dead) 
		{
			if (m_pPlayer->GetStateMachine()->Get_State() == State::Knock_Down) {
				auto sword = m_pPlayer->DropWeapon("SM_Wep_Cutlass_01");
				scene_manager->Get_Active_Scene()->obj_manager->Add_Object(sword, Object_Type::non_skinned);
				dead = true;
			}
		}
		else 
		{
			if (m_pPlayer->GetStateMachine()->Get_State() != State::Knock_Down) {
				m_pPlayer->RestoreWeapon("SM_Wep_Cutlass_01");
				dead = false;
			}
		}
}

void CGameFramework::Update_Scene()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	scene_manager->Update_Active_Objects(m_pd3dDevice, Active_CommandList);

	if (m_pPlayer)
		m_pPlayer->Update(fTimeElapsed);

}


void CGameFramework::After_Update_Scene()
{
	scene_manager->After_Update_Active_Objects();
}


void CGameFramework::BeginGPUStage(GPU_Stage stage)
{
	SafeSyncStage(stage); // Ensure previous work is completed

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
	SignalFence(stage, false); // Signal only to current value without fence increase
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
		if (scene_manager->Get_Active_Scene_Main_Camera())
			scene_manager->Get_Active_Scene_Main_Camera()->SetViewportsAndScissorRects(Active_CommandList);
	}
}


void CGameFramework::FrameAdvance()
{
	if (!scene_manager->Get_Active_Scene())
		return;

	WaitForGpuComplete(GPU_Stage::Compute);
	WaitForGpuComplete(GPU_Stage::Render);
	WaitForGpuComplete(GPU_Stage::Post);
	Change_Scene();


	m_GameTimer.Tick(100.0f);
	ProcessInput();

	BeginGPUStage(GPU_Stage::Compute);
	PrepareStage(GPU_Stage::Compute);
	{
//		std::lock_guard<std::mutex> lock(recvQueueMutex);
		while (!recvQueue.empty())
		{
			std::string receivedData = recvQueue.front();
			recvQueue.pop();

//			std::cout << "[FrameAdvance] Received packet processing: " << receivedData << std::endl;
			ProcessReceivedData(receivedData);
		}
	}




	EndGPUStage(GPU_Stage::Compute, true);

	// ====================== [1] Compute: Update Scene ======================
	BeginGPUStage(GPU_Stage::Compute);
	PrepareStage(GPU_Stage::Compute);
	{
		Animate_Scene();
	}
	EndGPUStage(GPU_Stage::Compute, true);
	// ====================== [2] Compute: After Update ======================

	BeginGPUStage(GPU_Stage::Compute);
	PrepareStage(GPU_Stage::Compute);
	{
		Update_Scene();
	}
	EndGPUStage(GPU_Stage::Compute, true);


	BeginGPUStage(GPU_Stage::Compute);
	PrepareStage(GPU_Stage::Compute);
	{
		scene_manager->Clear_Particles_Update_Result(Active_CommandList);
	}
	EndGPUStage(GPU_Stage::Compute, true);

	// ====================== [3] UI (CPU-only) ======================

	After_Update_Scene();

#ifdef WRITE_TEXT_UI
	scene_manager->Update_UI();
#endif
	// ====================== [3.5] ShadowMap Phase ======================

		for (int i = 0; i < NUM_CASCADES; i++)
		{
			{
				BeginGPUStage(GPU_Stage::Render);
				PrepareStage(GPU_Stage::Render);

				scene_manager->Prepare_Render_Scene_ShadowMap(Active_CommandList);
				scene_manager->Render_Scene_ShadowMap(m_pd3dDevice, Active_CommandList, i);

				EndGPUStage(GPU_Stage::Render, true);

			}
		}
	

	

	// ====================== [4] Render Phase ======================
	BeginGPUStage(GPU_Stage::Render);
	PrepareStage(GPU_Stage::Render);
	{
		auto dsvHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		Active_CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		scene_manager->Prepare_MRT_G_Buffer(Active_CommandList, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], &DsvDescriptorCPUHandle);

		scene_manager->Prepare_Render_Scene(m_pd3dDevice, Active_CommandList);
		UpdateShaderVariables();
		scene_manager->Render_MRT_Scene(m_pd3dDevice, Active_CommandList);

//		shared_ptr<CCamera> scene_camera = scene_manager->Get_Active_Scene_Main_Camera();
		


		SynchronizeResourceTransition(Active_CommandList, ptr_SwapChainBackBuffer_List[SwapChainBuffer_Index], 
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		
		// Merge G-Buffers
		Active_CommandList->OMSetRenderTargets(1, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], TRUE, nullptr);

		scene_manager->Prepare_Deffered_Render_Scene(Active_CommandList);
		scene_manager->Deffered_Render_Scene(m_pd3dDevice, Active_CommandList);
	}
	EndGPUStage(GPU_Stage::Render, true);

	// ====================== [5] Post Process Phase - Screen Effects ======================


	BeginGPUStage(GPU_Stage::Post);
	PrepareStage(GPU_Stage::Post);
	{
		Active_CommandList->OMSetRenderTargets(1, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], TRUE, nullptr);


		//Debuging G-Buffer
		//D3D12_GPU_DESCRIPTOR_HANDLE  Albedo_G_Buffer_SRV_handle = MRT_shader->GetTexture()[0].GetGraphicsSrvGpuDescriptorHandle(2);
		//post_effect_manager->fullscreen_shader->OnPrepareRender(Active_CommandList);
		//post_effect_manager->fullscreen_shader->Set_SRV_ScreenTexture(Active_CommandList, Albedo_G_Buffer_SRV_handle);
		//post_effect_manager->fullscreen_shader->Render(Active_CommandList);

		// Reserve Effects
		D3D12_GPU_DESCRIPTOR_HANDLE  Blur_Info_G_Buffer_SRV_handle = MRT_shader->GetTexture()[0].GetGraphicsSrvGpuDescriptorHandle(2);
		D3D12_GPU_DESCRIPTOR_HANDLE  Velocity_G_Buffer_SRV_handle = MRT_shader->GetTexture()[0].GetGraphicsSrvGpuDescriptorHandle(3);

		Resource_Bind_Set motion_blur_1 = { BLUR_INFO_SRV_ROOT_PARAMETER_INDEX, &Blur_Info_G_Buffer_SRV_handle };
		Resource_Bind_Set motion_blur_2 = { VELOCITY_SRV_ROOT_PARAMETER_INDEX, &Velocity_G_Buffer_SRV_handle };

		Resource_Bind_Set outline_blur = { BLUR_INFO_SRV_ROOT_PARAMETER_INDEX, &Blur_Info_G_Buffer_SRV_handle };
		Resource_Bind_Set zoom_blur = { BLUR_INFO_SRV_ROOT_PARAMETER_INDEX, &Blur_Info_G_Buffer_SRV_handle };

		post_effect_manager->Add_Effect(Effect_Type::Motion_Blur, motion_blur_1);
		post_effect_manager->Add_Effect(Effect_Type::Motion_Blur, motion_blur_2);

		post_effect_manager->Add_Effect(Effect_Type::Outline, outline_blur);

		if (test_button)
		{
			shared_ptr<CCamera> scene_camera = scene_manager->Get_Active_Scene_Main_Camera();

			post_effect_manager->Set_Zoom_Focus_and_Time({ 0.5, 0.3 }, m_GameTimer.GetTimeElapsed());
			post_effect_manager->Add_Effect(Effect_Type::Zoom, zoom_blur);
		}
		// Apply reserved effects
		post_effect_manager->Apply_Effect(Active_CommandList, SwapChainBuffer_Index);
		post_effect_manager->Clear_Reserved_Effect();

	}
	EndGPUStage(GPU_Stage::Post, true);
	
	// ====================== [5] Post Process Phase - Overlay Alpha Effects ======================
	BeginGPUStage(GPU_Stage::Post);
	PrepareStage(GPU_Stage::Post);
	{

		// Use previously stored depth buffer values
		auto dsvHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		Active_CommandList->OMSetRenderTargets(1, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], TRUE, &dsvHandle);


		// Rendering Transparent object
		scene_manager->Prepare_Render_Transparent_Scene(m_pd3dDevice, Active_CommandList);
		UpdateShaderVariables();
		scene_manager->Render_Transparent_Scene(m_pd3dDevice, Active_CommandList);


		// Record moving Object's Last Pos to use motion blur
		scene_manager->Post_Update_Scene(m_pd3dDevice, Active_CommandList);
		if (m_pPlayer)
		{
			m_pPlayer->Record_Last_Pos();
		}

		// Check MouseLock & FadeEffect
		scene_manager->Render_ScreenFade(m_pd3dDevice, Active_CommandList);


		scene_manager->Update_Texture_UI(m_GameTimer.GetTotalTime(), m_GameTimer.GetTimeElapsed());
		scene_manager->Render_Scene_Texture_UI(Active_CommandList, m_GameTimer.GetTotalTime(), m_GameTimer.GetTimeElapsed());

	}
	EndGPUStage(GPU_Stage::Post, true);




	// ====================== [6] Text UI Rendering ======================


	BeginGPUStage(GPU_Stage::Post);
	PrepareStage(GPU_Stage::Post);
	{
		SynchronizeResourceTransition(Active_CommandList, ptr_SwapChainBackBuffer_List[SwapChainBuffer_Index],
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

#ifdef WRITE_TEXT_UI
		scene_manager->Render_Scene_UI(SwapChainBuffer_Index);
#endif

	}
	EndGPUStage(GPU_Stage::Post);



	

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
	
	shared_ptr<Particle_Manager> active_scene_particle_manager = scene_manager->Get_Active_Scene_Particle_Manager();
	if (active_scene_particle_manager)
		active_scene_particle_manager->Destroy_Particle_Resource();

	SendPacket();

}




//==============================
void CGameFramework::ConnectToServer(const std::string& ip, int port)
{
	if (isRunning) return;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "[ERROR] Failed to initialize Winsock" << std::endl;
		return;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		std::cerr << "[ERROR] Failed to create socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	//serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) != 1)
	{
		//std::cerr << "[ERROR] Failed to convert IP address: " << ip << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}
	if (connect(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		//std::cerr << "[ERROR] Failed to connect to server: " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	std::cout << "[INFO] Successfully connected to server (IP: " << ip << ", Port: " << port << ")" << std::endl;

	isRunning = true;
	networkThread = std::thread(&CGameFramework::NetworkLoop, this);
}

void CGameFramework::SendPacket()
{
	auto active_scene = scene_manager->Get_Active_Scene();

	if (!active_scene || !bClientIdAssigned || serverSocket == INVALID_SOCKET || !m_pPlayer) return;


	std::ostringstream oss;
	oss << "PLAYER_UPDATE," << static_cast<int>(active_scene->scene_type) << "," << Client_ID;

	switch (active_scene->scene_type)
	{
	case Scene_Type::Lobby:
	{
		shared_ptr<Character_Select_Scene>characterScene = std::dynamic_pointer_cast<Character_Select_Scene>(active_scene);
		if (!characterScene)
			break;

		int selected_index = characterScene->Get_Selected_Character_Index();
		int select_status = characterScene->Get_Character_Select_Status();

		oss << "," + std::to_string(selected_index) << "," << std::to_string(select_status);
	}
	break;

	case Scene_Type::Board:
	{
		shared_ptr<Board_Scene>board_Scene = std::dynamic_pointer_cast<Board_Scene>(active_scene);
		if (!board_Scene)
			break;

		auto [selected_stage, is_selected] = board_Scene->Get_Sail_Status();

		if (is_selected)
			int a = 1;

		oss << "," << to_string(current_keyboard_inputFlags) << "," << to_string(selected_stage) << "," << (is_selected ? "1" : "0");
		
		
	}
	break;


	case Scene_Type::Stage_1:
	{
		XMFLOAT3 pos = m_pPlayer->GetPosition();
		XMFLOAT3 look = m_pPlayer->GetLookVector();

		oss << "," << to_string(current_keyboard_inputFlags) << ","
			<< to_string(pos.x) << "," << to_string(pos.y) << "," << to_string(pos.z) << "," 
			<< to_string(look.x) << "," << to_string(look.y) << "," << to_string(look.z) << ",";

		auto controller = m_pPlayer->GetSkinnedAnimationController();
		if (!controller) return;

		auto sync_Data = m_pPlayer->MakeSyncData();
		
		oss << sync_Data.track_info_list.size();

		for ( auto track_data : sync_Data.track_info_list)
		{
			oss << "," << to_string(track_data.track_index)
				<< "," << to_string(track_data.weight)
				<< "," << to_string(track_data.track_position);
		}

		oss << "," << (sync_Data.bStateChange ? "1" : "0");

		break;
	}

	case Scene_Type::Stage_2:
		break;


	default:
		std::cerr << "[ERROR] Unknown scene type in SendPacket()\n";
		return;
	}

	std::string packet = oss.str() + "\n";
	int result = SendPacket_String(packet);

	if (result == SOCKET_ERROR)
		std::cerr << "[ERROR] Failed to send PLAYER_UPDATE: " << WSAGetLastError() << std::endl;
}

int CGameFramework::SendPacket_String(const std::string& packet)
{
	if (serverSocket == INVALID_SOCKET)
	{
		std::cerr << "[ERROR] SendPacket failed: invalid socket" << std::endl;
		return serverSocket;
	}

	int result = send(serverSocket, packet.c_str(), static_cast<int>(packet.size()), 0);

//	std::cout << "[SEND] " << packet;

	if (result == SOCKET_ERROR)
	{
		std::cerr << "[ERROR] Failed to send packet: " << WSAGetLastError() << std::endl;
	}
}


void CGameFramework::ProcessReceivedData(const std::string& receivedData)
{
	if (sscanf_s(receivedData.c_str(), "CLIENT_ID,%d", &Client_ID) == 1) // Client_ID로 저장
	{
		Connected_Player_List[Client_ID] = true;
		HandleClientIdAssignment();
		return;
	}

	if (!bClientIdAssigned)
	{
		return;
	}

	std::vector<std::string> tokens;
	std::stringstream ss(receivedData);
	std::string item;
	while (std::getline(ss, item, ','))
		tokens.push_back(item);

	if (tokens.empty()) return;

	const std::string& cmd = tokens[0];

	// 공통 처리
	if (cmd == "PLAYER_LEFT_GAME " && tokens.size() >= 2)
	{
		int leaveId = std::stoi(tokens[1]);
		Multi_PlayerLeave(leaveId);
		return;
	}
	else if (cmd == "CHANGE_SCENE" && tokens.size() >= 2)
	{
		std::cout << "[CLIENT][RECV] CHANGE_SCENE " << tokens[1] << std::endl;
		HandleChangeScene(tokens);
		return;
	}

	// 씬 별 처리
	auto active_scene = scene_manager->Get_Active_Scene();
	if (!active_scene) return;

	switch (active_scene->scene_type)
	{
	case Scene_Type::Lobby:
	{
		shared_ptr<Character_Select_Scene> select_scene = dynamic_pointer_cast<Character_Select_Scene>(active_scene);
		if (!select_scene)
			break;

		ProcessReceivedData_Lobby(select_scene, cmd, tokens);		
	}
	break;

	case Scene_Type::Board:
	{
		shared_ptr<Board_Scene>board_scene = std::dynamic_pointer_cast<Board_Scene>(active_scene);
		if (!board_scene)
			break;

		ProcessReceivedData_Board(board_scene, cmd, tokens);
	}
	break;

	case Scene_Type::Stage_1:
	{
		shared_ptr<CScene>stage_scene = std::dynamic_pointer_cast<CScene>(active_scene);
		if (!stage_scene)
			break;
		if (cmd == "STAGE_1")
			ProcessReceivedData_Stage(stage_scene, cmd, tokens);  
		else if (cmd == "MONSTER_SNAPSHOT")
			ProcessReceivedData_Monster(stage_scene, tokens);
		else if (cmd == "MONSTER_COMMAND") {
			// need func
			int cmdCount = std::stoi(tokens[1]);
			int idx = 2;
			for (int i = 0; i < cmdCount; ++i) {
				std::string cmdType = tokens[idx++];
				if (cmdType == "DESPAWN") {
					int id = std::stoi(tokens[idx++]);
					stage_scene->DespawnMonster(id);
				}
			}
		}
		//ProcessReceivedData_Stage(stage_scene, cmd, tokens);
		else if (cmd == "PARTICLE_CREATE" || cmd == "PARTICLE_UPDATE" || cmd == "PARTICLE_REMOVE")
			ProcessReceivedData_Particle(stage_scene, cmd, tokens);
	}
	break;

	case Scene_Type::Stage_2:
	{
	}
	break;

	default:
		return;
	}
}


void CGameFramework::ProcessReceivedData_Lobby(shared_ptr<Character_Select_Scene> lobby_scene, const std::string& cmd, const std::vector<std::string>& tokens)
{

	if (cmd == "CHARACTER_SELECT_SCENE" && tokens.size() >= 2)
	{
		// 초기화
		for (int charId = 0; charId < MaxPlayer; ++charId)
		{
			readyClientIds[charId] = -1;
			characterSelections[charId].reset();  // 선택 여부 초기화
		}

		for (int i = 1; i + 2 < tokens.size(); i += 3)
		{
			int charId = std::stoi(tokens[i]);
			int readyClientId = std::stoi(tokens[i + 1]);
			std::string selectedRaw = tokens[i + 2];

			// Ready 정보 저장
			readyClientIds[charId] = readyClientId;

			// 선택 정보 저장
			if (selectedRaw != "-1")
			{
				std::stringstream ss(selectedRaw);
				std::string part;
				while (std::getline(ss, part, '|'))
				{
					try {
						int clientId = std::stoi(part);
						if (clientId >= 0 && clientId < MaxPlayer)
							characterSelections[charId].set(clientId);
					}
					catch (...) {
						std::cerr << "[WARN] Invalid client ID: " << part << std::endl;
					}
				}
			}
		}
		lobby_scene->SetCharacterSelections(characterSelections);
		lobby_scene->SetReadyClientIds(readyClientIds);
	}
	else if (cmd == "CHARACTER_SELECT_SUCCESS")
	{
		// 캐릭터 선택 변경 차단하기
	}
	else if (cmd == "CHARACTER_SELECT_FAIL")
	{
		// select 버튼 처리 값 초기화 하기
		//select_scene->Set_Character_Select_Status(false);
	}
}

void CGameFramework::ProcessReceivedData_Board(shared_ptr<Board_Scene> board_scene, const std::string& cmd, const std::vector<std::string>& tokens)
{
	if (cmd == "BOARD_SCENE")
	{
		if (tokens.size() < 7)
			return;

		// pos: tokens[1]~[3], look: tokens[4]~[6]
		XMFLOAT3 pos{
			std::stof(tokens[1]),
			std::stof(tokens[2]),
			std::stof(tokens[3])
		};
		XMFLOAT3 look{
			std::stof(tokens[4]),
			std::stof(tokens[5]),
			std::stof(tokens[6])
		};

		board_scene->Sync_Boat_Server(pos, look);
	}
}

void CGameFramework::ProcessReceivedData_Stage(shared_ptr<CScene> stage_scene, const std::string& command, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 2) return;

	int playerCount = std::stoi(tokens[1]);
	int startIndex = 2;

	for (int i = 0; i < playerCount; ++i)
	{
		int base = startIndex;

		if (base + 9 >= tokens.size()) break;

		int playerId = std::stoi(tokens[base + 0]);
		int modelId = std::stoi(tokens[base + 1]); 
		float px = std::stof(tokens[base + 2]);
		float py = std::stof(tokens[base + 3]);
		float pz = std::stof(tokens[base + 4]);
		float lx = std::stof(tokens[base + 5]);
		float ly = std::stof(tokens[base + 6]);
		float lz = std::stof(tokens[base + 7]);
		int trackCount = std::stoi(tokens[base + 8]);

		int trackStart = base + 9;

		int expectedTrackTokenCount = trackCount * 3;

		if (trackStart + expectedTrackTokenCount >= tokens.size()) 
			break;

		std::vector<Animation_Sync> track_list;

		for (int t = 0; t < trackCount; ++t)
		{
			int idx = trackStart + t * 3;
			int trackIdx = std::stoi(tokens[idx]);
			float weight = std::stof(tokens[idx + 1]);
			float position = std::stof(tokens[idx + 2]);
			track_list.push_back({ trackIdx, weight, position });
		}

		int stateFlagIndex = trackStart + expectedTrackTokenCount;
		if (stateFlagIndex >= tokens.size()) break;

		bool stateChanged = (tokens[stateFlagIndex] == "1");

		ServerSyncData syncData;
		syncData.position = XMFLOAT3(px, py, pz);
		syncData.lookVector = XMFLOAT3(lx, ly, lz);

		syncData.track_info_list = track_list;
		syncData.bStateChange = stateChanged;

		HandlePlayerSync(playerId, modelId, syncData);

		startIndex = stateFlagIndex + 1;
	}
}

void CGameFramework::ProcessReceivedData_Monster(std::shared_ptr<CScene> stage_scene, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 3) return;
	float list_size = std::stof(tokens[1]);

	int startIndex = 2;
	for (int i = 0; i<int(list_size); ++i) 
	{
		int base = startIndex;

		int monsterId = std::stoi(tokens[base + 0]);
		float px = std::stof(tokens[base + 1]);
		float py = std::stof(tokens[base + 2]);
		float pz = std::stof(tokens[base + 3]);
		float lx = std::stof(tokens[base + 4]);
		float ly = std::stof(tokens[base + 5]);
		float lz = std::stof(tokens[base + 6]);
		int trackCount = std::stoi(tokens[base + 7]);

		int trackStart = base + 8;

		int expectedTrackTokenCount = trackCount * 3;

		std::vector<Animation_Sync> track_list;

		for (int t = 0; t < trackCount; ++t)
		{
			int idx = trackStart + t * 3;
			int trackIdx = std::stoi(tokens[idx]);
			float weight = std::stof(tokens[idx + 1]);
			float position = std::stof(tokens[idx + 2]);
			track_list.push_back({ trackIdx, weight, position });
		}

		int stateFlagIndex = trackStart + expectedTrackTokenCount;

		ServerSyncData syncData;
		syncData.position = XMFLOAT3(px, py, pz);
		syncData.lookVector = XMFLOAT3(lx, ly, lz);

		syncData.track_info_list = track_list;
		syncData.bStateChange = std::stoi(tokens[stateFlagIndex]);


		stage_scene->Sync_Monster_Data(m_pd3dDevice, Active_CommandList, monsterId, syncData);

		startIndex = stateFlagIndex + 1;
	}
}

void CGameFramework::ProcessReceivedData_Particle(shared_ptr<CScene> stage_scene, const std::string& command, const std::vector<std::string>& tokens)
{
	if (command == "PARTICLE_CREATE" || command == "PARTICLE_UPDATE")
	{
		int count = std::stoi(tokens[1]);
		int base = 2;


		for (int i = 0; i < count; ++i)
		{
			int idx = base + i * 15;
			if (idx + 14 >= tokens.size()) break;

			Particle_Sync_Data particle_sync_data;

			UINT id = std::stoi(tokens[idx]);
			Particle_Type type = static_cast<Particle_Type>(std::stoi(tokens[idx + 1]));
			XMFLOAT3 pos{ std::stof(tokens[idx + 2]), std::stof(tokens[idx + 3]), std::stof(tokens[idx + 4]) };
			XMFLOAT3 look{ std::stof(tokens[idx + 5]), std::stof(tokens[idx + 6]), std::stof(tokens[idx + 7]) };
			XMFLOAT3 area{ std::stof(tokens[idx + 8]), std::stof(tokens[idx + 9]), std::stof(tokens[idx + 10]) };
			XMFLOAT3 dir{ std::stof(tokens[idx + 11]), std::stof(tokens[idx + 12]), std::stof(tokens[idx + 13]) };
			float lifetime = std::stof(tokens[idx + 14]);

			particle_sync_data.particle_ID = id;
			particle_sync_data.particle_type = type;
			particle_sync_data.obj_pos = pos;
			particle_sync_data.obj_look = look;
			particle_sync_data.area_extent = area;
			particle_sync_data.main_direction = dir;
			particle_sync_data.LifeTime = lifetime;

			if (command == "PARTICLE_CREATE")
				stage_scene->Create_Particle_Object(particle_sync_data);
			else if (command == "PARTICLE_UPDATE")
				stage_scene->Update_Particle_Object(particle_sync_data);
		}
	}
	else if (command == "PARTICLE_REMOVE")
	{
		int count = std::stoi(tokens[1]);

		for (int i = 0; i < count; ++i)
		{
			int idx = 2 + i;
			if (idx >= tokens.size()) break;

			UINT id = std::stoi(tokens[idx]);
			stage_scene->Remove_Particle_Object(id);

		}
	}
}




void CGameFramework::HandleClientIdAssignment()
{
	std::cout << "[DEBUG] Received my client ID: " << Client_ID << std::endl;
	bClientIdAssigned = true;

	auto scene = scene_manager->Get_Active_Scene();

	if (!m_pPlayer)
	{
		m_pPlayer = std::make_shared<CTerrainPlayer>(m_pd3dDevice, Active_CommandList, scene->Get_MRT_GraphicsRootSignature(), scene->m_pTerrain.get(), 0);
		m_pPlayer->SetID(Client_ID);
		m_pPlayer->Set_Name("LocalPlayer_" + std::to_string(Client_ID));
		m_pPlayer->Set_Active(true);
		scene_manager->Add_Player(m_pPlayer);
	}
	else
		m_pPlayer->SetID(Client_ID);


	auto m_pCamera = scene_manager->Get_Active_Scene_Main_Camera();
	if (m_pCamera && m_pPlayer)
		m_pCamera->SetPlayer(m_pPlayer.get());
}

void CGameFramework::HandleChangeScene(const std::vector<std::string>& tokens)
{
	if (tokens.size() < 2)
	{
		//std::cerr << "[ERROR][HandleChangeScene] Scene type 정보 부족" << std::endl;
		return;
	}

	std::string change_scene_type = tokens[1];
	Change_Call_By_Server = std::stoi(change_scene_type);

	change_signal = { false,Scene_Type::etc, "" };

	switch (Change_Call_By_Server)
	{
	case -1:		// 변경 없음
		break;

	case 0:		//로비
		change_signal.change = true;
		change_signal.scene_name = "Character_Select";
		change_signal.type = Scene_Type::Lobby;
		break;
		
	case 1:		// 스태이지 선택
		change_signal.change = true;
		change_signal.scene_name = "Stage_Select";
		change_signal.type = Scene_Type::Board;
		break;

	case 2:		// 스테이지 1
		change_signal.change = true;
		change_signal.scene_name = "Stage_1";
		change_signal.type = Scene_Type::Stage_1;
		break;

	case 3:
		// 스테이지 2
		change_signal.change = true;
		change_signal.scene_name = "Stage_2";
		change_signal.type = Scene_Type::Stage_2;
		break;

	case 4:
	case 5:
	case 6:
	default:
		// 추가 예정
		break;
	}
	
	std::cout << "[CLIENT][HandleChangeScene] received: " << change_signal.scene_name << std::endl;
	std::cout << "[CLIENT][HandleChangeScene] c_signal.change: " << change_signal.change << ", c_signal.scene_name: " << change_signal.scene_name << ", c_signal.change: " << change_signal.change << std::endl;

}

void CGameFramework::HandlePlayerSync(int player_ID, int character_model_ID, const ServerSyncData& syncData)
{
	std::lock_guard<std::mutex> lock(remotePlayerUpdateMutex);

	if (player_ID == Client_ID)
	{
		// 애니메이션 및 상태 전환은 추가 예정
		if (syncData.bStateChange)
			m_pPlayer->SetPosition(syncData.position);
		return;
	}
	else
	{
		if (Connected_Player_List[player_ID]) // 이미 플레이어 데이터 존재
		{
			scene_manager->Sync_Player_Data(player_ID, syncData);
		}
		else // 플레이어 데이터 없음, 추가 필요
		{
			auto newPlayer = Create_Player(player_ID, character_model_ID);
			scene_manager->Add_Player(newPlayer);
			Connected_Player_List[player_ID] = true;
		}

	}
}

 shared_ptr<CPlayer> CGameFramework::Create_Player(int playerId, int characterId)
{
	 auto scene = scene_manager->Get_Active_Scene();


	auto new_Player = std::make_shared<CTerrainPlayer>(m_pd3dDevice, Active_CommandList, scene->Get_MRT_GraphicsRootSignature(), scene->m_pTerrain.get(), characterId);


	new_Player->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	new_Player->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	new_Player->SetState(0);
	new_Player->SetID(playerId);
	new_Player->Set_Name("Remote_" + std::to_string(playerId));
	new_Player->type = EObjectType::Player;
	new_Player->SetRotationSpeed(1.0f);
	new_Player->Set_Active(true);
	new_Player->SetScale(XMFLOAT3(10.0f, 10.0f, 10.0f));
	new_Player->Set_Child(new_Player->m_pRootModel);
	new_Player->SetupWeaponCollider();
	new_Player->ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	new_Player->CreateShaderVariables(m_pd3dDevice, Active_CommandList);

	std::cout << "[SUCCESS] RemotePlayer creation completed: " << playerId << std::endl;

	return new_Player; 
}

 void CGameFramework::Multi_PlayerLeave(int leaveId)
 {
	 if (Connected_Player_List[leaveId] == true)
	 {
		 Connected_Player_List[leaveId] = false;
		 scene_manager->Remove_Player(leaveId);
		 std::cout << "[DEBUG] PLAYER_LEAVE detected: " << leaveId << std::endl;

	 }
 }



void CGameFramework::PlayerLeave(int playerId)
{
	std::string packet = "PLAYER_LEAVE," + std::to_string(playerId) + "\n";
	int result = send(serverSocket, packet.c_str(), (int)packet.size(), 0);
	if (result == SOCKET_ERROR)
	{
		std::cout << "[ERROR] Failed to send PLAYER_LEAVE: " << WSAGetLastError() << std::endl;
	}
	else
	{
		std::cout << "[SEND] " << packet << std::endl;
	}
}

void CGameFramework::Disconnect()
{
	isRunning = false;

	PlayerLeave(Client_ID);

	if (networkThread.joinable())
		networkThread.join();
	closesocket(serverSocket);
	WSACleanup();

	std::cout << "[INFO] Disconnected from server" << std::endl;

}


void CGameFramework::NetworkLoop()
{
	char buf[2048];
	std::string pending;                

	while (isRunning)
	{
		int n = recv(serverSocket, buf, sizeof(buf), 0);
		if (n <= 0) { break; }

		pending.append(buf, n);

		size_t pos;
		while ((pos = pending.find('\n')) != std::string::npos)
		{
			std::string line = pending.substr(0, pos);   
			pending.erase(0, pos + 1);                   

			{
				std::lock_guard<std::mutex> lk(recvQueueMutex);
				recvQueue.push(std::move(line));         
			}
		}
	}

	//while (isRunning)
	//{
	//	char buffer[1024 + 1];
	//	int bytesReceived = recv(serverSocket, buffer, 1024, 0);
	//	//		std::cout << "[recv] Receive successful: " << bytesReceived << std::endl;

	//	if (bytesReceived > 0)
	//	{
	//		buffer[bytesReceived] = '\0';
	//		std::string receivedData(buffer);

	//		{
	//			std::lock_guard<std::mutex> lock(recvQueueMutex);
	//			recvQueue.push(receivedData);
	//			//			std::cout << "[recvQueue] Data push completed, current queue size: " << recvQueue.size() << std::endl;
	//		}
	//	}

	//	else if (bytesReceived == SOCKET_ERROR)
	//	{
	//		std::cerr << "[ERROR] recv() FAIL: " << WSAGetLastError() << std::endl;
	//	}
	//	else if (bytesReceived == 0)
	//	{
	//		std::cerr << "[INFO] Connection with server closed" << std::endl;
	//		isRunning = false;
	//		break;
	//	}

	//}
}

bool CGameFramework::IsServerConnected()
{
	return isRunning && serverSocket != INVALID_SOCKET;
}

int CGameFramework::GetServerPlayerID()
{
	return Client_ID;
}

void CGameFramework::StartPingThread()
{
	std::thread([this]()
		{
			while (isRunning)
			{
				if (serverSocket != INVALID_SOCKET)
				{
					SendPacket_String("PING\n");
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}).detach();
}