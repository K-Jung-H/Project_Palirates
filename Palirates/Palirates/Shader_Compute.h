#pragma once
#include "Shader.h"

#define BACK_BUFFER_SRV_ROOT_PARAMETER_INDEX 0 // 이전 렌더링 결과물
#define BLUR_INFO_SRV_ROOT_PARAMETER_INDEX 1 // 블러 정보 G 버퍼
#define VELOCITY_SRV_ROOT_PARAMETER_INDEX 2 // 속도 G 버퍼
#define RESULT_ROOT_PARAMETER_INDEX 3 // CS 동작 후 결과물
#define ZOOM_INFO_PARAMETER_INDEX 4 // ZoomBlur에 필요한 정보

class Post_ComputeShader : public PostProcessBaseShader
{
private:
	static ID3D12RootSignature* Post_ComputeRootSignature_ptr;

	int									n_Post_computePipelineStates = 0;
	ID3D12PipelineState** Post_computePipelineStates = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE m_PostOutputSRV {};

public:
	// 스왑체인 개수만큼 생성하기
	static D3D12_GPU_DESCRIPTOR_HANDLE g_BackBufferSRVs[2];

	Post_ComputeShader();
	virtual ~Post_ComputeShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState = 0);

	static ID3D12RootSignature* CreateComputeRootSignature(ID3D12Device* pd3dDevice);
	void CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dComputeRootSignature, int nPipelineState = 0);

	virtual void CreateShader(ID3D12Device* pd3dDevice, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1, int nPipelineState = 0, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	
	static void CreateBackBufferSRV(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, UINT index, DXGI_FORMAT dxgiSrvFormat);
	void CreateResourcesAndUavs(ID3D12Device* pd3dDevice, UINT nUavs, UINT rootParameterIndex, DXGI_FORMAT format);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);

	//////
	static void OnPrepare_RootSignature(ID3D12GraphicsCommandList* pd3dCommandList);
	static void Set_BackBuffer_SRV(ID3D12GraphicsCommandList* pd3dCommandList, int back_buffer_index);

	virtual void Set_RootSignature_SRV(ID3D12GraphicsCommandList* pd3dCommandList, int rootsignature_index, D3D12_GPU_DESCRIPTOR_HANDLE srv_handle);

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);
	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState = 0);

	D3D12_GPU_DESCRIPTOR_HANDLE GetOutputTextureSRV() { return  m_PostOutputSRV; }
	ID3D12Resource* Post_ComputeShader::GetOutputTextureResource() { return (m_pTexture) ? m_pTexture->GetResource(0) : nullptr; }

protected:
	UINT							m_cxThreadGroups = 0;
	UINT							m_cyThreadGroups = 0;
	UINT							m_czThreadGroups = 0;
};


class CEdgeDetectCSShader : public Post_ComputeShader
{
public:
	CEdgeDetectCSShader();
	virtual ~CEdgeDetectCSShader();

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState = 0);

	virtual void CreateShader(ID3D12Device* pd3dDevice, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1, int nPipelineState = 0, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

};

class CMotionBlurShader : public Post_ComputeShader
{
public:
	CMotionBlurShader();
	virtual ~CMotionBlurShader();

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState = 0);

	virtual void CreateShader(ID3D12Device* pd3dDevice, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1, int nPipelineState = 0, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

};

class CZoomBlurShader : public Post_ComputeShader
{
public:
	CZoomBlurShader();
	virtual ~CZoomBlurShader();

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState = 0);

	virtual void CreateShader(ID3D12Device* pd3dDevice, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1, int nPipelineState = 0, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

};


//=======================================================================

class CTextureToFullScreenShader : public CStandardShader
{
private:
	ID3D12RootSignature* FullScreen_RootSignature_ptr = NULL;
	CTexture* m_pTexture = NULL;

public:
	CTextureToFullScreenShader();
	virtual ~CTextureToFullScreenShader();

	virtual void CreateShader(ID3D12Device* pd3dDevice);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int n);
	virtual D3D12_BLEND_DESC CreateBlendState(int n);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);

	virtual ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);

	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Set_SRV_ScreenTexture(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_GPU_DESCRIPTOR_HANDLE srv_handle);
};

//========================================================================

struct ZoomBlurInfo_CB
{
	XMFLOAT2 screen_pos;
	float elapsed_time;
	float s_base_blur_strength;

	float min_influence_dist;

	float ripple_speed;
	float ripple_strength;
	float ripple_width;

	XMFLOAT4 ripple_blend_color;

	float ripple_interval = 0.5f;
	int max_ripples = 5;

	XMFLOAT2 padding0;
};


enum class Effect_Type
{
	Motion_Blur,
	Outline,
	Zoom,
	 etc,
};


struct Resource_Bind_Set
{
	UINT root_param_index;
	const D3D12_GPU_DESCRIPTOR_HANDLE* srv_handle = nullptr;
};


class Post_Effect_Manager
{
private:
	static UINT Frame_Buffer_Width;
	static UINT Frame_Buffer_Height;

	std::unordered_map<Effect_Type, Post_ComputeShader*> m_EffectMap;
	std::unordered_map<Effect_Type, vector<Resource_Bind_Set>> m_Effect_reserved;


	ID3D12Resource* m_pd3dcb_zoomblur_info = NULL;
	ZoomBlurInfo_CB* m_pcb_Mapped_zoomblur_info = NULL;
	float Accumulated_Time = 0.0f;

public:
	CTextureToFullScreenShader* fullscreen_shader = NULL;
	Post_Effect_Manager(ID3D12Device* device);

	void CreateShaderResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


	void Clear_Reserved_Effect();                    
	void Add_Effect(Effect_Type type, Resource_Bind_Set reserved);
	void Apply_Effect(ID3D12GraphicsCommandList* pd3dCommandList, UINT back_buffer_index);

	void Resize_Screen_Size(UINT new_width, UINT new_height);

	void Set_Zoom_Focus_and_Time(XMFLOAT2 screen_pos, float elapsed_time);
	void Create_ZoomBlur_Info(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Update_ZoomBlur_Info(ID3D12GraphicsCommandList* pd3dCommandList);

};

//========================================================================


struct alignas(16) WaveParams
{
	// === Global Wave Parameters ===
	float g_WaveSpeed;            // Wave propagation speed
	float g_HeightDamping;        // Heightmap smoothing factor
	float g_WaveMin;              // Minimum wave height
	float g_WaveMax;              // Maximum wave height

	float g_BaseSpacing;          // Base wave spacing (for tiling)
	float g_BaseSharpness;        // Sharpness of wave crest
	float g_BandSize;             // Band height (for layer blending)
	float g_AngleOffsetPerBand;   // Angle difference per band (in radians)

	// === Boat Wake Parameters ===
	float g_WakeMaxDist;          // Max wake distance (from boat front)
	float g_WakeMaxAngle;         // Spread angle (Kelvin wake style, in radians)
	float g_WakeDepthStrength;    // Wake indentation strength
	float g_WakeDecay;            // Lateral decay (higher = sharper center)

	XMFLOAT2 g_BoatPos;           // Boat position (x, z)
	XMFLOAT2 g_BoatDir;           // Boat direction (normalized)

	float g_TotalTime;            // Global time (for wave animation)
	float _padding;               // Padding for 16-byte alignment
};

class CS_Wave_Shader : CShader
{
private:
	int									n_Wave_computePipelineStates = 0;
	ID3D12PipelineState** Wave_computePipelineStates = NULL;


	ID3D12Resource* Frame_Info = NULL;
	WaveParams* m_pcbMappedFrame_Info = NULL;

public:
	ID3D12RootSignature* Wave_ComputeRootSignature_ptr;
	static float total_time;
	static WaveParams* update_wave_info;

	static void Prepare_WaveParams();

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState = 0);

	static ID3D12RootSignature* CreateComputeRootSignature(ID3D12Device* pd3dDevice);
	void CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dComputeRootSignature, int nPipelineState = 0);

	virtual void CreateShader(ID3D12Device* pd3dDevice, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1, int nPipelineState = 0, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void ReleaseShaderVariables();

	void OnPrepareDispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState);
	void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList);
	void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups);
protected:
	UINT							m_cxThreadGroups = 0;
	UINT							m_cyThreadGroups = 0;
	UINT							m_czThreadGroups = 0;
};