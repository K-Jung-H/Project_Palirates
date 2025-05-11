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

	object_manager = std::make_shared<Object_Manager>();
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

	CDescriptor_Heap::Init(m_pd3dDevice, 0, 100, 50);

	scene_manager = new Scene_Manager(N_SwapChainBuffers, m_pd3dDevice, p_CommandQueue, ptr_SwapChainBackBuffer_List, m_nWndClientWidth, m_nWndClientHeight);
	post_effect_manager = new Post_Effect_Manager(m_pd3dDevice);
	
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
					multiMode = true;
					break;

				case VK_TAB:
				{
					scene_index = (scene_index + 1) % 3;

					switch (scene_index)
					{
					case 0:
						scene_manager->Set_Active_Scene("Character_Select");
						break;
					case 1:
						scene_manager->Set_Active_Scene("Game_Board");
						break;
					case 2:
						scene_manager->Set_Active_Scene("In_Stage");
						break;
					default:
						break;
					}

					m_pPlayer = scene_manager->Get_Active_Scene_Player();
					m_pCamera = m_pPlayer->GetCamera();
					Object_Manager::Reserve_Update();
					break;
				}
				break;

				case 0x38:
				{
					// Set scene_index to 0 when key 8 is pressed
					scene_index = 0;
					scene_manager->Set_Active_Scene("Character_Select");
					m_pPlayer = scene_manager->Get_Active_Scene_Player();
					m_pCamera = m_pPlayer->GetCamera();
					Object_Manager::Reserve_Update();
				}
					break;
				case 0x39:
				{
					// Set scene_index to 1 when key 9 is pressed
					scene_index = 1;
					scene_manager->Set_Active_Scene("Game_Board");
					m_pPlayer = scene_manager->Get_Active_Scene_Player();
					m_pCamera = m_pPlayer->GetCamera();
					Object_Manager::Reserve_Update();
				}
					break;
				case 0x30:
				{
					// Set scene_index to 2 when key 0 is pressed
					scene_index = 2;
					scene_manager->Set_Active_Scene("In_Stage");
					m_pPlayer = scene_manager->Get_Active_Scene_Player();
					m_pCamera = m_pPlayer->GetCamera();
					Object_Manager::Reserve_Update();
				}
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
			if (nMessageID == WM_KEYDOWN && wParam == VK_OEM_2) {
				// Monster State Change EX
				//auto* mon = scene_manager->Get_Active_Scene()->obj_manager->Get_Object_List(Object_Type::skinned);

				//if (mon && !mon->empty())
				//{
				//	std::shared_ptr<CGameObject> baseObj = (*mon)[0]; 

				//	CGameObject* base = baseObj.get();

				//	auto* anubis = dynamic_cast<CAnubisObject*>(base);
				//	if (anubis)
				//	{
				//		//anubis->GetStateMachine()->changeState(State::Get_Hit, Key_Value::None);
				//		anubis->GetStateMachine()->changeState(State::Attack1, Key_Value::None);
				//		//anubis->GetStateMachine()->changeState(State::Attack2, Key_Value::None);
				//		//anubis->GetStateMachine()->changeState(State::Attack3, Key_Value::None);
				//	}

				//	std::shared_ptr<CGameObject> baseObj2 = (*mon)[1];

				//	CGameObject* base2 = baseObj2.get();

				//	auto* dra = dynamic_cast<CDragonObject*>(base2);
				//	if (dra)
				//	{
				//		//anubis->GetStateMachine()->changeState(State::Get_Hit, Key_Value::None);
				//		dra->GetStateMachine()->changeState(State::Attack2, Key_Value::None);
				//		//anubis->GetStateMachine()->changeState(State::Attack2, Key_Value::None);
				//		//anubis->GetStateMachine()->changeState(State::Attack3, Key_Value::None);
				//		dra->MoveUp(30.0f);
				//	}
				//}
			}
			if (nMessageID == WM_KEYDOWN && wParam == 'U') {
				//ServerAnimationSyncData data;
				////data.position = XMFLOAT3(0.0f, 0.0f, 0.0f);
				//data.position = m_pPlayer->GetPosition();
				//data.lookVector = XMFLOAT3(0.0f, 0.0f, 1.0f);
				//data.currentState = State::Get_Hit_F2;
				//for (int i = 0; i < m_pPlayer->n_Animation; i++) {
				//	data.trackPositions.push_back(m_pPlayer->GetSkinnedAnimationController()->m_pAnimationTracks[i].m_fPosition);
				//	data.Weights.push_back(m_pPlayer->GetSkinnedAnimationController()->m_pAnimationTracks[i].m_fWeight);
				//}

				//GetSyncManager().AddPlayerSyncData(ClientNum, data);
				//m_pPlayer->ApplySyncData(GetSyncManager().GetPlayerSyncData(ClientNum));
				m_pPlayer->GetStateMachine()->changeState(State::Get_Hit_F2, Key_Value::None);
			}
			if (nMessageID == WM_KEYDOWN && wParam == 'T') {
				//auto* obj_list = scene_manager->Get_Active_Scene()->obj_manager->Get_Object_List(Object_Type::skinned);
				//if (obj_list && obj_list->size() < 7) {
				static int modelnum = 1;
				if (modelnum > 5) modelnum = 0;
					if (!Active_CommandList) {
						OutputDebugString(L"Active_CommandList is nullptr!\n");
						return 0;
					}

					HRESULT hr = Active_CommandList->Reset(Active_CommandAllocator, nullptr);
					if (FAILED(hr)) {
						OutputDebugString(L"CommandList Reset Failed!\n");
						return 0;
					}

					//CLoadedModelInfo* pGargoyleModel6 = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, Active_CommandList, scene_manager->Get_Active_Scene()->GetGraphicsRootSignature(), "Model/First_Mate_v12.bin", NULL);
					string player_name = "test_palyer" + std::to_string(nPlayer);


					std::string_view name_view = player_name;

					std::shared_ptr<CTerrainPlayer> humanObject_7 = std::make_shared<CTerrainPlayer>(m_pd3dDevice, Active_CommandList, scene_manager->Get_Active_Scene()->Get_MRT_GraphicsRootSignature(), scene_manager->Get_Active_Scene()->m_pTerrain.get(), modelnum);
					humanObject_7->SetPosition(XMFLOAT3(30.0f, scene_manager->Get_Active_Scene()->m_pTerrain->Get_Mesh_Height(30.0f, 10.0f+ nPlayer*30.0f), 10.0f + nPlayer * 30.0f));
					humanObject_7->Set_Name(player_name);
					humanObject_7->Set_Child(humanObject_7->m_pRootModel);
					humanObject_7->SetupWeaponCollider();
					humanObject_7->Object_type = OBJECT_TPYE_PLAYER;
					CPlayer* player = humanObject_7.get(); 
					auto st = std::make_unique<MultiPlayerStateMachine>(player);
					humanObject_7->SetStateMachine(std::move(st));
					humanObject_7->GetStateMachine()->changeState(State::Select_Idle, Key_Value::None);
					scene_manager->Get_Active_Scene()->obj_manager->Add_Object(humanObject_7, Object_Type::player);
					nPlayer++;
					modelnum++;

					string message2 = "Player list :";
					auto* player_list = scene_manager->Get_Active_Scene()->obj_manager->Get_Object_List(Object_Type::player);
					for (auto& obj : *player_list) {
						string name = obj->m_pstrFrameName;
						message2 += name + ", ";
					}
					message2 += "\n";
					DebugOutput(message2);

					Active_CommandList->Close();
					ID3D12CommandList* ppCommandLists[] = { Active_CommandList };
					p_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
				//}
			}
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

	Post_ComputeShader::CreateComputeRootSignature(m_pd3dDevice);
	
	for (UINT i = 0; i < N_SwapChainBuffers; ++i)
	{
		Post_ComputeShader::CreateBackBufferSRV(m_pd3dDevice, ptr_SwapChainBackBuffer_List[i], i, DXGI_FORMAT_R8G8B8A8_UNORM);
	}



	//========================================================
	std::shared_ptr<CScene> in_stage_scene = std::make_shared<CScene>();
	scene_manager->Register_Scene("In_Stage", in_stage_scene);
	scene_manager->Build_Scene("In_Stage", m_pd3dDevice, Active_CommandList);
	std::shared_ptr<CTerrainPlayer> pPlayer = std::make_shared<CTerrainPlayer>(m_pd3dDevice, Active_CommandList, in_stage_scene->Get_MRT_GraphicsRootSignature(), in_stage_scene->m_pTerrain.get(), Captain);
	pPlayer->Set_Child(pPlayer->m_pRootModel);
	pPlayer->SetupWeaponCollider();
	in_stage_scene->obj_manager->Add_Object(pPlayer, Object_Type::skinned);
	scene_manager->Set_Scene_Player("In_Stage", pPlayer);


	std::shared_ptr<Board_Scene> game_board_scene = std::make_shared<Board_Scene>();
	scene_manager->Register_Scene("Game_Board", game_board_scene);
	scene_manager->Build_Scene("Game_Board", m_pd3dDevice, Active_CommandList);
	std::shared_ptr<Observer> game_board_observer = std::make_shared<Observer>(m_pd3dDevice, Active_CommandList, game_board_scene->Get_MRT_GraphicsRootSignature());
	scene_manager->Set_Scene_Player("Game_Board", game_board_observer);


	std::shared_ptr<Character_Select_Scene> character_select_scene = std::make_shared<Character_Select_Scene>();
	scene_manager->Register_Scene("Character_Select", character_select_scene);
	scene_manager->Build_Scene("Character_Select", m_pd3dDevice, Active_CommandList);
	std::shared_ptr<Observer> select_scene_observer = std::make_shared<Observer>(m_pd3dDevice, Active_CommandList, character_select_scene->Get_MRT_GraphicsRootSignature());
	select_scene_observer->SetPosition(XMFLOAT3{ 37.0f, 0.0f, 28.0f });
	scene_manager->Set_Scene_Player("Character_Select", select_scene_observer);
	

	scene_manager->Set_Active_Scene("Character_Select");
	m_pPlayer = scene_manager->Get_Active_Scene_Player();

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

	if (!bProcessedByScene && m_pPlayer->bIsControllable)
	{
		DWORD dwDirection = 0;

		if ((pKeysBuffer[VK_UP] & 0xF0) || (pKeysBuffer[0x57] & 0xF0))
			dwDirection |= DIR_FORWARD;   
		if ((pKeysBuffer[VK_DOWN] & 0xF0) || (pKeysBuffer[0x53] & 0xF0))
			dwDirection |= DIR_BACKWARD;  
		if ((pKeysBuffer[VK_LEFT] & 0xF0) || (pKeysBuffer[0x41] & 0xF0))
			dwDirection |= DIR_LEFT;    
		if ((pKeysBuffer[VK_RIGHT] & 0xF0) || (pKeysBuffer[0x44] & 0xF0))
			dwDirection |= DIR_RIGHT;   
		if ((pKeysBuffer[VK_PRIOR] & 0xF0) || (pKeysBuffer[0x51] & 0xF0))
			dwDirection |= DIR_UP;       
		if ((pKeysBuffer[VK_NEXT] & 0xF0) || (pKeysBuffer[0x45] & 0xF0))
			dwDirection |= DIR_DOWN;      

		bool isMouseButtonDown = (pKeysBuffer[VK_LBUTTON] & 0xF0) || (pKeysBuffer[VK_RBUTTON] & 0xF0);

		if (m_pPlayer && m_pPlayer->GetCamera())
		{
			m_pPlayer->GetCamera()->SetMouseButtonHeld(isMouseButtonDown);
		}

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
				m_pPlayer->Move(dwDirection, 1000.0f * m_GameTimer.GetTimeElapsed(), true);
		}
		
	}
	
}

void CGameFramework::Animate_Scene()
{
	HRESULT hResult;
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	scene_manager->Animate_Active_Objects(m_pd3dDevice, Active_CommandList, fTimeElapsed);

	// Server logic EX
	//ServerAnimationSyncData data = m_pPlayer->MakeSyncData();
	//data.position.x += 10.0f;

	//GetSyncManager().AddPlayerSyncData(ClientNum, data);
	////m_pPlayer->ApplySyncData(GetSyncManager().GetPlayerSyncData(ClientNum));

	//auto* obj_list = scene_manager->Get_Active_Scene()->obj_manager->Get_Player_List();
	//auto player = std::dynamic_pointer_cast<CPlayer>((*obj_list)[ClientNum]);

	//player->ApplySyncData(GetSyncManager().GetPlayerSyncData(ClientNum));

	//===============================================================

	if (multiMode)
	{
		if (m_pPlayer)
		{
			m_pPlayer->Animate(m_GameTimer.GetTimeElapsed());
			m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
		}
		for (auto& [playerId, remotePlayer] : m_pRemotePlayers)
		{
			if (remotePlayer)
			{
				remotePlayer->Animate(m_GameTimer.GetTimeElapsed());
				remotePlayer->Update(m_GameTimer.GetTimeElapsed());
			}
		}
	}
	else
	{
		m_pPlayer->Animate(fTimeElapsed);
		m_pPlayer->Update(m_GameTimer.GetTimeElapsed());

		static bool dead = false;
		// Weapon Drop EX
		if (!dead) {
			if (m_pPlayer->GetStateMachine()->Get_State() == State::Knock_Down) {
				auto sword = m_pPlayer->DropWeapon("SM_Wep_Cutlass_01");
				scene_manager->Get_Active_Scene()->obj_manager->Add_Object(sword, Object_Type::non_skinned);
				dead = true;
			}
		}
		else {
			if (m_pPlayer->GetStateMachine()->Get_State() != State::Knock_Down) {
				m_pPlayer->RestoreWeapon("SM_Wep_Cutlass_01");
				dead = false;
			}
		}
	}
}

void CGameFramework::Update_Scene()
{
	scene_manager->Update_Active_Objects(m_pd3dDevice, Active_CommandList);
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

	BeginGPUStage(GPU_Stage::Compute);
	PrepareStage(GPU_Stage::Compute);
	{
		std::lock_guard<std::mutex> lock(recvQueueMutex);
		while (!recvQueue.empty())
		{
			std::string receivedData = recvQueue.front();
			recvQueue.pop();

			std::cout << "[FrameAdvance] 수신 패킷 처리: " << receivedData << std::endl;
			ProcessReceivedData(receivedData);
		}
	}

	{
		std::lock_guard<std::mutex> lock(pendingCreateMutex);
		while (!pendingPlayerCreates.empty())
		{
			int playerId = pendingPlayerCreates.front();
			pendingPlayerCreates.pop();
			CreateRemotePlayer(playerId);
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

	// ====================== [4] Render Phase ======================
	BeginGPUStage(GPU_Stage::Render);
	PrepareStage(GPU_Stage::Render);
	{
		auto dsvHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		Active_CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		scene_manager->Prepare_MRT_G_Buffer(Active_CommandList, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], &DsvDescriptorCPUHandle);

		scene_manager->Prepare_Render_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);
		UpdateShaderVariables();
		scene_manager->Render_MRT_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);

		if (m_pPlayer)
			m_pPlayer->Render(Active_CommandList, m_pCamera);

		{
			std::lock_guard<std::mutex> lock(remotePlayerUpdateMutex);

			for (auto& [id, remotePlayer] : m_pRemotePlayers)
			{
				if (remotePlayer)
					remotePlayer->Render(Active_CommandList, m_pCamera);
			}
		}

		SynchronizeResourceTransition(Active_CommandList, ptr_SwapChainBackBuffer_List[SwapChainBuffer_Index], 
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		
		// Merge G-Buffers
		Active_CommandList->OMSetRenderTargets(1, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], TRUE, nullptr);

		scene_manager->Prepare_Deffered_Render_Scene(Active_CommandList);
		scene_manager->Deffered_Render_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);
	}
	EndGPUStage(GPU_Stage::Render);

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
		D3D12_GPU_DESCRIPTOR_HANDLE  Velocity_G_Buffer_SRV_handle = MRT_shader->GetTexture()[0].GetGraphicsSrvGpuDescriptorHandle(3);
		post_effect_manager->Add_Effect(Effect_Type::Outline, 1, &Velocity_G_Buffer_SRV_handle);
		post_effect_manager->Add_Effect(Effect_Type::Motion_Blur, 1, &Velocity_G_Buffer_SRV_handle);
		
		// Apply reserved effects
		post_effect_manager->Apply_Effect(Active_CommandList, SwapChainBuffer_Index);
		post_effect_manager->Clear_Reserved_Effect();


		// ====================== [5] Post Process Phase - Overlay Alpha Effects ======================

		// Use previously stored depth buffer values
		auto dsvHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		Active_CommandList->OMSetRenderTargets(1, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], TRUE, &dsvHandle);


		// Rendering Transparent object
		scene_manager->Prepare_Render_Transparent_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);
		UpdateShaderVariables();
		scene_manager->Render_Transparent_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);


		// Record moving Object's Last Pos to use motion blur
		scene_manager->Post_Update_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);
		if (m_pPlayer)
		{
			m_pPlayer->Record_Last_Pos();
		}

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

	Particle_Manager* active_scene_particle_manager = scene_manager->Get_Active_Scene()->particle_manager;
	if (active_scene_particle_manager)
		active_scene_particle_manager->Process_Destroy_Queue();

	SendPacket();

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
	if (serverSocket == INVALID_SOCKET) return;
	if (!m_pPlayer) return;

	XMFLOAT3 pos = m_pPlayer->GetPosition();
	XMFLOAT3 look = m_pPlayer->GetLookVector();
	int state = static_cast<int>(m_pPlayer->GetState());

	char buffer[256];
	sprintf_s(buffer, "MOVE,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d", ClientNum, pos.x, pos.y, pos.z, look.x, look.y, look.z, state);

	send(serverSocket, buffer, (int)strlen(buffer), 0);
}

//void CGameFramework::ProcessReceivedData(const std::string& receivedData)
//{
//
//	std::cout << "[디버그] ProcessReceivedData() 호출됨" << std::endl;
//
//	int playerId;
//	float px, py, pz;
//	int state;
//
//	if (sscanf_s(receivedData.c_str(), "CLIENT_ID,%d", &ClientNum) == 1)
//	{
//		printf("[Debug] 내 클라이언트 ID 수신 완료: %d\n", ClientNum);
//
//		CreateLocalPlayer(ClientNum);
//
//		m_pPlayer = GetSceneManager().GetPlayerById(ClientNum);
//		if (m_pCamera && m_pPlayer)
//			m_pCamera->SetPlayer(m_pPlayer);
//
//		return;
//	}
//
//	if (sscanf_s(receivedData.c_str(), "PLAYER_UPDATE,%d,%f,%f,%f,%d", &playerId, &px, &py, &pz, &state) == 5)
//	{
//
//		std::cout << "[디버그] PLAYER_UPDATE 패킷 감지됨" << std::endl;
//		if (playerId == ClientNum)
//		{
//			if (m_pPlayer)
//			{
//				m_pPlayer->SetPosition(XMFLOAT3(px, py, pz));
//				m_pPlayer->SetState(state);
//			}
//		}
//		else
//		{
//			auto it = m_pRemotePlayers.find(playerId);
//			if (it == m_pRemotePlayers.end())
//			{
//				std::cout << "[디버그] playerId " << playerId << " 은 새 플레이어로 큐에 등록됨" << std::endl;
//				{
//					std::lock_guard<std::mutex> lock(pendingCreateMutex);
//					pendingPlayerCreates.push(playerId);
//				}
//				std::cout << "[디버그] playerId  "<< playerId  << std::endl;
//
//				return;
//			}
//			else
//			{
//				std::cout << "[디버그] playerId " << playerId << " 은 이미 m_pRemotePlayers에 있음" << std::endl;
//			}
//
//			if (it != m_pRemotePlayers.end())
//			{
//				auto remotePlayer = it->second;
//				if (remotePlayer)
//				{
//					auto syncData = GetSyncManager().GetPlayerSyncData(playerId);
//					remotePlayer->ApplySyncData(syncData);
//
//					remotePlayer->SetPosition(XMFLOAT3(px, py, pz));
//					remotePlayer->SetState(state);
//				}
//				else
//				{
//					std::cout << "[경고] remotePlayer null 상태" << std::endl;
//				}
//			}
//		}
//	}
//}

void CGameFramework::ProcessReceivedData(const std::string& receivedData)
{
	std::cout << "[디버그] ProcessReceivedData() 호출됨" << std::endl;

	int playerId;
	float px, py, pz;
	float lookX, lookY, lookZ;
	int state;

	if (sscanf_s(receivedData.c_str(), "CLIENT_ID,%d", &ClientNum) == 1)
	{
		std::cout << "[Debug] 내 클라이언트 ID 수신 완료: " << ClientNum << std::endl;
		bClientIdAssigned = true;

		CreateLocalPlayer(ClientNum);

		m_pPlayer = std::shared_ptr<CPlayer>(GetSceneManager().GetPlayerById(ClientNum));
		if (m_pCamera && m_pPlayer)
			m_pCamera->SetPlayer(m_pPlayer.get());

		// 지연된 remote 생성 처리
		while (!pendingPlayerCreates.empty()) {
			int pendingId = pendingPlayerCreates.front();
			pendingPlayerCreates.pop();
			if (pendingId != ClientNum)
				CreateRemotePlayer(pendingId);

			{
				std::lock_guard<std::mutex> lock(pendingUpdateMutex);
				if (pendingUpdateMap.contains(pendingId)) {
					ProcessReceivedData(pendingUpdateMap[pendingId]);
					pendingUpdateMap.erase(pendingId);
				}
			}
		}

		return;
	}

	if (!bClientIdAssigned)
	{
		std::cout << "[경고] 아직 CLIENT_ID를 받지 않아 패킷 처리 지연 중: " << receivedData << std::endl;

		int playerId;
		if (sscanf_s(receivedData.c_str(), "PLAYER_UPDATE,%d", &playerId) == 1)
		{
			std::lock_guard<std::mutex> lock(pendingUpdateMutex);
			pendingUpdateMap[playerId] = receivedData;  // 최신 패킷 하나만 저장
		}

		return;
	}

	if (sscanf_s(receivedData.c_str(), "PLAYER_UPDATE,%d,%f,%f,%f,%f,%f,%f,%d", &playerId, &px, &py, &pz, &lookX, &lookY, &lookZ, &state) == 8)
	{
		std::cout << "[디버그] PLAYER_UPDATE 패킷 감지됨" << std::endl;

		State convertedState = static_cast<State>(state);



		if (playerId == ClientNum)
		{
			if (px == 0.0f && py == 0.0f && pz == 0.0f)
			{
				std::cout << "[디버그] 좌표가 (0,0,0)이므로 무시됨" << std::endl;
				return;
			}

			if (m_pPlayer)
			{
				m_pPlayer->SetPosition(XMFLOAT3(px, py, pz));
				m_pPlayer->SetLookDirection(XMFLOAT3(lookX, lookY, lookZ));
				m_pPlayer->SetState(state);

				State convertedState = static_cast<State>(state);
				if (m_pPlayer->GetStateMachine())
				{
					m_pPlayer->GetStateMachine()->changeState(convertedState, Key_Value::None);
				}
			}
		}
		else
		{
			auto it = m_pRemotePlayers.find(playerId);
			if (it == m_pRemotePlayers.end())
			{
				// 중복 큐 방지
				bool alreadyQueued = false;
				std::queue<int> tempQueue = pendingPlayerCreates;
				while (!tempQueue.empty())
				{
					if (tempQueue.front() == playerId)
					{
						alreadyQueued = true;
						break;
					}
					tempQueue.pop();
				}

				if (!alreadyQueued)
				{
					std::lock_guard<std::mutex> lock(pendingCreateMutex);
					pendingPlayerCreates.push(playerId);
					std::cout << "[디버그] playerId " << playerId << " remote 큐에 추가됨" << std::endl;
				}
				else
				{
					std::cout << "[중복 방지] playerId " << playerId << " 이미 큐에 있음" << std::endl;
				}
				return;
			}

			auto remotePlayer = it->second;
			if (remotePlayer)
			{

				std::lock_guard<std::mutex> lock(remotePlayerUpdateMutex);

				auto& syncDataMap = GetSyncManager().GetAllSyncData();
				if (syncDataMap.find(playerId) != syncDataMap.end())
				{
					auto syncData = syncDataMap[playerId];

					remotePlayer->ApplySyncData(syncData);
				}

				remotePlayer->SetPosition(XMFLOAT3(px, py, pz));
				remotePlayer->SetLookDirection(XMFLOAT3(lookX, lookY, lookZ));


				State convertedState = static_cast<State>(state);
				if (remotePlayer->GetStateMachine())
				{
					remotePlayer->GetStateMachine()->changeState(convertedState, Key_Value::None);
				}
			}
			else
			{
				std::cout << "[경고] remotePlayer null 상태" << std::endl;
			}
		}
	}
	//else if (receivedData.rfind("MONSTER_UPDATE,", 0) == 0)
	//{
	//	int id, state, hp, type;
	//	float x, y, z;
	//	float lookX, lookY, lookZ;
	//
	//	CScene* scene = scene_manager->Get_Active_Scene_Ptr();
	//	if (!scene) return;


		//if (sscanf_s(receivedData.c_str(), "MONSTER_UPDATE,%d,%f,%f,%f,%f,%f,%f,%d,%d,%d",
		//	&id, &x, &y, &z, &lookX, &lookY, &lookZ, &hp, &state, &type) == 10)
		//{
		//	std::lock_guard<std::mutex> lock(monsterDataMutex);
		//
		//	auto& monsterMap = remoteMonsters;
		//
		//	if (monsterMap.find(id) == monsterMap.end())
		//	{
		//		CLoadedModelInfo* pFishmanModel = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, Active_CommandList, scene->Get_MRT_GraphicsRootSignature(), "Model/FishmanLP.bin", NULL);
		//		//CLoadedModelInfo* pFishmanModel_t = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, "Model/FishmanLP.bin", NULL);
		//
		//		std::shared_ptr<CMonsterObject> pMonster = std::make_shared<CMonsterObject>(m_pd3dDevice, Active_CommandList, scene->Get_MRT_GraphicsRootSignature(), pFishmanModel, 5);
		//		//std::shared_ptr<CMonsterObject> m =		   std::make_shared<CMonsterObject>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, pFishmanModel_t, 5);
		//
		//
		//		pMonster->SetPosition(XMFLOAT3(x, y, z));
		//		pMonster->SetLookDirection(XMFLOAT3(lookX, lookY, lookZ));
		//		pMonster->SetScale(10.0f, 10.0f, 10.0f);
		//		std::string name = "monster_" + std::to_string(id);
		//		pMonster->Set_Name(name.c_str());
		//		pMonster->test_num = id + 1000;
		//		pMonster->Set_Active(true);
		//
		//		pMonster->GetStateMachine()->changeState(static_cast<State>(state), Key_Value::None);
		//
		//		monsterMap[id] = pMonster;
		//
		//		CScene* scene = scene_manager->Get_Active_Scene_Ptr();
		//		if (!scene) return;
		//
		//		Object_Manager* renderManager = scene->obj_manager;
		//
		//		renderManager->Add_Object(pMonster, Object_Type::skinned);
		//	}
		//	else
		//	{
		//		// 기존 객체 위치 및 방향 업데이트
		//		auto& monster = monsterMap[id];
		//		monster->SetPosition(XMFLOAT3(x, y, z));
		//		monster->SetLookDirection(XMFLOAT3(lookX, lookY, lookZ));
		//		monster->GetStateMachine()->changeState(static_cast<State>(state), Key_Value::None);
		//	}
		//}
	//}
	else if (receivedData.rfind("PLAYER_LEAVE,", 0) == 0)
	{
		int leaveId;
		if (sscanf_s(receivedData.c_str(), "PLAYER_LEAVE,%d", &leaveId) == 1)
		{
			std::cout << "[디버그] PLAYER_LEAVE 감지됨: " << leaveId << std::endl;
			{
				auto it = m_pRemotePlayers.find(leaveId);
				if (it != m_pRemotePlayers.end())
				{
					CScene* scene = scene_manager->Get_Active_Scene_Ptr();
					if (scene && scene->obj_manager)
					{
						//scene->obj_manager->Remove_Object(it->second);  // Remove_Object() 함수 필요
					}
	
					m_pRemotePlayers.erase(it);
					std::cout << "[디버그] remote player 제거됨: " << leaveId << std::endl;
				}
			}
		}
	}

}

void CGameFramework::CreateLocalPlayer(int playerId)
{
	std::cout << "[디버그] CreateRemotePlayer 호출됨: " << playerId << std::endl;
	CScene* scene = scene_manager->Get_Active_Scene_Ptr();
	if (!scene || !scene->obj_manager) return;


	auto local_player = std::make_shared<CTerrainPlayer>(m_pd3dDevice, Active_CommandList, scene_manager->Get_Active_Scene()->Get_MRT_GraphicsRootSignature(), scene_manager->Get_Active_Scene()->m_pTerrain.get());

	local_player->SetPosition(XMFLOAT3(25.0f, 0.0f, 25.0f));
	//local_player->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	local_player->SetID(playerId);
	local_player->SetState(0);
	local_player->Set_Name("MyPlayer");
	local_player->Object_type = OBJECT_TPYE_PLAYER;

	scene->obj_manager->Add_Object(local_player, Object_Type::player);

	std::cout << "[디버그] Add_Object 완료" << std::endl;

	scene_manager->RegisterRemotePlayer(playerId, local_player);

	m_pPlayer = local_player;// .get();

	if (m_pCamera)
	{
		m_pCamera->SetPlayer(m_pPlayer.get());
	}

	auto playerList = scene->obj_manager->Get_Object_List(Object_Type::player);
	std::cout << "[디버그] player_list 크기: " << playerList->size() << std::endl;

	if (playerList)
	{
		printf("[디버그] 현재 player_list 등록 상태:\n");
		for (auto& obj : *playerList)
		{
			if (obj)
			{
				auto rawObj = obj.get();
				std::cout << " - 이름: " << rawObj->m_pstrFrameName << ", Active: " << rawObj->Get_Active() << std::endl;
			}
		}
	}
}

void CGameFramework::CreateRemotePlayer(int playerId)
{
	std::cout << "[디버그] CreateRemotePlayer() 호출됨 - ID: " << playerId << std::endl;

	// 중복 방지
	if (m_pRemotePlayers.find(playerId) != m_pRemotePlayers.end())
	{
		std::cout << "[중복 방지] m_pRemotePlayers 에 이미 존재: " << playerId << std::endl;
		return;
	}

	auto scene = scene_manager->Get_Active_Scene();
	if (!scene || !scene->obj_manager)
	{
		std::cout << "[오류] scene 또는 obj_manager 가 NULL" << std::endl;
		return;
	}

	// scene 내부 중복 플레이어 검사
	auto* playerList = scene->obj_manager->Get_Object_List(Object_Type::player);
	for (const auto& obj : *playerList)
	{
		if (obj && obj->GetID() == playerId)
		{
			std::cout << "[중복 방지] scene 내부에도 이미 playerId 있음: " << playerId << std::endl;
			return;
		}
	}

	if (!m_pPlayer)
	{
		std::cout << "[CreateRemotePlayer] 내 플레이어가 아직 생성되지 않음" << std::endl;
		return;
	}

	auto remotePlayer = std::make_shared<CTerrainPlayer>(m_pd3dDevice, Active_CommandList, scene->Get_MRT_GraphicsRootSignature(), scene->m_pTerrain.get());

	remotePlayer->m_pMesh = m_pPlayer->m_pMesh;
	remotePlayer->Material_list = m_pPlayer->Material_list;


	remotePlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	//remotePlayer->SetLookDirection(XMFLOAT3(0.0f, 1.0f, 0.0f));
	remotePlayer->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	remotePlayer->SetState(0);
	remotePlayer->SetID(playerId);
	remotePlayer->Set_Name("Remote_" + std::to_string(playerId));
	remotePlayer->Object_type = OBJECT_TPYE_PLAYER;
	remotePlayer->SetRotationSpeed(1.0f);
	remotePlayer->Set_Active(true);
	remotePlayer->ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	remotePlayer->CreateShaderVariables(m_pd3dDevice, Active_CommandList);

	scene->obj_manager->Add_Object(remotePlayer, Object_Type::player);
	scene_manager->RegisterRemotePlayer(playerId, remotePlayer);
	m_pRemotePlayers[playerId] = remotePlayer;

	std::cout << "[성공] RemotePlayer 생성 완료: " << playerId << std::endl;
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
	//std::cout << "[쓰레드 확인] NetworkLoop() 시작됨" << std::endl;

	while (isRunning)
	{
		char buffer[1024 + 1]; // null terminator 공간 확보
		int bytesReceived = recv(serverSocket, buffer, 1024, 0);
		std::cout << "[recv] 수신 성공: " << bytesReceived << std::endl;

		if (bytesReceived > 0)
		{
			buffer[bytesReceived] = '\0';
			std::string receivedData(buffer);

			{
				std::lock_guard<std::mutex> lock(recvQueueMutex);
				recvQueue.push(receivedData);
				std::cout << "[recvQueue] 데이터 push 완료, 현재 큐 크기: " << recvQueue.size() << std::endl;
			}
		}

		else if (bytesReceived == SOCKET_ERROR)
		{
			std::cerr << "[ERROR] recv() 실패: " << WSAGetLastError() << std::endl;
		}
		else if (bytesReceived == 0)
		{
			std::cerr << "[INFO] 서버와의 연결 종료" << std::endl;
			isRunning = false;
			break;
		}
		for (const auto& playerPair : m_pRemotePlayers)
		{
			std::shared_ptr<CPlayer> player = playerPair.second;
			XMFLOAT3 pos = player->GetPosition();
			XMFLOAT3 lookVec = player->GetLookVector();
			std::cout << "[DEBUG] Player ID: " << playerPair.first
				<< ", Position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << ", LookVec : (" << lookVec.x << ", " << lookVec.y << ", " << lookVec.z << ")"
				<< std::endl;
		}
	}
}
