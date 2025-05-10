#include "stdafx.h"
#include "Shader_Compute.h"
#include "Scene.h"

ID3D12RootSignature* Post_ComputeShader::Post_ComputeRootSignature_ptr = NULL;
D3D12_GPU_DESCRIPTOR_HANDLE Post_ComputeShader::g_BackBufferSRVs[2] = {};

Post_ComputeShader::Post_ComputeShader()
{
}

Post_ComputeShader::~Post_ComputeShader()
{
}

D3D12_SHADER_BYTECODE Post_ComputeShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}


void Post_ComputeShader::CreateShader(ID3D12Device* pd3dDevice, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState, DXGI_FORMAT format)
{
	n_Post_computePipelineStates = 1;
	Post_computePipelineStates = new ID3D12PipelineState * [n_Post_computePipelineStates];


	if (Post_ComputeRootSignature_ptr == NULL)
		Post_ComputeRootSignature_ptr = CreateComputeRootSignature(pd3dDevice);

	CreateComputePipelineState(pd3dDevice, Post_ComputeRootSignature_ptr, nPipelineState);

	m_cxThreadGroups = cxThreadGroups;
	m_cyThreadGroups = cyThreadGroups;
	m_czThreadGroups = czThreadGroups;

	constexpr UINT CX_THREADS = 32;
	constexpr UINT CY_THREADS = 32;

	m_cxThreadGroups = (FRAME_BUFFER_WIDTH + CX_THREADS - 1) / CX_THREADS;
	m_cyThreadGroups = (FRAME_BUFFER_HEIGHT + CY_THREADS - 1) / CY_THREADS;
	m_czThreadGroups = 1;

}

ID3D12RootSignature* Post_ComputeShader::CreateComputeRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dComputeRootSignature = NULL;

	// 루트 파라미터 설정
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[3];
	{
		pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[0].NumDescriptors = 1;
		pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0: Texture2D
		pd3dDescriptorRanges[0].RegisterSpace = 0;
		pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

		pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[1].NumDescriptors = 1;
		pd3dDescriptorRanges[1].BaseShaderRegister = 1; //t1: Velocity
		pd3dDescriptorRanges[1].RegisterSpace = 0;
		pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

		pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		pd3dDescriptorRanges[2].NumDescriptors = 1;
		pd3dDescriptorRanges[2].BaseShaderRegister = 0; //u0: RWTexture2D
		pd3dDescriptorRanges[2].RegisterSpace = 0;
		pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = 0;
	}

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];
	{
		pd3dRootParameters[BACK_BUFFER_SRV_ROOT_PARAMETER_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[BACK_BUFFER_SRV_ROOT_PARAMETER_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[BACK_BUFFER_SRV_ROOT_PARAMETER_INDEX].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture2D
		pd3dRootParameters[BACK_BUFFER_SRV_ROOT_PARAMETER_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[MOTION_VELOCITY_SRV_ROOT_PARAMETER_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[MOTION_VELOCITY_SRV_ROOT_PARAMETER_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[MOTION_VELOCITY_SRV_ROOT_PARAMETER_INDEX].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; //Texture2D
		pd3dRootParameters[MOTION_VELOCITY_SRV_ROOT_PARAMETER_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[RESULT_ROOT_PARAMETER_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[RESULT_ROOT_PARAMETER_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[RESULT_ROOT_PARAMETER_INDEX].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2]; //RWTexture2D
		pd3dRootParameters[RESULT_ROOT_PARAMETER_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc = {};
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = nullptr;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;


	HRESULT b = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);

	if (FAILED(b))
	{
		if (pd3dErrorBlob)
		{
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
			pd3dErrorBlob->Release();
		}
		return nullptr;
	}

	b = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dComputeRootSignature);
	
	if (FAILED(b))
	{
		if (pd3dErrorBlob)
		{
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
			pd3dErrorBlob->Release();
		}
	}


	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dComputeRootSignature);
}

void Post_ComputeShader::CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dComputeRootSignature, int nPipelineState)
{
	ID3DBlob* pd3dComputeShaderBlob = NULL;

	// 계산 파이프라인 상태 구성 구조체
	D3D12_COMPUTE_PIPELINE_STATE_DESC d3dComputePipelineStateDesc;
	::ZeroMemory(&d3dComputePipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	d3dComputePipelineStateDesc.pRootSignature = pd3dComputeRootSignature;
	d3dComputePipelineStateDesc.CS = CreateComputeShader(&pd3dComputeShaderBlob, nPipelineState);
	d3dComputePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	// 계산 파이프라인 상태 객체 생성
	HRESULT hResult = pd3dDevice->CreateComputePipelineState(&d3dComputePipelineStateDesc, IID_PPV_ARGS(&Post_computePipelineStates[nPipelineState]));

	if (pd3dComputeShaderBlob) 
		pd3dComputeShaderBlob->Release();
}

void Post_ComputeShader::CreateBackBufferSRV(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, UINT index, DXGI_FORMAT dxgiSrvFormat)
{
	g_BackBufferSRVs[index] = CDescriptor_Heap::CreateShaderResourceView(pd3dDevice, pd3dResource, dxgiSrvFormat);
}

// pd3dDevice         : D3D12 디바이스 포인터
// nUavs              : 생성할 UAV 리소스 개수
// rootParameterIndex : 루트 시그니처에서 바인딩할 슬롯 번호 (register(uX))
// format             : 생성할 UAV 텍스처 포맷
void Post_ComputeShader::CreateResourcesAndUavs(ID3D12Device* pd3dDevice, UINT nUavs, UINT rootParameterIndex, DXGI_FORMAT format)
{
	m_pTexture = new CTexture(nUavs, RESOURCE_TEXTURE2D, 0, 0, 1, 0, 0, nUavs, 0);


	for (UINT i = 0; i < nUavs; ++i)
	{
		m_pTexture->CreateTexture(pd3dDevice, nullptr, i, RESOURCE_TEXTURE2D, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT,
			1, 1, format, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr);
	}

	// 루트 파라미터 인덱스에 연속 UAV 디스크립터 등록
	CDescriptor_Heap::CreateComputeUnorderedAccessViews(pd3dDevice, m_pTexture, 1, rootParameterIndex, 0);

	// 다른 셰이더의 Srv 리소스로 전달하는 목적의 뷰
	m_PostOutputSRV = CDescriptor_Heap::CreateShaderResourceView(pd3dDevice, m_pTexture->GetResource(0), format);
}

void Post_ComputeShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void Post_ComputeShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	// 결과물 저장용 Uav
	m_pTexture->UpdateComputeUavShaderVariables(pd3dCommandList);
}

void Post_ComputeShader::ReleaseShaderVariables()
{

}

void Post_ComputeShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (Post_computePipelineStates && Post_computePipelineStates[nPipelineState])
		pd3dCommandList->SetPipelineState(Post_computePipelineStates[nPipelineState]);
}

void Post_ComputeShader::OnPrepare_RootSignature(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (Post_ComputeRootSignature_ptr)
		pd3dCommandList->SetComputeRootSignature(Post_ComputeRootSignature_ptr);
}

void Post_ComputeShader::Set_BackBuffer_SRV(ID3D12GraphicsCommandList* pd3dCommandList, int back_buffer_index)
{
	pd3dCommandList->SetComputeRootDescriptorTable(BACK_BUFFER_SRV_ROOT_PARAMETER_INDEX, g_BackBufferSRVs[back_buffer_index]);
}

void Post_ComputeShader::Set_RootSignature_SRV(ID3D12GraphicsCommandList* pd3dCommandList, int rootsignature_index, D3D12_GPU_DESCRIPTOR_HANDLE srv_handle)
{
	pd3dCommandList->SetComputeRootDescriptorTable(rootsignature_index, srv_handle);

}


void Post_ComputeShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	UpdateShaderVariables(pd3dCommandList);
	pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);
}

void Post_ComputeShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState)
{
	UpdateShaderVariables(pd3dCommandList);
	pd3dCommandList->Dispatch(cxThreadGroups, cyThreadGroups, czThreadGroups);
}

//==============================================================================

CEdgeDetectCSShader::CEdgeDetectCSShader()
{

}

CEdgeDetectCSShader::~CEdgeDetectCSShader()
{

}


D3D12_SHADER_BYTECODE CEdgeDetectCSShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Post_Compute_Shaders.hlsl", "CS_EdgeDetection", "cs_5_1", ppd3dShaderBlob));
}

void CEdgeDetectCSShader::CreateShader(ID3D12Device* pd3dDevice, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState, DXGI_FORMAT format)
{
	Post_ComputeShader::CreateShader(pd3dDevice, cxThreadGroups, cyThreadGroups, czThreadGroups, nPipelineState);

	CreateResourcesAndUavs(pd3dDevice, 1, RESULT_ROOT_PARAMETER_INDEX, format);
}

//==============================================================================

CMotionBlurShader::CMotionBlurShader()
{
}

CMotionBlurShader::~CMotionBlurShader()
{
}


D3D12_SHADER_BYTECODE CMotionBlurShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Post_Compute_Shaders.hlsl", "CS_MotionBlur", "cs_5_1", ppd3dShaderBlob));
}

void CMotionBlurShader::CreateShader(ID3D12Device* pd3dDevice, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState, DXGI_FORMAT format)
{
	Post_ComputeShader::CreateShader(pd3dDevice, cxThreadGroups, cyThreadGroups, czThreadGroups, nPipelineState);

	CreateResourcesAndUavs(pd3dDevice, 1, RESULT_ROOT_PARAMETER_INDEX, format);
}


//==========================================================================================

CTextureToFullScreenShader::CTextureToFullScreenShader()
{

}

CTextureToFullScreenShader::~CTextureToFullScreenShader()
{

}

void CTextureToFullScreenShader::CreateShader(ID3D12Device* pd3dDevice)
{
	m_ngraphicsPipelineStates = 1;
	m_ppd3dgraphicsPipelineStates = new ID3D12PipelineState * [m_ngraphicsPipelineStates];

	FullScreen_RootSignature_ptr = CreateGraphicsRootSignature(pd3dDevice);

	CreateGraphicsPipelineState(pd3dDevice, FullScreen_RootSignature_ptr, 0);
}

D3D12_INPUT_LAYOUT_DESC CTextureToFullScreenShader::CreateInputLayout(int nPipelineState)
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CTextureToFullScreenShader::CreateVertexShader(ID3DBlob** VertexShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Deffered_Shaders.hlsl", "VS_FullScreen", "vs_5_1", VertexShaderBlob));
	else
	{
		D3D12_SHADER_BYTECODE d3dShaderByteCode = { 0, NULL };
		return 		d3dShaderByteCode;
	}

}

D3D12_SHADER_BYTECODE CTextureToFullScreenShader::CreatePixelShader(ID3DBlob** PixelShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Deffered_Shaders.hlsl", "PS_FullScreen", "ps_5_1", PixelShaderBlob));
	else
	{
		D3D12_SHADER_BYTECODE d3dShaderByteCode = { 0, NULL };
		return 		d3dShaderByteCode;
	}
}

D3D12_DEPTH_STENCIL_DESC CTextureToFullScreenShader::CreateDepthStencilState(int n)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}


D3D12_BLEND_DESC CTextureToFullScreenShader::CreateBlendState(int n)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;

	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

ID3D12RootSignature* CTextureToFullScreenShader::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];
	{
		pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[0].NumDescriptors = 1;
		pd3dDescriptorRanges[0].BaseShaderRegister = 0; // Screen Texture
		pd3dDescriptorRanges[0].RegisterSpace = 0;
		pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	}
	D3D12_ROOT_PARAMETER pd3dRootParameters[1];
	{
		pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
		pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	}

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CTextureToFullScreenShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (FullScreen_RootSignature_ptr)
		pd3dCommandList->SetGraphicsRootSignature(FullScreen_RootSignature_ptr);

	if (m_ppd3dgraphicsPipelineStates && m_ppd3dgraphicsPipelineStates[nPipelineState])
		pd3dCommandList->SetPipelineState(m_ppd3dgraphicsPipelineStates[nPipelineState]);

}

void CTextureToFullScreenShader::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void CTextureToFullScreenShader::Set_SRV_ScreenTexture(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_GPU_DESCRIPTOR_HANDLE srv_handle)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(BACK_BUFFER_SRV_ROOT_PARAMETER_INDEX, srv_handle);
}

//=====================================================================
UINT Post_Effect_Manager::Frame_Buffer_Width = FRAME_BUFFER_WIDTH;
UINT Post_Effect_Manager::Frame_Buffer_Height = FRAME_BUFFER_HEIGHT;


Post_Effect_Manager::Post_Effect_Manager(ID3D12Device* pd3dDevice)
{
	CEdgeDetectCSShader* edge_detect_shader = new CEdgeDetectCSShader();
	CMotionBlurShader* motion_blur_shader = new CMotionBlurShader();

	fullscreen_shader = new CTextureToFullScreenShader();


	m_EffectMap[Effect_Type::Motion_Blur] = motion_blur_shader;
	m_EffectMap[Effect_Type::Outline] = edge_detect_shader;
	m_EffectMap[Effect_Type::etc] = NULL;


	edge_detect_shader->CreateShader(pd3dDevice);
	motion_blur_shader->CreateShader(pd3dDevice);

	fullscreen_shader->CreateShader(pd3dDevice);
}

void Post_Effect_Manager::Clear_Reserved_Effect()
{
	m_ActiveEffects.clear();
}

void Post_Effect_Manager::Add_Effect(Effect_Type type, UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE* srvHandle)
{
	ReservedEffect effect;
	effect.type = type;
	effect.root_param_index = rootIndex;
	effect.srv_handle = srvHandle; 

	m_ActiveEffects.push_back(effect);
}

void Post_Effect_Manager::Apply_Effect(ID3D12GraphicsCommandList* pd3dCommandList, UINT back_buffer_index)
{
	if (!m_ActiveEffects.size())
		return;
	
	Post_ComputeShader* shader = NULL;
	Post_ComputeShader::OnPrepare_RootSignature(pd3dCommandList);
	D3D12_GPU_DESCRIPTOR_HANDLE input_srv = Post_ComputeShader::g_BackBufferSRVs[back_buffer_index];

	for (const ReservedEffect& reserved : m_ActiveEffects)
	{
		shader = m_EffectMap[reserved.type];
		if (!shader) 
			continue;

		SynchronizeResourceTransition(pd3dCommandList, shader->GetOutputTextureResource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		shader->OnPrepareRender(pd3dCommandList);

		// uses the last output(SRV) as input
		shader->Set_RootSignature_SRV(pd3dCommandList, BACK_BUFFER_SRV_ROOT_PARAMETER_INDEX, input_srv);

		if (reserved.srv_handle)
			shader->Set_RootSignature_SRV(pd3dCommandList, reserved.root_param_index, *reserved.srv_handle);

		shader->Dispatch(pd3dCommandList);

		SynchronizeResourceTransition(pd3dCommandList, shader->GetOutputTextureResource(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		// Passed as input to the next effect
		input_srv = shader->GetOutputTextureSRV(); 
	}

	// Pass the final result to the fullscreen shader
	fullscreen_shader->OnPrepareRender(pd3dCommandList);
	fullscreen_shader->Set_SRV_ScreenTexture(pd3dCommandList, input_srv);
	fullscreen_shader->Render(pd3dCommandList);

}

void Post_Effect_Manager::Resize_Screen_Size(UINT new_width, UINT new_height)
{
	Frame_Buffer_Width = new_width;
	Frame_Buffer_Height = new_height;


}
//=====================================================================
float CS_Wave_Shader::total_time = 0.0f;
WaveParams* CS_Wave_Shader::update_wave_info = NULL;

D3D12_SHADER_BYTECODE CS_Wave_Shader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Wave.hlsl", "CS_Global_Wave_Height", "cs_5_1", ppd3dShaderBlob));
	else if (nPipelineState == 1)
		return(CShader::CompileShaderFromFile(L"Wave.hlsl", "CS_Boat_Wave_Height", "cs_5_1", ppd3dShaderBlob));
	else if (nPipelineState == 2)
		return(CShader::CompileShaderFromFile(L"Wave.hlsl", "CS_Wave_Normal", "cs_5_1", ppd3dShaderBlob));
}

ID3D12RootSignature* CS_Wave_Shader::CreateComputeRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dComputeRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[4];
	{
		pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // Read - HeightMap
		pd3dDescriptorRanges[0].NumDescriptors = 1;
		pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0: Texture2D
		pd3dDescriptorRanges[0].RegisterSpace = 0;
		pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

		pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; // Write - HeightMap
		pd3dDescriptorRanges[1].NumDescriptors = 1;
		pd3dDescriptorRanges[1].BaseShaderRegister = 0; //u0: RWTexture2D
		pd3dDescriptorRanges[1].RegisterSpace = 0;
		pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

		pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; // NormalMap
		pd3dDescriptorRanges[2].NumDescriptors = 1;
		pd3dDescriptorRanges[2].BaseShaderRegister = 1; //u1: RWTexture2D
		pd3dDescriptorRanges[2].RegisterSpace = 0;
		pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = 0;

		pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; // NormalMap
		pd3dDescriptorRanges[3].NumDescriptors = 1;
		pd3dDescriptorRanges[3].BaseShaderRegister = 2; //u1: RW Buffer for Normal
		pd3dDescriptorRanges[3].RegisterSpace = 0;
		pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = 0;
	}

	D3D12_ROOT_PARAMETER pd3dRootParameters[5];
	{
		pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //Frame_Info
		pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture2D
		pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; //RWTexture2D
		pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2]; //RWTexture2D
		pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3]; //RW Buffer
		pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc = {};
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = nullptr;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;


	HRESULT b = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);

	if (FAILED(b))
	{
		if (pd3dErrorBlob)
		{
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
			pd3dErrorBlob->Release();
		}
		return nullptr;
	}

	b = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dComputeRootSignature);

	if (FAILED(b))
	{
		if (pd3dErrorBlob)
		{
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
			pd3dErrorBlob->Release();
		}
	}


	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dComputeRootSignature);
}

void CS_Wave_Shader::CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dComputeRootSignature, int nPipelineState)
{
	ID3DBlob* pd3dComputeShaderBlob = NULL;

	// 계산 파이프라인 상태 구성 구조체
	D3D12_COMPUTE_PIPELINE_STATE_DESC d3dComputePipelineStateDesc;
	::ZeroMemory(&d3dComputePipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	d3dComputePipelineStateDesc.pRootSignature = pd3dComputeRootSignature;
	d3dComputePipelineStateDesc.CS = CreateComputeShader(&pd3dComputeShaderBlob, nPipelineState);
	d3dComputePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	// 계산 파이프라인 상태 객체 생성
	HRESULT hResult = pd3dDevice->CreateComputePipelineState(&d3dComputePipelineStateDesc, IID_PPV_ARGS(&Wave_computePipelineStates[nPipelineState]));

	if (pd3dComputeShaderBlob)
		pd3dComputeShaderBlob->Release();
}

void CS_Wave_Shader::CreateShader(ID3D12Device* pd3dDevice, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState, DXGI_FORMAT format)
{
	n_Wave_computePipelineStates = 3;
	Wave_computePipelineStates = new ID3D12PipelineState * [n_Wave_computePipelineStates];

	if (Wave_ComputeRootSignature_ptr == NULL)
		Wave_ComputeRootSignature_ptr = CreateComputeRootSignature(pd3dDevice);

	CreateComputePipelineState(pd3dDevice, Wave_ComputeRootSignature_ptr, 0);
	CreateComputePipelineState(pd3dDevice, Wave_ComputeRootSignature_ptr, 1);
	CreateComputePipelineState(pd3dDevice, Wave_ComputeRootSignature_ptr, 2);


	m_cxThreadGroups = cxThreadGroups;
	m_cyThreadGroups = cyThreadGroups;
	m_czThreadGroups = czThreadGroups;

	update_wave_info = new WaveParams();
}

void CS_Wave_Shader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(WaveParams) + 255) & ~255); //256의 배수

	Frame_Info = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	Frame_Info->Map(0, NULL, (void**)&m_pcbMappedFrame_Info);


	// === Global Wave Parameters ===
	update_wave_info->g_WaveSpeed = 0.5f;                            // Wave propagation speed
	update_wave_info->g_HeightDamping = 0.02f;                           // Damping factor for height interpolation
	update_wave_info->g_WaveMin = 0.0f;                            // Minimum wave height
	update_wave_info->g_WaveMax = 1.0f;                            // Maximum wave height
	update_wave_info->g_BaseSpacing = 0.01f;                           // Base spacing for wave pattern
	update_wave_info->g_BaseSharpness = 0.9f;                            // Wave sharpness (peak shaping)
	update_wave_info->g_BandSize = 30.0f;                         // Vertical layer height (band size)
	update_wave_info->g_AngleOffsetPerBand = XMConvertToRadians(5.1f);       // Direction offset per band in radians

	// === Boat Wake Parameters ===
	update_wave_info->g_WakeMaxDist = 150.0f;                          // Maximum distance the wake affects
	update_wave_info->g_WakeMaxAngle = XMConvertToRadians(30.0f);      // Maximum spread angle (Kelvin-like wake)
	update_wave_info->g_WakeDepthStrength = 1.0f;                            // Strength of depth indentation
	update_wave_info->g_WakeDecay = 5.0f;                            // Decay factor for lateral falloff

	// === Boat Position and Direction ===
	XMFLOAT3 boatPos = { 0.0f, 0.0f, 0.0f };                                    // World position of the boat
	XMFLOAT3 boatDir = { 0.0f, 1.0f, 0.0f };                                    // Normalized direction of the boat

	update_wave_info->g_BoatPos = XMFLOAT2(boatPos.x, boatPos.z);             // Use XZ only for 2D projection
	update_wave_info->g_BoatDir = XMFLOAT2(boatDir.x, boatDir.z);             // Use XZ direction for wake

	// === Time ===
	update_wave_info->g_TotalTime = 0.0f;							// Total accumulated time (in seconds)
	update_wave_info->_padding = 0.0f;                                      // Padding for 16-byte alignment
}

void CS_Wave_Shader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	update_wave_info->g_WakeMaxDist = std::min(update_wave_info->g_WakeMaxDist, 50.0f);

	::memcpy(m_pcbMappedFrame_Info, update_wave_info, sizeof(WaveParams));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = Frame_Info->GetGPUVirtualAddress();
	pd3dCommandList->SetComputeRootConstantBufferView(0, d3dGpuVirtualAddress);

}

void CS_Wave_Shader::ReleaseShaderVariables()
{

}

void CS_Wave_Shader::OnPrepareDispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (Wave_computePipelineStates && Wave_computePipelineStates[nPipelineState])
		pd3dCommandList->SetPipelineState(Wave_computePipelineStates[nPipelineState]);

	if (Wave_ComputeRootSignature_ptr)
		pd3dCommandList->SetComputeRootSignature(Wave_ComputeRootSignature_ptr);
}

void CS_Wave_Shader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);
}

void CS_Wave_Shader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups)
{
	pd3dCommandList->Dispatch(cxThreadGroups, cyThreadGroups, czThreadGroups);
}
