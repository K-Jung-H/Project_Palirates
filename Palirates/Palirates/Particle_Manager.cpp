#include "stdafx.h"
#include "Particle_Manager.h"

//==================================================
ID3D12RootSignature* ParticleShader::common_ComputeRootSignature = NULL;

ParticleShader::ParticleShader()
{
}

ParticleShader::~ParticleShader()
{
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE ParticleShader::GetPrimitiveTopologyType(int nPipelineState)
{
	if (nPipelineState == 0)
		return (D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
}

UINT ParticleShader::GetNumRenderTargets(int nPipelineState)
{
	return 1;
}

DXGI_FORMAT ParticleShader::GetRTVFormat(int nPipelineState, int nRenderTarget)
{
	return(DXGI_FORMAT_R8G8B8A8_UNORM);
}

DXGI_FORMAT ParticleShader::GetDSVFormat(int nPipelineState)
{
	return(DXGI_FORMAT_D24_UNORM_S8_UINT);
}

D3D12_SHADER_BYTECODE ParticleShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
		return(CShader::CompileShaderFromFile(L"Particle.hlsl", "VSParticleDraw", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE ParticleShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CreateGeometryShader(ppd3dShaderBlob, 0));
}

D3D12_SHADER_BYTECODE ParticleShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
		return(CShader::CompileShaderFromFile(L"Particle.hlsl", "PS_Transparent_ParticleDraw", "ps_5_1", ppd3dShaderBlob)); //PS_Deffered_ParticleDraw
}


D3D12_BLEND_DESC ParticleShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc = {};
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;

	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	if (nPipelineState == 0)
	{
		// snow
		d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

		d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	}
	else if (nPipelineState == 1)
	{
		// spark
		d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

		d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	}

	return d3dBlendDesc;
}

D3D12_DEPTH_STENCIL_DESC ParticleShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc = {};
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;

	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;

	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace = d3dDepthStencilDesc.FrontFace;

	return d3dDepthStencilDesc;
}


D3D12_INPUT_LAYOUT_DESC ParticleShader::CreateInputLayout(int nPipelineState)
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;

	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	//// 파티클 인스턴스 데이터
	pd3dInputElementDescs[1] = { "INSTANCE_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[2] = { "INSTANCE_VELOCITY", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12,	D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[3] = { "INSTANCE_COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 28, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC ParticleShader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

void ParticleShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL, * pd3dGeometryShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;

	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob, nPipelineState);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob, nPipelineState);

	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState(nPipelineState);
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

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dgraphicsPipelineStates[nPipelineState]);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs)
		delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void ParticleShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	//==================================================
	// Common Variables Part

	m_ngraphicsPipelineStates = 1;
	m_ppd3dgraphicsPipelineStates = new ID3D12PipelineState * [m_ngraphicsPipelineStates];

	CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0); 

	if (common_ComputeRootSignature == NULL)
		common_ComputeRootSignature = CreateComputeRootSignature(pd3dDevice);

	//==================================================

	m_ncomputePipelineStates = 2;
	m_ppd3dcomputePipelineStates = new ID3D12PipelineState * [m_ncomputePipelineStates];

	CreateComputePipelineState(pd3dDevice, common_ComputeRootSignature, 0); // Emit
	CreateComputePipelineState(pd3dDevice, common_ComputeRootSignature, 1); // Update

	m_cxThreadGroups = 1;
	m_cyThreadGroups = 1;
	m_czThreadGroups = 1;
}



D3D12_SHADER_BYTECODE ParticleShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return CShader::CompileShaderFromFile(L"Particles_Emit_CS.hlsl", "EmitCS", "cs_5_1", ppd3dShaderBlob);
	else if (nPipelineState == 1)
		return CShader::CompileShaderFromFile(L"Particles_Update_Extract_CS.hlsl", "Update_Extract_CS", "cs_5_1", ppd3dShaderBlob);

}

void ParticleShader::Set_ComputeRootSignature(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetComputeRootSignature(common_ComputeRootSignature);
}


ID3D12RootSignature* ParticleShader::CreateComputeRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dComputeRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[4];
	{
		pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		pd3dDescriptorRanges[0].NumDescriptors = 1;
		pd3dDescriptorRanges[0].BaseShaderRegister = 0; // u0
		pd3dDescriptorRanges[0].RegisterSpace = 0;
		pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		pd3dDescriptorRanges[1].NumDescriptors = 1;
		pd3dDescriptorRanges[1].BaseShaderRegister = 1; // u1
		pd3dDescriptorRanges[1].RegisterSpace = 0;
		pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		pd3dDescriptorRanges[2].NumDescriptors = 1;
		pd3dDescriptorRanges[2].BaseShaderRegister = 2;  // u2
		pd3dDescriptorRanges[2].RegisterSpace = 0;
		pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}
	D3D12_ROOT_PARAMETER pd3dRootParameters[4];
	{
		// b1 - ConstantBuffer 업데이트에 필요한 정보
		pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		pd3dRootParameters[0].Descriptor.ShaderRegister = 0; // Frame_Info
		pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[0].Constants.Num32BitValues = 12;
		pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// u0 - RWStructuredBuffer<Particle> : 파티클 데이터 버퍼 (읽기/쓰기 용도)
		pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
		pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// u2 - InstanceData : 인스턴스 정보만 추출한 버퍼
		pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
		pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
		pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

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

void ParticleShader::CreateComputePipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dComputeRootSignature, int nPipelineState)
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

void ParticleShader::Set_Compute_Pipeline(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (m_ppd3dcomputePipelineStates && m_ppd3dcomputePipelineStates[nPipelineState])
		pd3dCommandList->SetPipelineState(m_ppd3dcomputePipelineStates[nPipelineState]);
}


void ParticleShader::Update_Compute_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, CB_Particle_Update_Info* update_info)
{
	m_UpdateInfo = *update_info;
	pd3dCommandList->SetComputeRoot32BitConstants(0, sizeof(CB_Particle_Update_Info) / sizeof(UINT), &m_UpdateInfo, 0);
}

void ParticleShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);
}

void ParticleShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups)
{
	if (cxThreadGroups < 1 || cyThreadGroups < 1 || czThreadGroups < 1)
		return;
	else
	{
		pd3dCommandList->Dispatch(cxThreadGroups, cyThreadGroups, czThreadGroups);
	}
}

//------------------------------------------------------------------------------------------------

D3D12_SHADER_BYTECODE Spread_ParticleShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
		return(CShader::CompileShaderFromFile(L"Particle.hlsl", "VSParticleDraw", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE Spread_ParticleShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CreateGeometryShader(ppd3dShaderBlob, 0));
}

D3D12_SHADER_BYTECODE Spread_ParticleShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
		return(CShader::CompileShaderFromFile(L"Particle.hlsl", "PS_Transparent_ParticleDraw", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE Spread_ParticleShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return CShader::CompileShaderFromFile(L"Particles_Emit_CS.hlsl", "EmitCS", "cs_5_1", ppd3dShaderBlob);
	else if (nPipelineState == 1)
		return CShader::CompileShaderFromFile(L"Particles_Update_Extract_CS.hlsl", "Update_Spread_CS", "cs_5_1", ppd3dShaderBlob);

}


//===================================================================

Particle_Manager::Particle_Manager()
{
}

Particle_Manager::~Particle_Manager()
{

}

void Particle_Manager::Create_Particle_Manager(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	ParticleShader* spread_shader = new Spread_ParticleShader();
	spread_shader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

	//===================================================================

	particle_shader_map[Particle_Type::spread] = spread_shader;
	particle_shader_map[Particle_Type::sample_1] = NULL;
	particle_shader_map[Particle_Type::sample_2] = NULL;
}

void Particle_Manager::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
}

std::shared_ptr<ParticleObject> Particle_Manager::Add_Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, Particle_Shape_Mesh* particle_shape_mesh, Particle_Format particle_info)
{
	static int N = 0;
	std::shared_ptr<ParticleObject> new_particle_obj = make_shared<ParticleObject>();
	new_particle_obj->Set_Shape(particle_shape_mesh);
	new_particle_obj->Init_Info(particle_info);
	new_particle_obj->Set_Name(to_string(N));
	N++;

	Particle* new_particle_data = new Particle(pd3dDevice, pd3dCommandList, particle_info);
	new_particle_obj->Set_Particle_Data(new_particle_data);

	particle_object_list_map[particle_info.shader_type].push_back(new_particle_obj);

	return new_particle_obj;
}

void Particle_Manager::AnimateObjects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	ParticleShader::Set_ComputeRootSignature(pd3dCommandList);

	Emit_Particles(pd3dCommandList, fTimeElapsed);
	Update_and_Extract_Instance_Particles(pd3dCommandList, fTimeElapsed);
}

void Particle_Manager::Emit_Particles(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	CB_Particle_Update_Info update_info;

	for (auto& [type, shader_ptr] : particle_shader_map)
	{
		if (!shader_ptr)
			continue;

		shader_ptr->Set_Compute_Pipeline(pd3dCommandList, 0);

		for (std::shared_ptr<ParticleObject> particle_obj : particle_object_list_map[type])
		{
			std::pair<XMFLOAT3, XMFLOAT3> aabb_pos = particle_obj->GetAABB();

			Particle* particle_data = particle_obj->Get_Particle_Data();
			update_info.Max_Particle_N = particle_data->Get_Particle_Max_Num();
			update_info.ElapsedTime = fTimeElapsed;
			update_info.EmitRegionMin = aabb_pos.first;
			update_info.EmitRegionMax = aabb_pos.second;
			update_info.Main_Direction = particle_obj->Get_Main_Direction();
			update_info.Init_Velocity_Value = particle_obj->Get_Init_Velocity_Value();

			particle_data->UpdateBuffers(pd3dCommandList);
			shader_ptr->Update_Compute_ShaderVariables(pd3dCommandList, &update_info);

			UINT dispatchCount = (update_info.Max_Particle_N + THREAD_COUNT - 1) / THREAD_COUNT;
			shader_ptr->Dispatch(pd3dCommandList, dispatchCount, 1, 1);
		}
	}
}


void Particle_Manager::Update_and_Extract_Instance_Particles(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	CB_Particle_Update_Info update_info;

	for (auto& [type, shader_ptr] : particle_shader_map)
	{
		if (!shader_ptr)
			continue;

		shader_ptr->Set_Compute_Pipeline(pd3dCommandList, 1);

		for (std::shared_ptr<ParticleObject> particle_obj : particle_object_list_map[type])
		{
			std::pair<XMFLOAT3, XMFLOAT3> aabb_pos = particle_obj->GetAABB();
			

			Particle* particle_data = particle_obj->Get_Particle_Data();
			update_info.Max_Particle_N = particle_data->Get_Particle_Max_Num();
			update_info.ElapsedTime = fTimeElapsed;
			update_info.EmitRegionMin = aabb_pos.first;
			update_info.EmitRegionMax = aabb_pos.second;
			update_info.Main_Direction = particle_obj->Get_Main_Direction();
			update_info.Init_Velocity_Value = particle_obj->Get_Init_Velocity_Value();

			particle_data->UpdateBuffers(pd3dCommandList);
			shader_ptr->Update_Compute_ShaderVariables(pd3dCommandList, &update_info);


			UINT dispatchCount = (update_info.Max_Particle_N + THREAD_COUNT - 1) / THREAD_COUNT;
			shader_ptr->Dispatch(pd3dCommandList, dispatchCount, 1, 1);
		}
	}
}


void Particle_Manager::Clear_CounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList)
{

	for (auto& [type, shader_ptr] : particle_shader_map)
	{
		if (!shader_ptr)
			continue;
		for (std::shared_ptr<ParticleObject> particle_obj : particle_object_list_map[type])
		{
			Particle* particle_data = particle_obj->Get_Particle_Data();

			particle_data->Reset_Instance_CounterBuffer(pd3dCommandList);
			particle_data->Reset_Debug_Buffer(pd3dCommandList);
		}
	}

}

void Particle_Manager::Copy_CounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (auto& [type, shader_ptr] : particle_shader_map)
	{
		if (!shader_ptr)
			continue;
		for (std::shared_ptr<ParticleObject> particle_obj : particle_object_list_map[type])
		{
			Particle* particle_data = particle_obj->Get_Particle_Data();

			if (particle_data != NULL)
			{
				particle_data->Copy_CounterBuffer_All(pd3dCommandList);

			}
		}
	}
}


void Particle_Manager::Sync_AfterAnimate( Particle_Type type)
{
	for (std::shared_ptr<ParticleObject> particle_obj : particle_object_list_map[type])
	{
		Particle* particle_data = particle_obj->Get_Particle_Data();

		if (particle_data != NULL)
		{
			particle_data->Readback_All();


			//DebugOutput("======================================\n");
			//DebugOutput("Particle_Info_List : " + to_string(particle_data->N_Particle_Info_List) + "\n");
			//DebugOutput("Render_Instance : " + to_string(particle_data->N_Render_Instance) + "\n");
			//DebugOutput("======================================\n");
			//DebugOutput("\n");

		}
	}
}

void Particle_Manager::Sync_AfterAnimateObjects()
{
	for (auto& [type, shader_ptr] : particle_shader_map)
	{
		if (!shader_ptr)
			continue;

		Sync_AfterAnimate(type);

	}
}

void Particle_Manager::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, Particle_Type type)
{
	if (!particle_shader_map[type])
		return;

	particle_shader_map[type]->Setting_Render(pd3dCommandList);

	for (std::shared_ptr<ParticleObject> particle_obj : particle_object_list_map[type])
		particle_obj->Render(pd3dCommandList, pCamera);

}

void Particle_Manager::Render_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	Render(pd3dCommandList, pCamera, Particle_Type::spread);
	Render(pd3dCommandList, pCamera, Particle_Type::sample_1);
	Render(pd3dCommandList, pCamera, Particle_Type::sample_2);
}

