#pragma once


#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "UI_Manager.h"
#include "Scene_Manager.h"

struct CB_FRAMEWORK_INFO
{
	float m_fCurrentTime;      
	float m_fElapsedTime;         

	float m_fSecondsPerFirework;    
	int m_nFlareParticlesToEmit;     
	int m_nMaxFlareType2Particles;   
	XMFLOAT3 m_xmf3Gravity;          
};

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

	void CreateShaderVariables();
	void UpdateShaderVariables();
	void ReleaseShaderVariables();

    void Build_Scenes();
    void Release_Scenes();

    void ProcessInput();

	void Update_Scene();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();
	void Clear_RenderTarget(XMFLOAT3 background_color);


	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;
        
	IDXGIFactory4				*m_pdxgiFactory = NULL;
	IDXGISwapChain3				*m_pdxgiSwapChain = NULL;
	ID3D12Device				*m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			N_SwapChainBuffers = 2;
	UINT						SwapChainBuffer_Index;

	//=======================================================
	//	RTV
	ID3D12Resource				*ptr_SwapChainBackBuffer_List[N_SwapChainBuffers];
	ID3D12DescriptorHeap		*ptr_Rtv_DescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		SwapChainBack_Buffer_RTV_CPUHandle_list[N_SwapChainBuffers];
	//=======================================================
	// DSV
	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		DsvDescriptorCPUHandle;
	//=======================================================
	// Command
	ID3D12CommandQueue			*p_CommandQueue = NULL;

	ID3D12CommandAllocator		*Compute_CommandAllocator = NULL;
	ID3D12CommandAllocator		*Render_CommandAllocator = NULL;

	ID3D12GraphicsCommandList	*Compute_CommandList = NULL;
	ID3D12GraphicsCommandList* Render_CommandList = NULL;

	// 사용할 커멘드 할당자, 큐로 연결하여 사용
	ID3D12CommandAllocator* Active_CommandAllocator = NULL;
	ID3D12GraphicsCommandList* Active_CommandList = NULL;
	//=======================================================

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[N_SwapChainBuffers];
	HANDLE						m_hFenceEvent;
	//=======================================================
#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif
protected:
	ID3D12Resource* FrameworkInfo = NULL;
	CB_FRAMEWORK_INFO* MappedFrameworkInfo = NULL;
	
	CGameTimer					m_GameTimer;

public:
	CPostProcessingShader* PostProcessing_shader = NULL;
	Scene_Manager* scene_manager = NULL;



	CPlayer						*m_pPlayer = NULL;
	CCamera						*m_pCamera = NULL;




	POINT						m_ptOldCursorPos;
	_TCHAR						m_pszFrameRate[70];

#ifdef WRITE_TEXT_UI
	Text_UI_Renderer* text_ui_renderer = NULL;
#endif 
};

