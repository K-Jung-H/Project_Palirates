//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"

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
}

CGameFramework::~CGameFramework()
{
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

	if (CScene::m_pDescriptorHeap == NULL)
	{
		CScene::m_pDescriptorHeap = new CDescriptorHeap();
		CScene::CreateCbvSrvDescriptorHeaps(m_pd3dDevice, 0, 70);
	}

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
	for (UINT i = 0; i < N_SwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	if (pd3dAdapter) pd3dAdapter->Release();
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
	WaitForGpuComplete();

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
					scene_manager->Load_Scene("Scene_2");
					Object_Manager::Reserve_Update();
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

	delete PostProcessing_shader;

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
	FrameworkInfo = ::CreateBufferResource(m_pd3dDevice, Active_CommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
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
	Active_CommandAllocator = Render_CommandAllocator;
	Active_CommandList = Render_CommandList;

	Active_CommandAllocator->Reset();
	Active_CommandList->Reset(Active_CommandAllocator, NULL);

	CreateShaderVariables();


	PostProcessing_shader = new CTextureToFullScreenShader();
	PostProcessing_shader->CreateShader(m_pd3dDevice, NULL, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = ptr_Rtv_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * N_SwapChainBuffers);

	PostProcessing_shader->CreateResourcesAndRtvsSrvs(m_pd3dDevice, Active_CommandList, RTV_Format_Num, RenderTarget_Config::RTV_FORMATS, d3dRtvCPUDescriptorHandle); 


	D3D12_GPU_DESCRIPTOR_HANDLE d3dDsvGPUDescriptorHandle = CScene::CreateShaderResourceView(m_pd3dDevice, m_pd3dDepthStencilBuffer, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);




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
	scene_manager->Set_Scene_Player("Scene_2", m_pPlayer);

	m_pCamera = m_pPlayer->GetCamera();
	//========================================================

	Active_CommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { Active_CommandList };
	p_CommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	WaitForGpuComplete();

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

	scene_manager->Update_Active_Particles(m_pd3dDevice, Active_CommandList, fTimeElapsed);

	//===============================================================

	m_pPlayer->Animate(fTimeElapsed);

}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[SwapChainBuffer_Index];
	HRESULT hResult = p_CommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	SwapChainBuffer_Index = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[SwapChainBuffer_Index];
	HRESULT hResult = p_CommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

//#define _WITH_PLAYER_TOP

void CGameFramework::Clear_RenderTarget(XMFLOAT3 background_color)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = ptr_Rtv_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (SwapChainBuffer_Index * ::gnRtvDescriptorIncrementSize);

	float pfClearColor[4] = { background_color.x, background_color.y, background_color.z, 1.0f };
	Active_CommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	Active_CommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	Active_CommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);
}

void CGameFramework::FrameAdvance()
{    
	HRESULT hResult;

	m_GameTimer.Tick(100.0f);
	
	ProcessInput();

	//==================================================
	Active_CommandAllocator = Compute_CommandAllocator;
	Active_CommandList = Compute_CommandList;

	hResult = Active_CommandAllocator->Reset();
	hResult = Active_CommandList->Reset(Active_CommandAllocator, NULL);

	Update_Scene();

	hResult = Active_CommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { Active_CommandList };
	p_CommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	//==================================================

#ifdef WRITE_TEXT_UI
	scene_manager->Update_UI();
#endif

	Active_CommandAllocator = Render_CommandAllocator;
	Active_CommandList = Render_CommandList;
	WaitForGpuComplete();
	hResult = Active_CommandAllocator->Reset();
	hResult = Active_CommandList->Reset(Active_CommandAllocator, NULL);


	//D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = ptr_Rtv_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//d3dRtvCPUDescriptorHandle.ptr += (SwapChainBuffer_Index * ::gnRtvDescriptorIncrementSize);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	Active_CommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	//Active_CommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	::SynchronizeResourceTransition(
		Active_CommandList,
		ptr_SwapChainBackBuffer_List[SwapChainBuffer_Index],
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	// Connect Multi_RenderTarget
	// nRenderTarget = 0 -> Not use BackBuffer in this time, 
	PostProcessing_shader->OnPrepareRenderTarget(Active_CommandList, 0, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], &DsvDescriptorCPUHandle);


	scene_manager->Pre_Render_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);

	//	Frame_Info_Buffer Update
	UpdateShaderVariables();

	scene_manager->Render_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);

	if (m_pPlayer) 
		m_pPlayer->Render(Active_CommandList, m_pCamera);

	//	Change Used RenderTarget Resource State
	PostProcessing_shader->OnPostRenderTarget(Active_CommandList);

	// Connect One RenderTarget
	Active_CommandList->OMSetRenderTargets(1, &SwapChainBack_Buffer_RTV_CPUHandle_list[SwapChainBuffer_Index], TRUE, NULL);



	// Testing Code
	PostProcessing_shader->Setting_Render(Active_CommandList, 0);
	scene_manager->Prepare_Deffered_Render_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);


	// Draw Scene by Mixing resource
	PostProcessing_shader->Render(Active_CommandList, NULL);




#ifndef WRITE_TEXT_UI
	::SynchronizeResourceTransition(
		Active_CommandList,
		ptr_SwapChainBackBuffer_List[SwapChainBuffer_Index],
		D3D12_RESOURCE_STATE_RENDER_TARGET, 
		D3D12_RESOURCE_STATE_PRESENT);

#endif

	hResult = Active_CommandList->Close();
	ID3D12CommandList *Render_CommandLists[] = { Active_CommandList };
	p_CommandQueue->ExecuteCommandLists(1, Render_CommandLists);
	WaitForGpuComplete();



	hResult = Active_CommandAllocator->Reset();
	hResult = Active_CommandList->Reset(Active_CommandAllocator, NULL);

	scene_manager->Post_Render_Scene(m_pd3dDevice, Active_CommandList, m_pCamera);


	hResult = Active_CommandList->Close();
	ID3D12CommandList* Post_Render_CommandLists[] = { Active_CommandList };
	p_CommandQueue->ExecuteCommandLists(1, Post_Render_CommandLists);
	WaitForGpuComplete();


#ifdef WRITE_TEXT_UI
	scene_manager->Render_Scene_UI(SwapChainBuffer_Index);

#endif

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 13, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);

}

