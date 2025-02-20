#include "stdafx.h"
#include "Particle_Manager.h"


//==============================================================================

CParticleObject::CParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CGameObject(1)
{
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CParticleObject::~CParticleObject()
{
}

void CParticleObject::ReleaseUploadBuffers()
{
	CGameObject::ReleaseUploadBuffers();
}

void CParticleObject::Animate(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CParticleMesh* particle_mesh = (CParticleMesh*)m_pMesh;

	// Draw buffer를 COPY_SOURCE 상태로 전환하고, UAV 버퍼를 COPY_DEST 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dStreamOutputBuffer, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dUAVBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

	// Draw buffer에서 UAV 버퍼로 복사
	pd3dCommandList->CopyResource(particle_mesh->m_pd3dUAVBuffer, particle_mesh->m_pd3dStreamOutputBuffer);

	// UAV 버퍼를 UNORDERED_ACCESS 상태로, Draw buffer는 COPY_DEST 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dUAVBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dStreamOutputBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

	// Draw buffer에 대해 UAV 뷰를 설정하고, Compute Shader 실행
	pd3dCommandList->SetComputeRootUnorderedAccessView(1, particle_mesh->m_pd3dStreamOutputBuffer->GetGPUVirtualAddress());
	pd3dCommandList->Dispatch(1, 1, 1);

	// UAV 버퍼를 COPY_SOURCE 상태로 전환하고, Draw buffer를 COPY_DEST 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dUAVBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	// UAV 버퍼에서 Draw buffer로 복사
	pd3dCommandList->CopyResource(particle_mesh->m_pd3dStreamOutputBuffer, particle_mesh->m_pd3dUAVBuffer);

	// Draw buffer를 다시 STREAM_OUT 상태로, UAV 버퍼는 UNORDERED_ACCESS 상태로 전환
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dStreamOutputBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT);
	::SynchronizeResourceTransition(pd3dCommandList, particle_mesh->m_pd3dUAVBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

}

void CParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int progress)
{
	/*
		if (Material_list.size())
		{
			int i = 0;
			for (std::shared_ptr<CMaterial> material_ptr : Material_list)
			{
				if (material_ptr)
				{
					CShader* pShader = material_ptr->m_pShader;
					if (pShader)
					{
						// PSO 순회 및 렌더링
						int pipelineStateNum = pShader->Get_Num_PipelineState();
						for (int j = 0; j < pipelineStateNum; ++j)
						{
							// PSO 설정
							pShader->Setting_Render(pd3dCommandList, j);

							// 재료(Material) 셰이더 변수 업데이트
							material_ptr->UpdateShaderVariable(pd3dCommandList);

							// 메쉬 렌더링
							m_pMesh->Render(pd3dCommandList, i);
						}
					}
	*/



	OnPrepareRender();

	if (progress == 0)
	{
		if (Material_list.size())
		{
			for (std::shared_ptr<CMaterial> material_ptr : Material_list)
			{
				if (material_ptr)
				{
					CShader* pShader = material_ptr->m_pShader;
					if (pShader)
					{
						pShader->OnPrepareRender(pd3dCommandList, 0); // == SetPipelineState
					}

					material_ptr->UpdateShaderVariable(pd3dCommandList); // 재질에 저장된 텍스처도 함께 업데이트 되는 함수임, bool 값으로 기능 분리하기

					UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

					if (particle_mesh)
					{
						particle_mesh->PreRender(pd3dCommandList, 0); //Stream Output
						particle_mesh->Render(pd3dCommandList, 0); //Stream Output
						particle_mesh->PostRender(pd3dCommandList, 0); //Stream Output
					}
				}
			}
		}
		else if (progress == 1)
		{
			for (std::shared_ptr<CMaterial> material_ptr : Material_list)
			{
				if (material_ptr)
				{
					CShader* pShader = material_ptr->m_pShader;
					if (pShader)
						pShader->OnPrepareRender(pd3dCommandList, 1); // == SetPipelineState
					

					if (particle_mesh)
						particle_mesh->PreRender(pd3dCommandList, 1); //Draw

					if (shape_mesh) 					
						shape_mesh->Instancing_Render(pd3dCommandList, particle_mesh->m_d3dParticleBufferView, particle_mesh->Get_Num()); //Draw
				}
			}
		}
	}
}


void CParticleObject::OnPostRender()
{
	if (particle_mesh)
		particle_mesh->OnPostRender(0); //Read Stream Output Buffer Filled Size
}

//==================================================

CParticleShader::CParticleShader()
{
}

CParticleShader::~CParticleShader()
{
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE CParticleShader::GetPrimitiveTopologyType(int nPipelineState)
{
	if (nPipelineState == 0)
		return(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	else if (nPipelineState == 1)
		return (D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
}

UINT CParticleShader::GetNumRenderTargets(int nPipelineState)
{
	return((nPipelineState == 0) ? 0 : 1);
}

DXGI_FORMAT CParticleShader::GetRTVFormat(int nPipelineState, int nRenderTarget)
{
	return((nPipelineState == 0) ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R8G8B8A8_UNORM);
}

DXGI_FORMAT CParticleShader::GetDSVFormat(int nPipelineState)
{
	return(DXGI_FORMAT_D24_UNORM_S8_UINT);
}

D3D12_SHADER_BYTECODE CParticleShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSParticleStreamOutput", "vs_5_1", ppd3dShaderBlob));
	else
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSParticleDraw", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CParticleShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSParticleStreamOutput", "gs_5_1", ppd3dShaderBlob));
	else
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSParticleDraw", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CParticleShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CreatePixelShader());
	else
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSParticleDraw", "ps_5_1", ppd3dShaderBlob));
}


D3D12_BLEND_DESC CParticleShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

D3D12_DEPTH_STENCIL_DESC CParticleShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
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

D3D12_INPUT_LAYOUT_DESC CParticleShader::CreateInputLayout(int nPipelineState)
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	if (nPipelineState == 0)
	{
		UINT nInputElementDescs = 4;
		D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

		pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[1] = { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[2] = { "LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[3] = { "PARTICLETYPE", 0, DXGI_FORMAT_R32_UINT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
		d3dInputLayoutDesc.NumElements = nInputElementDescs;
	}
	else if (nPipelineState == 1)
	{
		UINT nInputElementDescs = 6;
		D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

		pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		pd3dInputElementDescs[1] = { "COLOR",	 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		// 파티클 인스턴스 데이터
		pd3dInputElementDescs[2] = { "WORLD_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	2, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[3] = { "VELOCITY",				0, DXGI_FORMAT_R32G32B32_FLOAT,	2, 12, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[4] = { "LIFETIME",				0, DXGI_FORMAT_R32_FLOAT,				2, 24, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
		pd3dInputElementDescs[5] = { "PARTICLETYPE",		0, DXGI_FORMAT_R32_UINT,				2, 28, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

		d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
		d3dInputLayoutDesc.NumElements = nInputElementDescs;
	}
	return(d3dInputLayoutDesc);
}

D3D12_STREAM_OUTPUT_DESC CParticleShader::CreateStreamOuputState(int nPipelineState)
{
	D3D12_STREAM_OUTPUT_DESC d3dStreamOutputDesc;
	::ZeroMemory(&d3dStreamOutputDesc, sizeof(D3D12_STREAM_OUTPUT_DESC));

	if (nPipelineState == 0)
	{
		UINT nStreamOutputDecls = 4;
		D3D12_SO_DECLARATION_ENTRY* pd3dStreamOutputDecls = new D3D12_SO_DECLARATION_ENTRY[nStreamOutputDecls];
		pd3dStreamOutputDecls[0] = { 0, "POSITION", 0, 0, 3, 0 };
		pd3dStreamOutputDecls[1] = { 0, "VELOCITY", 0, 0, 3, 0 };
		pd3dStreamOutputDecls[2] = { 0, "LIFETIME", 0, 0, 1, 0 };
		pd3dStreamOutputDecls[3] = { 0, "PARTICLETYPE", 0, 0, 1, 0 };

		UINT* pBufferStrides = new UINT[1];
		pBufferStrides[0] = sizeof(CParticleVertex);

		d3dStreamOutputDesc.NumEntries = nStreamOutputDecls;
		d3dStreamOutputDesc.pSODeclaration = pd3dStreamOutputDecls;
		d3dStreamOutputDesc.NumStrides = 1;
		d3dStreamOutputDesc.pBufferStrides = pBufferStrides;
		d3dStreamOutputDesc.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;
	}

	return(d3dStreamOutputDesc);
}


void CParticleShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL, * pd3dGeometryShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob, nPipelineState);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob, nPipelineState);
	if (nPipelineState == 0)
	{
		d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryShaderBlob, nPipelineState);
		d3dPipelineStateDesc.StreamOutput = CreateStreamOuputState(nPipelineState);
	}
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.BlendState = CreateBlendState(nPipelineState);
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState(nPipelineState);
	d3dPipelineStateDesc.InputLayout = CreateInputLayout(nPipelineState);
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = GetPrimitiveTopologyType(nPipelineState);
	d3dPipelineStateDesc.NumRenderTargets = GetNumRenderTargets(nPipelineState);
	d3dPipelineStateDesc.RTVFormats[0] = GetRTVFormat(nPipelineState, 0);
	d3dPipelineStateDesc.DSVFormat = GetDSVFormat(nPipelineState);
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[nPipelineState]);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CParticleShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 2;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	m_ncomputePipelineStates = 1;
	m_ppd3dcomputePipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0); //Stream Output Pipeline State
	CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 1); //Draw Pipeline State

	m_pd3dComputeRootSignature = CreateComputeRootSignature(pd3dDevice);
	CreateComputePipelineState(pd3dDevice, m_pd3dComputeRootSignature);

}


D3D12_SHADER_BYTECODE CParticleShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return CShader::CompileShaderFromFile(L"ComputeShader.hlsl", "CSMain", "cs_5_1", ppd3dShaderBlob);
}


ID3D12RootSignature* CParticleShader::CreateComputeRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dComputeRootSignature = NULL;

	// 루트 파라미터 설정
	D3D12_ROOT_PARAMETER pd3dRootParameters[2];
	{
		// 상수 버퍼 (CBV) 정의
		pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[0].Descriptor.ShaderRegister = 0;  // b0
		pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// UAV 정의
		pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		pd3dRootParameters[1].Descriptor.ShaderRegister = 0;  // u0
		pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[1];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
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

void CParticleShader::CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dComputeRootSignature, int nPipelineState)
{
	ID3DBlob* pd3dComputeShaderBlob = NULL;

	// 계산 파이프라인 상태 구성 구조체
	D3D12_COMPUTE_PIPELINE_STATE_DESC d3dComputePipelineStateDesc;
	::ZeroMemory(&d3dComputePipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	d3dComputePipelineStateDesc.pRootSignature = pd3dComputeRootSignature;
	d3dComputePipelineStateDesc.CS = CreateComputeShader(&pd3dComputeShaderBlob, nPipelineState);
	d3dComputePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	// 계산 파이프라인 상태 객체 생성
	HRESULT hResult = pd3dDevice->CreateComputePipelineState(&d3dComputePipelineStateDesc, IID_PPV_ARGS(&m_ppd3dcomputePipelineStates[nPipelineState]));

	if (pd3dComputeShaderBlob) pd3dComputeShaderBlob->Release();
}

void CParticleShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	UINT ncbElementBytes = ((sizeof(CB_Particle_Update_Info) + 255) & ~255); //256의 배수
	m_pd3dcbParticlenfo = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbParticlenfo->Map(0, NULL, (void**)&m_pcbMappedParticleInfo);

	//=================================================


	//CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 2); // texture, random value

	particle_shape_mesh = new CSphereMeshDiffused(pd3dDevice, pd3dCommandList, 10.0f, 30, 30);

	particle_Material = new CMaterial();
	CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"texture/RoundSoftParticle.dds", RESOURCE_TEXTURE2D, 0);

	particle_Material->SetTexture(pParticleTexture);

	srand((unsigned)time(NULL));

	XMFLOAT4* pxmf4RandomValues = new XMFLOAT4[1024];
	for (int i = 0; i < 1024; i++)
	{
		pxmf4RandomValues[i].x = float((rand() % 10000) - 5000) / 5000.0f;
		pxmf4RandomValues[i].y = float((rand() % 10000) - 5000) / 5000.0f;
		pxmf4RandomValues[i].z = float((rand() % 10000) - 5000) / 5000.0f;
		pxmf4RandomValues[i].w = float((rand() % 10000) - 5000) / 5000.0f;
	}

	m_pRandowmValueTexture = new CTexture(1, RESOURCE_BUFFER, 0, 1);
	m_pRandowmValueTexture->CreateBuffer(pd3dDevice, pd3dCommandList, pxmf4RandomValues, 1024, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, 0);


	CScene::CreateShaderResourceViews(pd3dDevice, pParticleTexture, 0, PARAMETER_DEFAULT_TEXTURE);
	CScene::CreateShaderResourceViews(pd3dDevice, m_pRandowmValueTexture, 0, 14);

	particle_Material->SetShader(this);

}

void CParticleShader::Add_Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 pos, XMFLOAT3 velocity, XMFLOAT3 acceleration, XMFLOAT3 color, XMFLOAT2 size, UINT max_particles)
{
	CParticleObject* new_particle_obj = new CParticleObject(pd3dDevice, pd3dCommandList);
	CParticleMesh* new_particle_mesh = new CParticleMesh(pd3dDevice, pd3dCommandList, pos, velocity, 0.0f, acceleration, color, size, max_particles);

	new_particle_obj->SetShape(particle_shape_mesh);
	new_particle_obj->SetMesh(new_particle_mesh);
	new_particle_obj->SetMaterial(0, particle_Material);


	particle_list.push_back(new_particle_obj);
}

void CParticleShader::AnimateObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	pd3dCommandList->SetComputeRootSignature(m_pd3dComputeRootSignature);
	pd3dCommandList->SetPipelineState(m_ppd3dcomputePipelineStates[0]);

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbParticlenfo->GetGPUVirtualAddress();
	m_pcbMappedParticleInfo->gfElapsedTime = fTimeElapsed;



	for (CParticleObject* particle_obj : particle_list)
	{
		m_pcbMappedParticleInfo->Particle_N = particle_obj->Get_Num();
		pd3dCommandList->SetComputeRootConstantBufferView(0, d3dGpuVirtualAddress);

		particle_obj->Animate(pd3dDevice, pd3dCommandList);
	}
}

void CParticleShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int N)
{
	if (N == 0)
	{
		if (m_pRandowmValueTexture)
			m_pRandowmValueTexture->UpdateShaderVariables(pd3dCommandList);
	}

	for (CParticleObject* particle_obj : particle_list)
		particle_obj->Render(pd3dCommandList, pCamera, N);

};

void CParticleShader::OnPostRender()
{
	for (CParticleObject* particle_obj : particle_list)
		particle_obj->OnPostRender();
};