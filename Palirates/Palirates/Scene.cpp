//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

//=============================================================================================

shared_ptr<CShader> Shadow_Camera::shadow_map_shader = NULL;


Shadow_Camera::Shadow_Camera() : CCamera()
{
}

Shadow_Camera::~Shadow_Camera()
{

}

void Shadow_Camera::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CCamera::CreateShaderVariables(pd3dDevice, pd3dCommandList); 

	UINT ncbElementBytes = ((sizeof(LightCamera_Info) + 255) & ~255); //256의 배수
	m_pd3dcb_LightCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcb_LightCamera->Map(0, NULL, (void**)&m_pcb_MappedLightCamera);

	//===============================================================

	shadow_map = make_shared<CMaterial>(1);
	CTexture* shadowTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil = { 1.0f, 0 };

	shadowTexture->CreateTexture(pd3dDevice, pd3dCommandList, 0, RESOURCE_TEXTURE2D, _SHADOWMAP_WIDTH, _SHADOWMAP_HEIGHT, 1, 1, DXGI_FORMAT_R32_TYPELESS, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = CDescriptor_Heap::Get_Instance()->CreateDsv(pd3dDevice, shadowTexture, 0);
	shadowTexture->SetDSV(dsvHandle);


	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, shadowTexture, 0, ROOT_PARAMETER_FIXED_SHADOWMAP_TEXTURE_SRV_INDEX); 

	shadow_map->SetTexture(shadowTexture, 0);

}

void Shadow_Camera::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	shadow_map->Update_TextureShaderVariables(pd3dCommandList);

	XMMATRIX view = XMLoadFloat4x4(&m_xmf4x4View);
	XMMATRIX proj = XMLoadFloat4x4(&m_xmf4x4Projection);

	XMMATRIX texTransform = {
		0.5f,  0.0f,  0.0f, 0.0f,
		0.0f, -0.5f,  0.0f, 0.0f,
		0.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.0f, 1.0f
	};

	XMMATRIX viewProj = view * proj;
	XMMATRIX viewProjTex = viewProj * texTransform;
	XMStoreFloat4x4(&m_pcb_MappedLightCamera->LightViewProjTex, XMMatrixTranspose(viewProjTex));

	m_pcb_MappedLightCamera->shadow_pass = 1;
	m_pcb_MappedLightCamera->light_type = LIGHT_CAMERA_TYPE_DIRECTIONAL;
	m_pcb_MappedLightCamera->LightDirectionWS = m_light_direction;
	m_pcb_MappedLightCamera->LightPositionWS = m_light_position;

	m_pcb_MappedLightCamera->shadow_bias = 0.001f;

	m_pcb_MappedLightCamera->shadow_map_size = XMFLOAT2(static_cast<float>(_SHADOWMAP_WIDTH), static_cast<float>(_SHADOWMAP_HEIGHT));
	m_pcb_MappedLightCamera->inv_shadow_map_size = XMFLOAT2(1.0f / _SHADOWMAP_WIDTH, 1.0f / _SHADOWMAP_HEIGHT);

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_POST_SHADOW_INFO_CBV_INDEX, m_pd3dcb_LightCamera->GetGPUVirtualAddress());
}



void Shadow_Camera::SetupDirectionalLightCamera(XMFLOAT3& light_direction, float width, float height, float nearZ, float farZ)
{
	m_light_direction = Vector3::Normalize(XMFLOAT3(-1.0f, -1.0f, -1.0f)); // 45도 아래
	XMFLOAT3 sceneCenter = { 1280.0f, 0.0f, 1280.0f };
	XMFLOAT3 offset = Vector3::Scale(m_light_direction, -2000.0f);
	m_light_position = Vector3::Add(sceneCenter, offset);

	XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
	GenerateViewMatrix(m_light_position, sceneCenter, up);

	// 씬 크기를 전부 포함하는 orthographic projection
	float orthoWidth = 3000.0f;   // 씬보다 넉넉하게
	float orthoHeight = 3000.0f;
	float nearPlane = 1.0f;
	float farPlane = 5000.0f;

	XMMATRIX ortho = XMMatrixOrthographicLH(orthoWidth, orthoHeight, nearPlane, farPlane);
	XMStoreFloat4x4(&m_xmf4x4Projection, ortho);
}

D3D12_CPU_DESCRIPTOR_HANDLE Shadow_Camera::Get_Shadow_Map_DSV() const
{
	if (shadow_map)
	{
		CTexture* shadowTex = shadow_map->m_ppTextures[0];
		if (shadowTex)
			return shadowTex->GetDSVDescriptorHandle();
	}

	// 기본값 반환 (nullptr 방지용)
	return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
}

//=============================================================================================

std::shared_ptr<ID3D12RootSignature> CScene::m_MRT_GraphicsRootSignature = NULL;
std::shared_ptr<ID3D12RootSignature> CScene::m_Transparent_GraphicsRootSignature = NULL;
std::shared_ptr<ID3D12RootSignature> CScene::m_Plane_GraphicsRootSignature = NULL;


CScene::CScene()
{
}

CScene::~CScene()
{
	DebugOutput("\nDelete Scene");
}

ID3D12RootSignature* CScene::Create_MRT_GraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[8];
	{
		pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[0].NumDescriptors = 1;
		pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0: gtxtAlbedoTexture
		pd3dDescriptorRanges[0].RegisterSpace = 0;
		pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[1].NumDescriptors = 1;
		pd3dDescriptorRanges[1].BaseShaderRegister = 1; //t1: gtxtSpecularTexture
		pd3dDescriptorRanges[1].RegisterSpace = 0;
		pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[2].NumDescriptors = 1;
		pd3dDescriptorRanges[2].BaseShaderRegister = 2; //t2: gtxtNormalTexture
		pd3dDescriptorRanges[2].RegisterSpace = 0;
		pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[3].NumDescriptors = 1;
		pd3dDescriptorRanges[3].BaseShaderRegister = 3; //t3: gtxtMetallicTexture
		pd3dDescriptorRanges[3].RegisterSpace = 0;
		pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[4].NumDescriptors = 1;
		pd3dDescriptorRanges[4].BaseShaderRegister = 4; //t4: gtxtEmissionTexture
		pd3dDescriptorRanges[4].RegisterSpace = 0;
		pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[5].NumDescriptors = 1;
		pd3dDescriptorRanges[5].BaseShaderRegister = 5; //t5: gtxtTerrainBaseTexture
		pd3dDescriptorRanges[5].RegisterSpace = 0;
		pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[6].NumDescriptors = 1;
		pd3dDescriptorRanges[6].BaseShaderRegister = 6; //t6: gtxtTerrainDetailTexture
		pd3dDescriptorRanges[6].RegisterSpace = 0;
		pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[7].NumDescriptors = 1;
		pd3dDescriptorRanges[7].BaseShaderRegister = 7; //t7: gtxtSkyBoxTexture
		pd3dDescriptorRanges[7].RegisterSpace = 0;
		pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//=======================================================================
	}

	D3D12_ROOT_PARAMETER pd3dRootParameters[14];
	{
		// n = 0, b0 = Frame_Info
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].Descriptor.ShaderRegister = 0; //Frame_Info
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 1, b1 = GameObject
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.Num32BitValues = 28;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.ShaderRegister = 1;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 2, b2 = Camera
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].Descriptor.ShaderRegister = 2;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 3, b2 = Camera
		pd3dRootParameters[ROOT_PARAMETER_PREV_CAMERA_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_PREV_CAMERA_CBV_INDEX].Descriptor.ShaderRegister = 3;
		pd3dRootParameters[ROOT_PARAMETER_PREV_CAMERA_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_PREV_CAMERA_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 4, b4 = Skinned Bone Offsets
		pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET_CBV_INDEX].Descriptor.ShaderRegister = 4;
		pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		// n = 5, b5 = Skinned Bone Transforms
		pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM_CBV_INDEX].Descriptor.ShaderRegister = 5;
		pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;



		// n = 6, t0 = Albeo_Texture
		pd3dRootParameters[ROOT_PARAMETER_ALBEDO_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_ALBEDO_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_ALBEDO_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
		pd3dRootParameters[ROOT_PARAMETER_ALBEDO_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 7, t1 = Specular_Texture
		pd3dRootParameters[ROOT_PARAMETER_SPECULAR_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_SPECULAR_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_SPECULAR_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
		pd3dRootParameters[ROOT_PARAMETER_SPECULAR_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 8, t2 = Normal_Texture
		pd3dRootParameters[ROOT_PARAMETER_NORMAL_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_NORMAL_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_NORMAL_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
		pd3dRootParameters[ROOT_PARAMETER_NORMAL_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 9, t3 = Metallic_Texture
		pd3dRootParameters[ROOT_PARAMETER_METALLIC_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_METALLIC_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_METALLIC_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
		pd3dRootParameters[ROOT_PARAMETER_METALLIC_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 10, t4 = Emission_Texture
		pd3dRootParameters[ROOT_PARAMETER_EMISSION_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_EMISSION_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_EMISSION_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
		pd3dRootParameters[ROOT_PARAMETER_EMISSION_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 11, t5 = Terrain_Base_Texture
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_BASE_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_BASE_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_BASE_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_BASE_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 12, t6 = Terrain_Detail_Texture
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 13,  t7 = Sky_Box
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	}

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

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

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hr = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);

	if (FAILED(hr))
	{

		if (pd3dErrorBlob)
		{
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
		}
		else
		{

			OutputDebugStringA("Failed to create root signature.\n");
		}
	}

	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

ID3D12RootSignature* CScene::Create_Transparent_GraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];
	{
		pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[0].NumDescriptors = 1;
		pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0: gtxtAlbedoTexture
		pd3dDescriptorRanges[0].RegisterSpace = 0;
		pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	D3D12_ROOT_PARAMETER pd3dRootParameters[4];
	{
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].Descriptor.ShaderRegister = 0; //Frame_Info
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 1, b1 = GameObject
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.Num32BitValues = 28;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.ShaderRegister = 1;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 2, b2 = Camera
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].Descriptor.ShaderRegister = 2;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 3, t0 = Albeo_Texture
		pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
		pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

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

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hr = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);

	if (FAILED(hr))
	{

		if (pd3dErrorBlob)
		{
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
		}
		else
		{

			OutputDebugStringA("Failed to create root signature.\n");
		}
	}

	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

ID3D12RootSignature* CScene::Create_Plane_GraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[4];
	{
		pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[0].NumDescriptors = 1;
		pd3dDescriptorRanges[0].BaseShaderRegister = 0; // t0: Base_Texture
		pd3dDescriptorRanges[0].RegisterSpace = 0;
		pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[1].NumDescriptors = 1;
		pd3dDescriptorRanges[1].BaseShaderRegister = 1; // t1: Detail_Texture
		pd3dDescriptorRanges[1].RegisterSpace = 0;
		pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[2].NumDescriptors = 1;
		pd3dDescriptorRanges[2].BaseShaderRegister = 2; // t2: Normal_map
		pd3dDescriptorRanges[2].RegisterSpace = 0;
		pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[3].NumDescriptors = 1;
		pd3dDescriptorRanges[3].BaseShaderRegister = 3; // t3: Normal_map
		pd3dDescriptorRanges[3].RegisterSpace = 0;
		pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	D3D12_ROOT_PARAMETER pd3dRootParameters[7];
	{
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].Descriptor.ShaderRegister = 0; //Frame_Info
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 1, b1 = GameObject
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.Num32BitValues = 28;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.ShaderRegister = 1;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 2, b2 = Camera
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].Descriptor.ShaderRegister = 2;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 3, t0 = Base_Texture
		pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
		pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 4, t1 = Detail_Texture
		pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
		pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 5, t0 = Height_Map
		pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
		pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 6, t1 = Normal_Map
		pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
		pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

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
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hr = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);

	if (FAILED(hr))
	{

		if (pd3dErrorBlob)
		{
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
		}
		else
		{

			OutputDebugStringA("Failed to create root signature.\n");
		}
	}

	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 5;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_pLights[0].m_bEnable = false;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 300.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(250.0f, 50.0f, 250.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights[1].m_bEnable = false;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));


	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	m_pLights[3].m_bEnable = false;
	m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights[3].m_fRange = 600.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(550.0f, 330.0f, 530.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[3].m_fFalloff = 8.0f;
	m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	m_pLights[4].m_bEnable = false;
	m_pLights[4].m_nType = POINT_LIGHT;
	m_pLights[4].m_fRange = 200.0f;
	m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.3f, 0.3f, 1.0f);
	m_pLights[4].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[4].m_xmf3Position = XMFLOAT3(600.0f, 250.0f, 700.0f);
	m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);


	fixed_shadow_camera = std::make_shared<Shadow_Camera>();
	fixed_shadow_camera->shadow_active = false;
	for (int i = 0; i < m_nLights; ++i)
	{
		if (m_pLights[i].m_bEnable && m_pLights[i].m_nType == DIRECTIONAL_LIGHT)
		{
			fixed_shadow_camera->SetupDirectionalLightCamera(m_pLights[i].m_xmf3Direction);
			fixed_shadow_camera->shadow_active = true;
			break; 
		}
	}
}

void CScene::Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	BuildDefaultLightsAndMaterials();
	CS_Wave_Shader::Prepare_WaveParams();

	obj_manager = new Object_Manager();

	auto com_deleter = [](ID3D12RootSignature* p) { if (p) p->Release(); };


	if (!m_MRT_GraphicsRootSignature)
		m_MRT_GraphicsRootSignature = std::shared_ptr<ID3D12RootSignature>(Create_MRT_GraphicsRootSignature(pd3dDevice), com_deleter);

	if (!m_Transparent_GraphicsRootSignature)
		m_Transparent_GraphicsRootSignature = std::shared_ptr<ID3D12RootSignature>(Create_Transparent_GraphicsRootSignature(pd3dDevice), com_deleter);

	if (!m_Plane_GraphicsRootSignature)
		m_Plane_GraphicsRootSignature = std::shared_ptr<ID3D12RootSignature>(Create_Plane_GraphicsRootSignature(pd3dDevice), com_deleter);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);

	fog_info = make_shared<Fog_Info>();
	{
		fog_info->fogColor = XMFLOAT3(0.8f, 0.6f, 0.3f);
		fog_info->Fog_Trigger = false;

		fog_info->fogStart = 5.0f;
		fog_info->fogEnd = 200.0f;
		fog_info->fogDensity = 2.0f;
		fog_info->noiseScale = 0.001f;

		fog_info->noiseStrength = 0.5f;
		fog_info->time = 0.0f;
		fog_info->padding0 = XMFLOAT2(0.0f, 0.0f);
	}

}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);

	Shadow_Camera::shadow_map_shader = std::make_shared<CShadowMapShader>();
	Shadow_Camera::shadow_map_shader->CreateShader(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);
	Shadow_Camera::shadow_map_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	fixed_shadow_camera->CreateShaderVariables(pd3dDevice, pd3dCommandList);


	Object_Manager::trail_shader = std::make_shared<Trail_Shader>();
	Object_Manager::trail_shader->CreateShader(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
	Object_Manager::trail_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

#ifdef RENDER_WAVE
	std::shared_ptr<Wave_Object> wave_obj = std::make_shared<Wave_Object>(pd3dDevice, pd3dCommandList, m_Plane_GraphicsRootSignature, 3000, 10, false);
	wave_obj->Set_Name("in_game_wave");
	wave_obj->SetPosition(XMFLOAT3(1500.0f, -25.0f, 1500.0f));

	wave_obj->Set_BaseTexture(pd3dDevice, pd3dCommandList, L"Terrain/Wave_2.dds");
	wave_obj->Set_DetailTexture(pd3dDevice, pd3dCommandList, L"Terrain/Wave_2.dds");
	obj_manager->Set_Wave_Object(wave_obj);
#endif


#ifdef RENDER_PARTICLE
	particle_manager = new Particle_Manager();
	particle_manager->Create_Particle_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);

//	Particle_Shape_Mesh* tri_dust_shape_mesh = new Tetrahedron_Shape_Mesh(pd3dDevice, pd3dCommandList, 10.0f);
//	Particle_Shape_Mesh* sphere_shape_mesh = new Sphere_Shape_Mesh(pd3dDevice, pd3dCommandList, 20.0f);

	Particle_Format test_dragon_fire_info;
	{
		test_dragon_fire_info.shader_type = Particle_Type::loop;
		test_dragon_fire_info.particle_type = 5;
		test_dragon_fire_info.max_particles = 3000;
		test_dragon_fire_info.MaxLifetime = 1.0f;

		test_dragon_fire_info.area_xyz = XMFLOAT3(1000.0f, 1000.0f, 1000.0f);
		test_dragon_fire_info.EmitFaceIndex = 0;


		test_dragon_fire_info.main_direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
		test_dragon_fire_info.init_velocity_value = 100.0f;
		test_dragon_fire_info.acceleration = XMFLOAT3(0.0f, 10.0f, 0.0f);

		test_dragon_fire_info.size = 1.0f;
		test_dragon_fire_info.color = XMFLOAT3(1.0f, 0.5f, 0.0f);
	}

	Particle_Format test_sand_storm_info;
	{
		test_sand_storm_info.shader_type = Particle_Type::sand;
		test_sand_storm_info.particle_type = 3;
		test_sand_storm_info.max_particles = 10000;
		test_sand_storm_info.MaxLifetime = 10.0f;

		test_sand_storm_info.area_xyz = XMFLOAT3(2400.0f, 1000.0f, 2400.0f);
		test_sand_storm_info.EmitFaceIndex = 5;

		test_sand_storm_info.main_direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
		test_sand_storm_info.init_velocity_value = 100.0f;
		test_sand_storm_info.acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);

		test_sand_storm_info.size = 0.3f;
		test_sand_storm_info.color = XMFLOAT3(0.761f, 0.698f, 0.502f);
	}

	Particle_Format bleeding_info;
	{
		bleeding_info.shader_type = Particle_Type::interval;
		bleeding_info.particle_type = 6;
		bleeding_info.max_particles = 30;
		bleeding_info.MaxLifetime = 3.0f;

		bleeding_info.area_xyz = XMFLOAT3(500.0f, 500.0f, 500.0f);
		bleeding_info.EmitFaceIndex = 5;

		bleeding_info.main_direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
		bleeding_info.init_velocity_value = 50.0f;
		bleeding_info.acceleration = XMFLOAT3(0.0f, -9.8f, 0.0f);

		bleeding_info.size = 0.3f;
		bleeding_info.color = XMFLOAT3(1.0f, 0.3f, 0.0f);
	}
	shared_ptr<Particle_Shape_Mesh> particle_mesh;

	particle_mesh = particle_manager->Get_Particle_Mesh("cube");
	test_dragonfire = particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, particle_mesh, test_dragon_fire_info);
	test_dragonfire->Set_Active(false);


	particle_mesh = particle_manager->Get_Particle_Mesh("billboard");
	test_sand = particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, particle_mesh, test_sand_storm_info);
	test_sand->Set_BaseTexture(pd3dDevice, pd3dCommandList, L"Terrain/dust_particle.dds");
	test_sand->Set_Local_Coordinate();
	test_sand->SetPosition(1200.0f, 1000.0f, 1200.0f);
	test_sand->Set_Area(XMFLOAT3(2400.0f, 2000.0f, 2400.0f));
	
	particle_mesh = particle_manager->Get_Particle_Mesh("cube_dust");
	test_bleeding = particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, particle_mesh, bleeding_info);
	test_bleeding->Set_World_Coordinate();


#endif


#ifdef USING_OBB
	obj_manager->Create_OBB_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif

	XMFLOAT3 xmf3Scale(10.0f, 0.0f, 10.0f); // y = 0 -> 평지
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f); // HeightMap
	m_pTerrain = make_shared<CHeightMapTerrain>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 257, 257, xmf3Scale, xmf4Color, 8, 3);
	m_pTerrain->DivideIntoChildren(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, _T("Terrain/HeightMap.raw"), xmf3Scale, 8);
	m_pTerrain->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	obj_manager->Set_Terrain_Object(m_pTerrain);


	{
		string obj_name_1 = "Anubis";
		string obj_name_2 = "test_obj_name_2";
		string obj_name_3 = "FishMan";
		string obj_name_4 = "test_palyer2";
		string obj_name_5 = "test_palyer3";
		string obj_name_6 = "test_palyer4";
		string obj_name_7 = "test_palyer5";
		string obj_name_8 = "test_palyer6";


	/*	
		std::string_view name_view = obj_name_1;
		std::shared_ptr<CMonsterObject> AnubisObject = std::make_shared<CAnubisObject>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);
		AnubisObject->SetPosition(1450.0f, m_pTerrain->Get_Mesh_Height(1450.0f, 650.0f), 650.0f);
		AnubisObject->Set_Name(obj_name_1);
		AnubisObject->test_num = 1;
		AnubisObject->Set_Child(AnubisObject->m_pRootModel);
		AnubisObject->SetupWeaponCollider();
		obj_manager->Add_Object(AnubisObject, Object_Type::skinned);
	*/

		std::shared_ptr<CMonsterObject> Dragon = std::make_shared<CDragonObject>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);
		Dragon->Set_Child(Dragon->m_pRootModel);
		Dragon->SetupWeaponCollider();
		Dragon->SetPosition(1550.0f, m_pTerrain->Get_Mesh_Height(1550.0f, 680.0f), 680.0f);
		Dragon->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 0.0f));
		XMFLOAT3 tt2 = { 0.0f, 1.0f, 0.0f };
		Dragon->Rotate(&tt2, 180.0f);
		Dragon->test_num = 5;
		obj_manager->Add_Object(Dragon, Object_Type::skinned);


		/*for (int i = 0; i < 5; i++)
		{
			std::shared_ptr<CMonsterObject> m = std::make_shared<CFishManObject>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);
			m->Set_Child(m->m_pRootModel);
			m->SetupWeaponCollider();
			m->SetPosition(10.0f * i + 1450.0f, m_pTerrain->Get_Mesh_Height(10.0f * i + 1450.0f, 10.0f * i + 700.0f), 10.0f * i + 700.0f);
			m->Set_Name(obj_name_3);
			m->test_num = i + 4;
			obj_manager->Add_Object(m, Object_Type::skinned);
		}*/

#ifdef LOAD_SCENE


		CLoadedModelInfo* Test_Scene_Model = CGameObject::Load_Scene_File(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, "Scene/Scene_File_3/Scene_Name.bin", NULL);


		std::shared_ptr<CGameObject> test_scene = std::make_shared<CGameObject>();
		test_scene->Set_Name("test_scene");
		test_scene = Test_Scene_Model->m_pModelRootObject;
		test_scene->SetPosition(1300.0f, m_pTerrain->Get_Mesh_Height(1300.0f, 800.0f) - 27.0f , 800.0f);
		test_scene->SetScale({ 10.0f, 10.0f ,10.0f }, true);
		obj_manager->Add_Object(test_scene, Object_Type::fixed);
#endif
		//=====================================================


		Object_Manager::Reserve_Update();

#ifdef USING_OBB
		obj_manager->Update_OBB_Data(pd3dDevice, pd3dCommandList, Object_Type::etc);
		obj_manager->Update_OBB_Data(pd3dDevice, pd3dCommandList, Object_Type::fixed);
#endif
	}


#ifdef RENDER_PARTICLE
	obj_manager->Update(pd3dDevice, pd3dCommandList); // 미리 한번 업데이트 해야 파티클 메니저에서 fixed 타입 정보 얻을 수 있음
	obj_manager->Update_Fixed_OBBs(); // 내부에서 m_OBBDataArray 생성
	particle_manager->Create_OBB_Data_ShaderVariables(pd3dDevice, pd3dCommandList, obj_manager->Get_Fixed_OBBs());
#endif

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

#ifdef WRITE_TEXT_UI
void CScene::Build_Text_UI(Text_UI_Renderer* text_ui_renderer_ptr)
{
	text_ui_manager = new Text_UI_Manager(text_ui_renderer_ptr->m_pd2dWriteFactory, text_ui_renderer_ptr->m_pd2dDeviceContext);

	if (text_ui_manager)
	{
		std::shared_ptr<TextDesign> design_ptr = text_ui_manager->Create_Text_Design("White_Text", D2D1::ColorF(D2D1::ColorF::Black, 1.0f), L"Gothic", 20.0f);
		text_ui_manager->Add_Text_Design(design_ptr);

		D2D1_RECT_F player_pos_text_area = D2D1::RectF(0.0f, 0.0f, 400.0f, 30.0f);
		D2D1_RECT_F player_normal_text_area = D2D1::RectF(0.0f, 30.0f, 400.0f, 60.0f);
		D2D1_RECT_F tile_info_text_area = D2D1::RectF(0.0f, 60.0f, 200.0f, 90.0f);
		D2D1_RECT_F player_xz = D2D1::RectF(0.0f, 90.0f, 400.0f, 120.0f);
		D2D1_RECT_F player_state = D2D1::RectF(0.0f, 120.0f, 200.0f, 150.0f);
		D2D1_RECT_F player_Laststate = D2D1::RectF(0.0f, 150.0f, 400.0f, 180.0f);

		TextBlock* player_pos_text_block_ptr = new TextBlock(design_ptr, L"Player_pos: ", player_pos_text_area);
		TextBlock* player_normal_text_block_ptr = new TextBlock(design_ptr, L"Player_normal: ", player_normal_text_area);
		TextBlock* tile_info_text_block_ptr = new TextBlock(design_ptr, L"Tile: : ", tile_info_text_area);
		TextBlock* player_xz_ptr = new TextBlock(design_ptr, L"Player_XZ: : ", player_xz);
		TextBlock* player_state_ptr = new TextBlock(design_ptr, L"Player_state: : ", player_state);
		TextBlock* player_Laststate_ptr = new TextBlock(design_ptr, L"Player_LastState: : ", player_Laststate);

		text_ui_manager->Add_TextBlock(player_pos_text_block_ptr);
		text_ui_manager->Add_TextBlock(player_normal_text_block_ptr);
		text_ui_manager->Add_TextBlock(tile_info_text_block_ptr);
		text_ui_manager->Add_TextBlock(player_xz_ptr);
		text_ui_manager->Add_TextBlock(player_state_ptr);
		text_ui_manager->Add_TextBlock(player_Laststate_ptr);

	}
}

std::vector<TextBlock*>* CScene::Get_Text_List()
{
	if (text_ui_manager)
		return text_ui_manager->Get_Text_Block_List();
	else
		return NULL;
}

void CScene::Update_UI()
{
	static wchar_t Player_pos_Buffer[100];
	static wchar_t Player_normal_Buffer[100];
	static wchar_t Tile_Info_Buffer[100];
	static wchar_t Player_XZ_Buffer[100];
	static wchar_t Player_state_Buffer[100];
	static wchar_t Player_Laststate_Buffer[100];

	if (text_ui_manager)
	{
		XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
		int tile_n = 0;
		XMFLOAT3 tile_normal{};
		if (m_pTerrain) {
			tile_n = m_pTerrain->Get_Tile(xmf3Position.x, xmf3Position.z, m_pPlayer->Get_Last_Tile());
			tile_normal = m_pTerrain->Get_Mesh_Normal(xmf3Position.x, xmf3Position.z);
		}
			
		float player_x = m_pPlayer->GetMoveX();
		float player_z = m_pPlayer->GetMoveZ();
		State currentState = m_pPlayer->GetStateMachine()->Get_State();
		std::wstring stateStr = stateToStringMap[currentState];  
		State LastState = m_pPlayer->GetStateMachine()->Get_LastState();
		std::wstring LastStateStr = stateToStringMap[LastState];


		_stprintf_s(Player_pos_Buffer, 100, _T("Player_pos >>%.2f,%.2f,%.2f"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
		_stprintf_s(Player_normal_Buffer, 100, _T("Player_normal >> %.2f,%.2f,%.2f"), tile_normal.x, tile_normal.y, tile_normal.z);
		_stprintf_s(Tile_Info_Buffer, 100, _T("Tile  >> %d"), tile_n);
		_stprintf_s(Player_XZ_Buffer, 100, _T("Player_XZ  >> %.2f,%.2f"), player_x, player_z);
		_stprintf_s(Player_state_Buffer, 100, _T("Player_state  >> %s"), stateStr.c_str());
		_stprintf_s(Player_Laststate_Buffer, 100, _T("Player_LastState  >> %s"), LastStateStr.c_str());

		text_ui_manager->UpdateTextBlock(0, Player_pos_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(1, Player_normal_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(2, Tile_Info_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(3, Player_XZ_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(4, Player_state_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(5, Player_Laststate_Buffer, NULL, NULL);
	}
}

#endif

void CScene::ReleaseObjects()
{
	obj_manager->Clear_Object_List_All();
#ifdef WRITE_TEXT_UI
	delete text_ui_manager;
#endif

	if (Shader_list.size())
		for (std::shared_ptr<CShader> shader_ptr : Shader_list)
			shader_ptr.reset();
		
		if (m_pSkyBox) 
			delete m_pSkyBox;


	ReleaseShaderVariables();

	if (m_pLights) 
		delete[] m_pLights;
}


void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256 * N
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);


	fog_noise = make_shared<CMaterial>(1);
	CTexture* noise_texture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
	noise_texture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Test_Noise.dds", RESOURCE_TEXTURE2D, 0);

	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, noise_texture, 0, ROOT_PARAMETER_FOG_NOISE_TEXTURE_SRV_INDEX);

	fog_noise->SetTexture(noise_texture, 0);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CScene::UpdateShaderVariables_Light_Info(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_POST_LIGHT_INFO_CBV_INDEX, d3dcbLightsGpuVirtualAddress); //Lights
}

void CScene::UpdateShaderVariables_Fog_Info(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (fog_info->time >= 100.0f)
		fog_info->time = fmod(fog_info->time, 100.0f);

	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_FOG_INFO_INDEX, 8, fog_info.get(), 0);

	fog_noise->Update_TextureShaderVariables(pd3dCommandList);
}

void CScene::UpdateShaderVariables_ShadowMap(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if(fixed_shadow_camera)
		fixed_shadow_camera->UpdateShaderVariables(pd3dCommandList);

}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	if (Shader_list.size())
		for (std::shared_ptr<CShader> shader_ptr : Shader_list)
			shader_ptr->ReleaseUploadBuffers();

	
	std::vector<std::shared_ptr<CGameObject>>* skinned_obj_container = obj_manager->Get_Object_List(Object_Type::skinned);
	std::vector<std::shared_ptr<CGameObject>>* non_skinned_obj_container = obj_manager->Get_Object_List(Object_Type::non_skinned);

	for (std::shared_ptr<CGameObject> obj_ptr : *skinned_obj_container)
		obj_ptr->ReleaseUploadBuffers();
	
	for (std::shared_ptr<CGameObject> obj_ptr : *non_skinned_obj_container)
		obj_ptr->ReleaseUploadBuffers();

}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'Q':
			{
				m_pPlayer->SetBlurMask(test_button);

			}		break;

		case 'R':
		{
			test_button = !test_button;
		}
			break;

		case 'F':		case 'f':
		{
			// Toggle Fog On/Off
			fog_info->Fog_Trigger ^= 1;
		}
		break;

		case 'E':
		{
			particle_test_button = !particle_test_button;
			if (test_dragonfire == NULL)
				break;

			test_dragonfire->Set_Active(particle_test_button);
			auto* mon = obj_manager->Get_Object_List(Object_Type::skinned);
			if (mon)
			{
				for (const auto& obj : *mon)
				{
					if (!obj) continue;

					if (auto* dragon = dynamic_cast<CDragonObject*>(obj.get()))
					{
						if (particle_test_button) {
							dragon->Test_Mode = true;
							float centerZ = 1590.0f;
							XMFLOAT3 centerPos = XMFLOAT3(1723.0f, 35.0f, 831.0f);
							dragon->GetStateMachine()->changeState(State::Attack3, Key_Value::None);
							dragon->SetPosition(centerPos);
							dragon->SetLookDirection(XMFLOAT3(1.0f, 0.0f, 0.0f));
						}
						else {
							dragon->SetPosition(XMFLOAT3(1723.0f, 35.0f, 831.0f));
							dragon->GetStateMachine()->changeState(State::Idle, Key_Value::None);
						}
						break; 
					}
				}
			}
		}
		break;

		case 'T':
		{
			if (test_sand == NULL)
				break;

			test_sand->Update_Func_Index +=1;
			test_sand->Update_Func_Index %= 3;

			if (test_sand->Update_Func_Index == 0)
			{
				test_sand->SetPosition(1200.0f, 1000.0f, 1200.0f);
				test_sand->Set_Area(XMFLOAT3(2400.0f, 2000.0f, 2400.0f));

				test_sand->Set_Speed(0.0f);
				test_sand->Set_Main_Direction(XMFLOAT3(0.0f, 0.0f, -1.0f));
			}
			else if (test_sand->Update_Func_Index == 1 || test_sand->Update_Func_Index == 2)
			{
				auto* mon = obj_manager->Get_Object_List(Object_Type::skinned);
				if (mon)
				{
					for (const auto& obj : *mon)
					{
						if (!obj) continue;

						if (auto* anu = dynamic_cast<CAnubisObject*>(obj.get()))
						{
							XMFLOAT3 anubisPos = anu->GetPosition();

							// focus_point만 설정 (1번 공통 처리)
							test_sand->SetPosition(XMFLOAT3(1200.0f, 1000.0f, 1200.0f));
							test_sand->Set_Focus_Point(anubisPos);
							test_sand->Set_Speed(0.0f);

							// 2번 전용 처리
							if (test_sand->Update_Func_Index == 2)
							{
								anu->GetStateMachine()->changeState(State::Attack3, Key_Value::None);
								XMFLOAT3 pos = anubisPos;
								XMFLOAT3 dir = anu->GetLook();

								test_sand->Set_Main_Direction(XMFLOAT3(0.0f, 1.0f, 0.0f));
								test_sand->SetPosition(pos);
								test_sand->Set_Focus_Point(anubisPos);

								test_sand->Set_Speed(100.0f);
								test_sand->Set_Direction(dir);
							}

							break; 
						}
					}
				}
			}

		}
		break;

		case 'Z':
		{
			m_pPlayer->GetStateMachine()->changeState(State::Knock_Down, Key_Value::None);
			//m_pPlayer->GetStateMachine()->changeState(State::Get_Hit_F2, Key_Value::None);
			m_pPlayer->SetStateElapsedTime(0.0f);
		}		break;
		case 'X':
		{
			m_pPlayer->GetStateMachine()->changeState(State::Get_Up, Key_Value::None);
			m_pPlayer->SetStateElapsedTime(0.0f);
		}		break;
		case 'C':
		{
			obj_manager->Clear_Object_List(Object_Type::skinned);

		}		break;
		case 'P':
		{
			bOBBRender = !bOBBRender;

		}		break;

		case 'V':
		{
			/*auto it = obj_manager->Get_Object_List(Object_Type::skinned);
			if (it && it->size() > 5) {
				auto multiPlayerObj = std::dynamic_pointer_cast<CMultiPlayerObject>((*it)[5]);
				if (multiPlayerObj) {
					multiPlayerObj->GetStateMachine()->changeState(State::Get_Up, Key_Value::None);
					multiPlayerObj->SetStateElapsedTime(0.0f);
				}
			}

			if (it && it->size() > 6) {
				auto multiPlayerObj = std::dynamic_pointer_cast<CTerrainPlayer>((*it)[6]);
				if (multiPlayerObj) {
					multiPlayerObj->GetStateMachine()->changeState(State::Get_Up, Key_Value::None);
					multiPlayerObj->SetStateElapsedTime(0.0f);
				}
			}*/

		}		break;
		case 'B': 
		{
			//auto it = obj_manager->Get_Object_List(Object_Type::skinned);
			//if (it && it->size() > 6) {
			//	auto multiPlayerObj = std::dynamic_pointer_cast<CTerrainPlayer>((*it)[6]);
			//	if (multiPlayerObj) {
			//		//multiPlayerObj->SetCamera(m_pPlayer->GetCamera());
			//		//m_pPlayer->DelCamera();
			//	/*	std::shared_ptr<CPlayer> tempShared = multiPlayerObj;  
			//		multiPlayerObj = std::dynamic_pointer_cast<CTerrainPlayer>(std::shared_ptr<CPlayer>(m_pPlayer));
			//		m_pPlayer = tempShared.get();*/

			//		std::weak_ptr<CTerrainPlayer> weakMultiPlayer = multiPlayerObj;  // weak_ptr로 참조 유지
			//		std::shared_ptr<CPlayer> tempShared = multiPlayerObj;  // 기존 shared_ptr을 유지
			//		multiPlayerObj = std::dynamic_pointer_cast<CTerrainPlayer>(tempShared); // 안전한 캐스팅
			//		m_pPlayer = tempShared.get(); // raw pointer 할당

			//		if (auto locked = weakMultiPlayer.lock()) {
			//			multiPlayerObj = locked;  // 원본 shared_ptr을 다시 참조
			//		}
			//	}
			//}
		}		break;
		case '+':
		{
			/*CLoadedModelInfo* pGargoyleModel6 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/First_Mate_v12.bin", NULL);
			string obj_name_7 = "test_palyer7";


			std::string_view name_view = obj_name_7;

			std::shared_ptr<CMultiPlayerObject> humanObject_7 = std::make_shared<CMultiPlayerObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGargoyleModel6, 12);
			humanObject_7->SetPosition(20.0f, m_pTerrain->Get_Mesh_Height(20.0f, 10.0f), 10.0f);
			humanObject_7->Set_Name(obj_name_7);
			humanObject_7->test_num = 7;
			obj_manager->Add_Object(humanObject_7, Object_Type::skinned);*/

		}		break;

		default:
			break;
		}
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	bool bKeyProcessed = false;

	// W, A, S, D 
	if (pKeysBuffer[0x57] & 0xF0) // W 
	{
		//DebugOutput("W key is pressed\n");
		bKeyProcessed = true;
	}

	if (pKeysBuffer[0x41] & 0xF0) // A 
	{
		//DebugOutput("A key is pressed\n");
		bKeyProcessed = true;
	}

	if (pKeysBuffer[0x53] & 0xF0) // S 
	{
		//DebugOutput("S key is pressed\n");
		bKeyProcessed = true;
	}

	if (pKeysBuffer[0x44] & 0xF0) // D 
	{
		//DebugOutput("D key is pressed\n");
		bKeyProcessed = true;
	}



	return false; 

}

void CScene::Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	fog_info->time += fTimeElapsed;

	obj_manager->Animate_Objects_All(fTimeElapsed);

	if (Shader_list.size())
		for (std::shared_ptr<CShader> shader_ptr : Shader_list)
			shader_ptr->AnimateObjects(fTimeElapsed);

	auto list = obj_manager->Get_Object_List(Object_Type::skinned);
	if (list) {
		for (const std::shared_ptr<CGameObject>& obj_ptr : *list) {
			if (!obj_ptr || !obj_ptr->Get_Active()) continue;
			if (auto monster_ptr = std::dynamic_pointer_cast<CMonsterObject>(obj_ptr)) {
				monster_ptr->GetStateMachine()->SetTargetPos(m_pPlayer->GetPosition());
			}
		}
	}
	// Dragon
	if (particle_test_button)
	{
		auto list = obj_manager->Get_Object_List(Object_Type::skinned);
		XMFLOAT3 test_pos{};
		if (list) {
			for (auto& obj : *list) {
				const char* objName = obj->Get_Name();
				CMonsterObject* monster = dynamic_cast<CMonsterObject*>(obj.get());
				if (strcmp(objName, "Dragon") == 0 && monster->GetStateMachine()->Get_State() == State::Attack3) {
					shared_ptr<CGameObject> weapon = obj->FindFrame(obj->WeaponName);

					if (weapon) {
						XMMATRIX worldMatrix = XMLoadFloat4x4(&weapon->WeaponMatrix);

						XMVECTOR scale, rotQuat, trans;
						XMMatrixDecompose(&scale, &rotQuat, &trans, worldMatrix);

						XMVECTOR forward = XMVector3Normalize(worldMatrix.r[2]);

						float forwardOffset = 10.0f;
						float heightOffset = -5.0f;

						XMVECTOR offsetVec = forward * forwardOffset + XMVectorSet(0, heightOffset, 0, 0);

						XMVECTOR finalPos = trans + offsetVec;

						XMFLOAT3 position;
						XMStoreFloat3(&position, finalPos);
						test_dragonfire->SetPosition(position);

						XMFLOAT3 look;
						XMStoreFloat3(&look, forward);
						test_dragonfire->Set_Main_Direction(look);
					}
				}
			}
		}
	}

	if (test_bleeding)
	{

	}
#ifdef RENDER_WAVE

	CS_Wave_Shader::update_wave_info->g_WaveMin = 0.15f;
	CS_Wave_Shader::update_wave_info->g_WaveMax = 0.75f;
	CS_Wave_Shader::update_wave_info->g_HeightDamping = 0.1f;

	shared_ptr<Wave_Object> wave_obj = obj_manager->Get_Wave_Object();
	if (wave_obj)
	{
		Plane_Shader::Update(fTimeElapsed);
		wave_obj->Animate(pd3dCommandList, fTimeElapsed);
	}
#endif


}

void CScene::Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
#ifdef RENDER_WAVE
	shared_ptr<Wave_Object> wave_obj = obj_manager->Get_Wave_Object();
	if (wave_obj)
		wave_obj->Copy_Buffer_Data(pd3dCommandList);
#endif

#ifdef USING_OBB
	obj_manager->Update_OBB_Data(pd3dDevice, pd3dCommandList, Object_Type::etc);	// Update every frame
	obj_manager->Check_OBB_Collision();
	obj_manager->Check_OBB_Culling(pd3dDevice, pd3dCommandList, main_Camera.get());
#endif

	obj_manager->Update(pd3dDevice, pd3dCommandList);
	Light_Material_Manager::Update(pd3dDevice, pd3dCommandList);


	if (m_pPlayer->GetTrailOn())
	{
		if (!m_pPlayer->GetTrailStart()) 
		{
			shared_ptr<CGameObject> trail_target = m_pPlayer->FindFrame("SM_Wep_Cutlass_01");
			std::shared_ptr<Trail_Object> trail_obj = std::make_shared<Trail_Object>(pd3dDevice, pd3dCommandList);
			trail_obj->Set_Trail_Target(trail_target, false);
			trail_obj->Set_Trail_LocalOffset(XMFLOAT3(0.0f, 9.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f));
			obj_manager->Add_Object(trail_obj, Object_Type::trail);
			m_pPlayer->SetTrailObj(trail_obj);
			m_pPlayer->GetTrailObj()->Set_Active(true);
			m_pPlayer->Trail_Start();
		}

		if (!m_pPlayer->GetTrailObj()->Get_Active()) 
		{
			m_pPlayer->GetTrailObj()->GetTrailMesh()->ResetTrail();
			m_pPlayer->GetTrailObj()->Set_Active(true);
		}
	}



	if (test_sand && test_sand->Update_Func_Index == 1)
	{
		auto* mon = obj_manager->Get_Object_List(Object_Type::skinned);
		if (mon)
		{
			for (const auto& obj : *mon)
			{
				if (!obj) continue;

				if (auto* anu = dynamic_cast<CAnubisObject*>(obj.get()))
				{
					test_sand->Set_Focus_Point(anu->GetPosition());
				}
			}
		}
	}

	if (test_button)
	{
		test_button = false;

		int  cbv = CDescriptor_Heap::GetCreatedCbvCount();
		int  srv = CDescriptor_Heap::GetCreatedSrvCount();
		int  uav = CDescriptor_Heap::GetCreatedUavCount();

		DebugOutput("CBV: " + to_string(cbv) + "\n");
		DebugOutput("SRV: " + to_string(srv) + "\n");
		DebugOutput("UAV: " + to_string(uav) + "\n");

		test_bleeding = particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, "bleeding");
		test_bleeding->Set_World_Coordinate();

		XMFLOAT3 p_pos = m_pPlayer->GetPosition();
		p_pos.y += 15.0f;
		test_bleeding->SetPosition(p_pos);
		test_bleeding->Set_Main_Direction(XMFLOAT3(0.0f, 0.0f, 1.0f));

	}
}

void CScene::After_Update_Objects()
{

#ifdef RENDER_WAVE
	shared_ptr<Wave_Object> wave_obj = obj_manager->Get_Wave_Object();
	if (wave_obj)
		wave_obj->Readback_Buffer_Data();

#endif

}

void CScene::Prepare_Shadow_Map_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (fixed_shadow_camera)
	{
		if (fixed_shadow_camera->shadow_active == false)
			return;
		if (m_MRT_GraphicsRootSignature)
			pd3dCommandList->SetGraphicsRootSignature(m_MRT_GraphicsRootSignature.get());

		//fixed_shadow_camera->shadow_map_shader->OnPrepareRender(pd3dCommandList);

		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = fixed_shadow_camera->Get_Shadow_Map_DSV();

		pd3dCommandList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);

		pd3dCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		fixed_shadow_camera->Update_Render_ShaderVariables(pd3dCommandList);
	}
}

void CScene::Shadow_Map_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (fixed_shadow_camera)
	{
		if (fixed_shadow_camera->shadow_active == false)
			return;

		obj_manager->Render_Objects_Shadow_All(pd3dCommandList, fixed_shadow_camera.get());
	}
}

void CScene::Prepare_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_MRT_GraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_MRT_GraphicsRootSignature.get());

	main_Camera.get()->Update_Render_ShaderVariables(pd3dCommandList);
	main_Camera.get()->Update_Last_Frame_Info(pd3dCommandList);

	//씬의 객체들 프러스텀 컬링
	//obj_manager->Check_Culling_All(pCamera);

	// Light Update
	UpdateShaderVariables(pd3dCommandList);

}

void CScene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	obj_manager->Render_Objects_All(pd3dCommandList, main_Camera.get());
}

void CScene::Prepare_Transparent_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_Transparent_GraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_Transparent_GraphicsRootSignature.get());

	main_Camera.get()->Update_Render_ShaderVariables(pd3dCommandList);


}

void CScene::Transparent_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	obj_manager->Render_Transparent_Objects_All(pd3dCommandList, main_Camera.get());

#ifdef RENDER_PARTICLE
	if (particle_manager)
	{
		particle_manager->Render_All(pd3dCommandList, main_Camera.get());
	}
#endif

#ifdef USING_OBB
	if (bOBBRender)
		obj_manager->Render_OBB(pd3dCommandList, main_Camera.get());
#endif

#ifdef RENDER_WAVE
	pd3dCommandList->SetGraphicsRootSignature(m_Plane_GraphicsRootSignature.get());
	obj_manager->Render_Wave(pd3dCommandList, main_Camera.get());
#endif
	// For UI
	//if (Shader_list.size())
	//	for (std::shared_ptr<CShader> shader_ptr : Shader_list)
	//		shader_ptr->Render_Objects(pd3dCommandList, pCamera);

}


void CScene::Post_Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	obj_manager->Post_Update_All();
}

//==========================================================================================

void Test_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);


#ifdef RENDER_PARTICLE
	particle_manager = new Particle_Manager();
	particle_manager->Create_Particle_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif


	obj_manager = new Object_Manager();

	//=====================================================

	Object_Manager::Reserve_Update();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void Test_Scene::Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	fog_info->time += fTimeElapsed;

	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Position.y += 10.0f;
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}

}

void Test_Scene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	obj_manager->Render_Objects_All(pd3dCommandList, main_Camera.get());
}

//==========================================================================================

void Character_Select_Scene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 4;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 50.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.9f, 0.4f, 0.1f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.9f, 0.4f, 0.1f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.2f, 0.1f, 0.05f, 0.1f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(0.0f, 2.0f, 15.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.1f, 0.01f);

	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = POINT_LIGHT;
	m_pLights[1].m_fRange = 10.0f; 
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(1.0f, 0.5f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(1.0f, 0.5f, 0.1f, 1.0f); 
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.2f, 0.1f, 0.05f, 0.1f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(0.0f, 2.0f, 15.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.2f, 0.05f);

	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = POINT_LIGHT;
	m_pLights[2].m_fRange = 5.0f;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.1f);
	m_pLights[2].m_xmf3Position = XMFLOAT3(0.0f, 2.0f, 15.0f);
	m_pLights[2].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.2f, 0.05f);

	m_pLights[3].m_bEnable = false;
	m_pLights[3].m_nType = POINT_LIGHT;
	m_pLights[3].m_fRange = 10.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.1f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(0.0f, 2.0f, 15.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.2f, 0.05f);


}

void Character_Select_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);

	m_pLights[0].m_bEnable = true;
	m_pLights[1].m_bEnable = true;
	m_pLights[2].m_bEnable = true;
	m_pLights[3].m_bEnable = false;


#ifdef RENDER_PARTICLE
	particle_manager = new Particle_Manager();
	particle_manager->Create_Particle_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif


#ifdef USING_OBB
	obj_manager->Create_OBB_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif

	//=====================================================

	float centerX = 5.0f;
	float centerZ = 10.0f;
	float radiusX = 30.0f;
	float radiusZ = 15.0f; 
	float minY = -3.0f;
	float maxY = 0.0f;

	float totalRotationRad = XMConvertToRadians(110.0f);

	for (int i = 0; i < 6; ++i) {
		float angle = XM_PI * ((float)i / 5.0f);
		float localX = radiusX * cosf(angle);
		float localZ = radiusZ * sinf(angle);

		float rotatedX = centerX + (localX * cosf(totalRotationRad) - localZ * sinf(totalRotationRad));
		float rotatedZ = centerZ + (localX * sinf(totalRotationRad) + localZ * cosf(totalRotationRad));

		float t = (float)i / 5.0f;
		float y = (maxY - minY) * sinf(t * XM_PI) + minY;

		std::shared_ptr<CTerrainPlayer> player = std::make_shared<CTerrainPlayer>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, (void*)NULL, i);
		player->Set_Child(player->m_pRootModel);
		player->SetupWeaponCollider();

		player->SetPosition(XMFLOAT3(rotatedX, y, rotatedZ));
		player->Object_type = OBJECT_TPYE_SELECT_PLAYER;
		player->GetStateMachine()->changeState(State::Select_Idle, Key_Value::None);
		obj_manager->Add_Object(player, Object_Type::player);
	}

	CLoadedModelInfo* Test_Island_Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, "Model/Island_0.bin", NULL);
	std::shared_ptr<CGameObject> test_Island = CGameObject::Make_Instance(Test_Island_Model->m_pModelRootObject, true);
	test_Island->Set_Name("Island");
	test_Island->SetScale({ 2.0f, 3.0f ,2.0f }, true);
	test_Island->SetPosition(0.0f, 0.0f, 0.0f);

	obj_manager->Add_Object(test_Island, Object_Type::fixed);


	//=====================================================
	Object_Manager::Reserve_Update();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void Character_Select_Scene::Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	CScene::Animate_Objects(pd3dCommandList, fTimeElapsed);

	if (!m_pPlayer) 
		return;

	XMFLOAT3 targetPos = m_pPlayer->GetPosition();
	targetPos.y = 0.0f;

	auto playerList = obj_manager->Get_Object_List(Object_Type::player);
	for (const auto& obj : *playerList)
	{
		XMFLOAT3 objPos = obj->GetPosition();
		objPos.y = 0.0f;

		XMVECTOR dir = XMVectorSubtract(XMLoadFloat3(&targetPos), XMLoadFloat3(&objPos));
		dir = XMVector3Normalize(dir);

		XMFLOAT3 lookDir;
		XMStoreFloat3(&lookDir, dir);

		obj->SetLookDirection(lookDir);
	}

	static float m_fAccumulatedTime = 0.0f;
	m_fAccumulatedTime += fTimeElapsed;
	m_fAccumulatedTime = fmodf(m_fAccumulatedTime, 10.0f);

	float flicker = 0.6f + 0.4f * sinf(m_fAccumulatedTime * 1.5f);
	float fillFactor = 0.8f + 0.2f * flicker;

	XMFLOAT3 baseColor = { 0.9f, 0.4f, 0.1f };
	XMFLOAT3 targetColor = { 1.0f, 0.4f, 0.1f };

	float r = baseColor.x + (targetColor.x - baseColor.x) * (flicker - 0.6f) / 0.4f;
	float g = baseColor.y + (targetColor.y - baseColor.y) * (flicker - 0.6f) / 0.4f;
	float b = baseColor.z + (targetColor.z - baseColor.z) * (flicker - 0.6f) / 0.4f;
	float intensity = 1.0f + 1.5f * (flicker - 0.6f);

	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(r * intensity, g * intensity, b * intensity, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f * flicker, 0.2f * flicker, 0.1f * flicker, 0.2f);
	m_pLights[1].m_fRange = 10.0f + 25.0f * flicker;

	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.05f * fillFactor, 0.03f * fillFactor, 0.02f * fillFactor, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.02f * fillFactor, 0.01f * fillFactor, 0.005f * fillFactor, 0.05f);
	m_pLights[0].m_fRange = 50.0f + 20.0f * (fillFactor - 0.8f);
}

void Character_Select_Scene::Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	obj_manager->Update(pd3dDevice, pd3dCommandList);
	Light_Material_Manager::Update(pd3dDevice, pd3dCommandList);
}


void Character_Select_Scene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::Render(pd3dDevice, pd3dCommandList);
	//obj_manager->Render_Objects_All(pd3dCommandList, pCamera);
}

void Character_Select_Scene::UpdatePlayerSelection(int new_index)
{
	auto player_list = obj_manager->Get_Object_List(Object_Type::player);
	int list_size = static_cast<int>(player_list->size());
	if (list_size == 0)
		return;

	int next_index = (new_index + list_size) % list_size;

	if (prev_index >= 0 && prev_index < list_size && prev_index != next_index)
		(*player_list)[prev_index]->SetOutlineColor(0);

	(*player_list)[next_index]->SetOutlineColor(1);

	prev_index = next_index;
	select_index = next_index;

	if (select_index != -1)
	{
		XMFLOAT3 character_pos = (*player_list)[next_index]->GetPosition();
		m_pLights[3].m_bEnable = true;
		m_pLights[3].m_xmf3Position = character_pos;
		m_pLights[3].m_xmf3Position.y += 15.0f;
	}
}

bool Character_Select_Scene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'Z':
			UpdatePlayerSelection(select_index - 1);
			break;

		case 'C':
			UpdatePlayerSelection(select_index + 1);
			break;

		case 'F':		case 'f':
		{
			// Toggle Fog On/Off
			fog_info->Fog_Trigger ^= 1;
		}
		break;

		default:
			break;
		}
	}
	return(false);
}

//==========================================================================================

void Board_Scene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 9;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	XMFLOAT4 rainbowColors[7] = {
		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),  // Red
		XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),  // Orange
		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),  // Yellow
		XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),  // Green
		XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),  // Blue
		XMFLOAT4(0.29f, 0.0f, 0.51f, 1.0f),// Indigo
		XMFLOAT4(0.58f, 0.0f, 0.83f, 1.0f) // Violet
	};

	for (int i = 0; i < 7; ++i)
	{
		m_pLights[i].m_bEnable = true;
		m_pLights[i].m_nType = POINT_LIGHT;
		m_pLights[i].m_fRange = 300.0f;
		m_pLights[i].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // white ambient
		m_pLights[i].m_xmf4Diffuse = rainbowColors[i];                 // rainbow color diffuse
		m_pLights[i].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // white specular
		m_pLights[i].m_xmf3Position = XMFLOAT3(100.0f + 100.0f * i, 50.0f, 100.0f); // position along x-axis
		m_pLights[i].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f); // attenuation factors
	}

	m_pLights[7].m_bEnable = true;
	m_pLights[7].m_nType = POINT_LIGHT;
	m_pLights[7].m_fRange = 200.0f;
	m_pLights[7].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights[7].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[7].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.3f);
	m_pLights[7].m_xmf3Position = XMFLOAT3(250.0f, 50.0f, 250.0f);
	m_pLights[7].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights[8].m_bEnable = true;
	m_pLights[8].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[8].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights[8].m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights[8].m_xmf4Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[8].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
}

void Board_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);

	m_pLights[8].m_bEnable = true;



#ifdef RENDER_PARTICLE
	particle_manager = new Particle_Manager();
	particle_manager->Create_Particle_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif


#ifdef USING_OBB
	obj_manager->Create_OBB_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif


#ifdef RENDER_WAVE
	std::shared_ptr<Wave_Object> wave_obj = std::make_shared<Wave_Object>(pd3dDevice, pd3dCommandList, m_Plane_GraphicsRootSignature, 3000, 100, true);
	wave_obj->Set_Name("board_scene_wave");
	wave_obj->SetPosition(XMFLOAT3(0.0f, 10.0f, 0.0f));

	wave_obj->Set_BaseTexture(pd3dDevice, pd3dCommandList, L"Terrain/Water_Detail_Texture_0.dds");
	wave_obj->Set_DetailTexture(pd3dDevice, pd3dCommandList, L"Terrain/Water_Detail_Texture_0.dds");
	obj_manager->Set_Wave_Object(wave_obj);
#endif

	//=====================================================
	{
		CLoadedModelInfo* Test_Island_Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, "Model/Island_0.bin", NULL);
		std::shared_ptr<CGameObject> test_Island_0 = CGameObject::Make_Instance(Test_Island_Model->m_pModelRootObject, true);
		test_Island_0->Set_Name("Island_0");
		test_Island_0->SetScale({ 2.0f, 3.0f ,2.0f }, true);

		std::shared_ptr<CGameObject> test_Island_1 = CGameObject::Make_Instance(test_Island_0, true);
		test_Island_1->Set_Name("Island_1");

		std::shared_ptr<CGameObject> test_Island_2 = CGameObject::Make_Instance(test_Island_1, true);
		test_Island_1->Set_Name("Island_2");

		std::shared_ptr<CGameObject> test_Island_3 = CGameObject::Make_Instance(test_Island_2, true);
		test_Island_1->Set_Name("Island_3");

		std::shared_ptr<CGameObject> test_Island_4 = CGameObject::Make_Instance(test_Island_3, true);
		test_Island_1->Set_Name("Island_4");

		std::shared_ptr<CGameObject> test_Island_5 = CGameObject::Make_Instance(test_Island_4, true);
		test_Island_1->Set_Name("Island_5");

		std::shared_ptr<CGameObject> test_Island_6 = CGameObject::Make_Instance(test_Island_5, true);
		test_Island_1->Set_Name("Island_6");


		test_Island_0->SetPosition(-1200.0f, 40.0f, -1200.0f);
		m_pLights[0].m_xmf3Position = XMFLOAT3(-1200.0f, 40.0f, -1200.0f);

		test_Island_1->SetPosition(-1200.0f, 40.0f, 0.0f);
		m_pLights[1].m_xmf3Position = XMFLOAT3(-1200.0f, 40.0f, 0.0f);

		
		test_Island_2->SetPosition(-1200.0f, 40.0f, 1200.0f);
		m_pLights[2].m_xmf3Position = XMFLOAT3(-1200.0f, 40.0f, 1200.0f);

		test_Island_3->SetPosition(0.0f, 40.0f, 1200.0f);
		m_pLights[3].m_xmf3Position = XMFLOAT3(0.0f, 40.0f, 1200.0f);


		test_Island_4->SetPosition(1200.0f, 40.0f, 1200.0f);
		m_pLights[4].m_xmf3Position = XMFLOAT3(1200.0f, 40.0f, 1200.0f);


		test_Island_5->SetPosition(1200.0f, 40.0f, 0.0f);
		m_pLights[5].m_xmf3Position = XMFLOAT3(1200.0f, 40.0f, 0.0f);

		test_Island_6->SetPosition(1200.0f, 40.0f, -1200.0f);
		m_pLights[6].m_xmf3Position = XMFLOAT3(1200.0f, 40.0f, -1200.0f);


		obj_manager->Add_Object(test_Island_0, Object_Type::fixed);
		obj_manager->Add_Object(test_Island_1, Object_Type::fixed);
		obj_manager->Add_Object(test_Island_2, Object_Type::fixed);
		obj_manager->Add_Object(test_Island_3, Object_Type::fixed);
		obj_manager->Add_Object(test_Island_4, Object_Type::fixed);
		obj_manager->Add_Object(test_Island_5, Object_Type::fixed);
		obj_manager->Add_Object(test_Island_6, Object_Type::fixed);

		//if (Test_Island_Model)
		//	delete Test_Island_Model;
	}
	//=====================================================
	{
		CLoadedModelInfo* Ship_Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, "Model/Pirate_Ship_Model.bin", NULL);

		pirate_ship = std::make_shared<Boat_Object>();
		pirate_ship->Set_Child(Ship_Model->m_pModelRootObject);


		pirate_ship->Set_Name("player's pirate_ship");
		pirate_ship->SetPosition(0.0f, 0.0f, 0.0f);


		pirate_ship->SetScale({ 3.0f, 3.0f, 3.0f }, true);
		obj_manager->Add_Object(pirate_ship, Object_Type::non_skinned);

		pirate_ship->RegisterMarker("Captain", pirate_ship->FindFrame("Captain_pos"));
		pirate_ship->RegisterMarker("Sailor_0", pirate_ship->FindFrame("Sailor_Pos_0"));
		pirate_ship->RegisterMarker("Sailor_1", pirate_ship->FindFrame("Sailor_Pos_1"));
		pirate_ship->RegisterMarker("Sailor_2", pirate_ship->FindFrame("Sailor_Pos_2"));
		pirate_ship->RegisterMarker("Sailor_3", pirate_ship->FindFrame("Sailor_Pos_3"));
		pirate_ship->RegisterMarker("Sailor_4", pirate_ship->FindFrame("Sailor_Pos_4"));

		pirate_ship->RegisterMarker("Head", pirate_ship->FindFrame("Bottom_Head"));
		pirate_ship->RegisterMarker("Tail", pirate_ship->FindFrame("Bottom_Tail"));

		pirate_ship->RegisterMarker("Move_Model_1", pirate_ship->FindFrame("SM_Veh_Boat_Warship_01_Mast_01"));
		pirate_ship->RegisterMarker("Move_Model_2", pirate_ship->FindFrame("SM_Veh_Boat_Warship_01_Mast_02"));
		pirate_ship->RegisterMarker("Move_Model_3", pirate_ship->FindFrame("SM_Veh_Boat_Warship_01_Mast_03"));
		pirate_ship->RegisterMarker("Move_Model_4", pirate_ship->FindFrame("SM_Veh_Boat_Warship_01_Sails_04"));
		pirate_ship->RegisterMarker("Move_Model_5", pirate_ship->FindFrame("SM_Veh_Boat_Warship_01_Sails_05"));

		pirate_ship->RegisterMarker("Stay_Model_1", pirate_ship->FindFrame("SM_Veh_Boat_Warship_01_Mast_SailUp_01"));
		pirate_ship->RegisterMarker("Stay_Model_2", pirate_ship->FindFrame("SM_Veh_Boat_Warship_01_Mast_SailUp_02"));
		pirate_ship->RegisterMarker("Stay_Model_3", pirate_ship->FindFrame("SM_Veh_Boat_Warship_01_Mast_SailUp_03"));
		pirate_ship->RegisterMarker("Stay_Model_4", pirate_ship->FindFrame("SM_Veh_Boat_Warship_01_Mast_SailUp_04"));

		pirate_ship->RegisterMarker("Captain_Wheel", pirate_ship->FindFrame("SM_Prop_ShipWheel_02"));

		pirate_ship->Set_Sail_Mode(false);
		pirate_ship->Change_Model(true);


		if (Ship_Model)
			delete Ship_Model;

	}

	//=====================================================

#ifdef RENDER_PARTICLE
	Particle_Shape_Mesh* cube_shape_mesh = new Cube_Shape_Mesh(pd3dDevice, pd3dCommandList, 2.0f);
	Particle_Format water_splashes_info;
	{
		water_splashes_info.shader_type = Particle_Type::loop;
		water_splashes_info.particle_type = 2;
		water_splashes_info.max_particles = 300;

		water_splashes_info.area_xyz = XMFLOAT3(1000.0f, 100.0f, 1000.0f);

		water_splashes_info.MaxLifetime = 0.3f;

		water_splashes_info.main_direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
		water_splashes_info.init_velocity_value = 100.0f;
		water_splashes_info.acceleration = XMFLOAT3(0.0f, 10.0f, 0.0f);

		water_splashes_info.size = 1.0f;
		water_splashes_info.color = XMFLOAT3(0.0f, 0.0f, 1.0f);
	}

	shared_ptr<Particle_Shape_Mesh> particle_mesh;
	particle_mesh = particle_manager->Get_Particle_Mesh("cube");

	water_particle_1 = particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, particle_mesh, water_splashes_info);
	water_particle_2 = particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, particle_mesh, water_splashes_info);
#endif
	//=====================================================


	Object_Manager::Reserve_Update();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void Board_Scene::Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	fog_info->time += fTimeElapsed;

#ifdef RENDER_WAVE

	CS_Wave_Shader::update_wave_info->g_WaveMin = 0.35f;
	CS_Wave_Shader::update_wave_info->g_WaveMax = 0.65f;
	CS_Wave_Shader::update_wave_info->g_HeightDamping = 0.01f;

	Deferred_Plane_Shader::Update(fTimeElapsed);

	shared_ptr<Wave_Object> wave_obj = obj_manager->Get_Wave_Object();

	if (wave_obj)
	{
		wave_obj->Synchronize_Wave_to_Boat(pirate_ship.get());
		wave_obj->Animate(pd3dCommandList, fTimeElapsed);
	}

#endif


	pirate_ship->Animate(fTimeElapsed);
	pirate_ship->HandleBoundaryReflection(1500.0f);

	if (m_pLights)
	{
		m_pLights[7].m_xmf3Position = pirate_ship->GetPosition();
		m_pLights[7].m_xmf3Position.y += 100.0f;
	}


#ifdef RENDER_PARTICLE
	XMFLOAT3 bottom_head_particle_pos;
	pirate_ship->GetMarkerWorldPosition("Head", bottom_head_particle_pos);


	water_particle_1->SetPosition(bottom_head_particle_pos);
	water_particle_1->Set_Main_Direction(Vector3::ScalarProduct(pirate_ship->GetLook(), -1.0f, false));

	XMFLOAT3 bottom_tail_particle_pos;
	pirate_ship->GetMarkerWorldPosition("Tail", bottom_tail_particle_pos);

	water_particle_2->SetPosition(bottom_tail_particle_pos);
	water_particle_2->Set_Main_Direction(Vector3::ScalarProduct(pirate_ship->GetLook(), -1.0f, false));
#endif

	if (m_pPlayer && m_pPlayer->GetCamera())
	{
		auto camera = m_pPlayer->GetCamera();

		camera->UpdateMouseHold(fTimeElapsed);
	}
}

void Board_Scene::Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
#ifdef RENDER_WAVE
	shared_ptr<Wave_Object> wave_obj = obj_manager->Get_Wave_Object();
	if (wave_obj)
		wave_obj->Copy_Buffer_Data(pd3dCommandList);
#endif

	obj_manager->Update(pd3dDevice, pd3dCommandList);
	Light_Material_Manager::Update(pd3dDevice, pd3dCommandList);

	bool isShipMoving = pirate_ship->Is_Moving(); 
	bool isSailMode = pirate_ship->Get_Sail_Mode(); 

	if (isShipMoving && !isSailMode)
	{
		pirate_ship->Set_Sail_Mode(true); 
		pirate_ship->Change_Model(false); 
	}
	else if (!isShipMoving && isSailMode)
	{
		pirate_ship->Set_Sail_Mode(false); 
		pirate_ship->Change_Model(true); 
	}


	if (focus_button)
	{
		shared_ptr<CCamera> player_camera = m_pPlayer->GetCamera();

		XMFLOAT3 new_camera_pos;
		pirate_ship->UpdateTransform(NULL);
		pirate_ship->GetMarkerWorldPosition(camera_position, new_camera_pos);

		player_camera->UpdateFocusTracking(new_camera_pos);
	}
	else
	{
		XMFLOAT3 Fixed_Position = { 0.0f, 1400.0f, 2500.0f };
		XMFLOAT3 UpVector = { 0.0f, 1.0f, 0.0f };

		//m_pPlayer->SetPosition(Fixed_Position);
		
		auto pCamera = m_pPlayer->GetCamera();
		pCamera->SetPosition(Fixed_Position);
		pCamera->GenerateViewMatrix(Fixed_Position, XMFLOAT3(0.0f, 0.0f, 0.0f), UpVector); 
		pCamera->RegenerateViewMatrix();
		}

}

void Board_Scene::After_Update_Objects()
{
	CScene::After_Update_Objects();

}

void Board_Scene::SetCameraTarget(std::string_view target)
{
	shared_ptr<CCamera> camera_ptr = m_pPlayer->GetCamera();
	if (!camera_ptr)
		return;

	if (camera_position != target)
	{
		camera_position = target;
		focus_button = true;

		XMFLOAT3 camera_pos = camera_ptr->GetPosition();
		XMFLOAT3 lookDir = camera_ptr->GetLookVector();

		XMFLOAT3 newFocus = Vector3::Add(camera_pos, Vector3::ScalarProduct(lookDir, CAMERA_FOCUS_DISTANCE, false));
		m_pPlayer->GetCamera()->EnableFocusTracking(true, newFocus);

	}
	else
	{
		camera_ptr->EnableFocusTracking(false, XMFLOAT3(0.0f, 0.0f, 0.0f));
		XMFLOAT3 lookDir = XMFLOAT3(0.0f, 0.0f, 1.0f); 
		camera_ptr->SetLookDirection(lookDir);
		camera_ptr->RegenerateViewMatrix();

		camera_position = "";
		focus_button = false;
	}
}

void Board_Scene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	obj_manager->Render_Objects_All(pd3dCommandList, main_Camera.get());

#ifdef RENDER_WAVE
	pd3dCommandList->SetGraphicsRootSignature(m_Plane_GraphicsRootSignature.get()); // Wave_RootSignature
	obj_manager->Render_Wave(pd3dCommandList, main_Camera.get());
#endif

}

bool Board_Scene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool Board_Scene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'Q':
		{
			if (test_button)
				break;

			test_button = true;
		}	break;

		case 'W':		case 'w':
		{
			pirate_ship->MoveForward(20);
		}
		break;

		case 'A':		case 'a':
		{
			pirate_ship->Add_Rotate(-10.0f);
		}
		break;

		case 'F':		case 'f':
		{
			// Toggle Fog On/Off
			fog_info->Fog_Trigger ^= 1;
		}
		break;

		case 'D':		case 'd':
		{
			pirate_ship->Add_Rotate(10.0f);
		}
		break;

		case '1':
			SetCameraTarget("Captain");
			break;

		case '2':
			SetCameraTarget("Sailor_0");
			break;

		case '3':
			SetCameraTarget("Sailor_1");
			break;

		case '4':
			SetCameraTarget("Sailor_2");
			break;

		case '5':
			SetCameraTarget("Sailor_3");
			break;

		case '6':
			SetCameraTarget("Sailor_4");
			break;

		default:
			break;
		}
	}
	return(false);
}


//==========================================================================================

void Weapon_Select_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);

}

