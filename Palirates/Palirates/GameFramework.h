#pragma once


#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "UI_Manager.h"
#include "Scene_Manager.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

enum class GPU_Stage
{
	Compute,
	Render,
	Post
};

class ServerSyncManager
{
public:
	void AddPlayerSyncData(int playerId, const ServerAnimationSyncData& data)
	{
		syncDataMap[playerId] = data;
	}

	ServerAnimationSyncData& GetPlayerSyncData(int clientNum) {
		return syncDataMap.at(clientNum); 
	}

	std::unordered_map<int, ServerAnimationSyncData>& GetAllSyncData()
	{
		return syncDataMap;
	}

	void ClearAll() { syncDataMap.clear(); }

private:
	std::unordered_map<int, ServerAnimationSyncData> syncDataMap;
};

struct CB_FRAMEWORK_INFO
{
	float m_fCurrentTime;      
	float m_fElapsedTime;         
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

	void PrepareStage(GPU_Stage stage);

	void BeginGPUStage(GPU_Stage stage);
	void EndGPUStage(GPU_Stage stage, bool wait = true);

	HRESULT SignalFence(GPU_Stage stage, bool shouldAdvanceFence);
	void WaitForGpuComplete(GPU_Stage stage);
	void SafeSyncStage(GPU_Stage stage);
	
	UINT64 GetFenceValue(GPU_Stage stage, UINT bufferIndex) const;


	void MoveToNextFrame();


	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//=================서버=================
	void ConnectToServer(const std::string& ip, int port);
	void SendPacket();
	std::string ReceiveData();
	void NetworkLoop();
	void Disconnect();

	Scene_Manager sceneManager;
	//=================서버=================

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

	ID3D12Resource* ptr_RTV_Buffer_List[RTV_Format_Num];
	D3D12_CPU_DESCRIPTOR_HANDLE		RTV_Buffer_CPUHandle_list[RTV_Format_Num];

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

	ID3D12CommandAllocator* Post_CommandAllocator = nullptr;
	ID3D12GraphicsCommandList* Post_CommandList = nullptr;

	// 사용할 커멘드 할당자, 큐로 연결하여 사용
	ID3D12CommandAllocator* Active_CommandAllocator = NULL;
	ID3D12GraphicsCommandList* Active_CommandList = NULL;
	//=======================================================

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[N_SwapChainBuffers];
	HANDLE						m_hFenceEvent;

	UINT64						m_ComputeFenceValues[N_SwapChainBuffers];
	UINT64						m_RenderFenceValues[N_SwapChainBuffers];
	UINT64						m_PostFenceValues[N_SwapChainBuffers];


	//=======================================================

	//=================서버=================
	SOCKET serverSocket;
	sockaddr_in serverAddr;
	std::thread networkThread;
	std::mutex networkMutex;
	bool isRunning;
	//=================서버=================

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif
protected:
	ID3D12Resource* FrameworkInfo = NULL;
	CB_FRAMEWORK_INFO* MappedFrameworkInfo = NULL;
	
	CGameTimer					m_GameTimer;

public:
	Scene_Manager* scene_manager = NULL;
	Post_Effect_Manager* post_effect_manager = NULL;

	PostProcessBaseShader* MRT_shader = NULL;


	CPlayer						*m_pPlayer = NULL;
	CCamera						*m_pCamera = NULL;



	POINT						m_ptOldCursorPos;
	_TCHAR						m_pszFrameRate[70];

	//=================서버=================
	Scene_Manager& GetSceneManager() { return *scene_manager; } 
	CPlayer* GetPlayer() { return m_pPlayer; }
	bool multiMode{ false };
	int nPlayer{ 0 };
	int ClientNum{ 0 };
	ServerSyncManager syncManager;
	ServerSyncManager& GetSyncManager() { return syncManager; }
	//=================서버=================

#ifdef WRITE_TEXT_UI
	Text_UI_Renderer* text_ui_renderer = NULL;
#endif 
};

