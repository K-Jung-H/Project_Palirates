//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"


//=============================================================================================

CScene::CScene()
{
	//if (m_pDescriptorHeap == NULL)
	//	m_pDescriptorHeap = new CDescriptorHeap();

}

CScene::~CScene()
{
	DebugOutput("\nDelete Scene");
}

ID3D12RootSignature* CScene::Create_MRT_GraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[9];
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

		pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[8].NumDescriptors = 1;
		pd3dDescriptorRanges[8].BaseShaderRegister = 8; //t8: random value for particle move
		pd3dDescriptorRanges[8].RegisterSpace = 0;
		pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		//=======================================================================
	}

	D3D12_ROOT_PARAMETER pd3dRootParameters[15];
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

		// n = 12, t6 = Terrain_Base_Texture
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 13,  t7 = Sky_Box
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 14,  t8 = RandomValue
		pd3dRootParameters[ROOT_PARAMETER_RANDOM_VALUE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_RANDOM_VALUE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_RANDOM_VALUE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
		pd3dRootParameters[ROOT_PARAMETER_RANDOM_VALUE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

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

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
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
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);
//	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
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
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	BuildDefaultLightsAndMaterials();

	m_MRT_GraphicsRootSignature = Create_MRT_GraphicsRootSignature(pd3dDevice);
	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature); 

	m_Transparent_GraphicsRootSignature = Create_Transparent_GraphicsRootSignature(pd3dDevice);


	Object_Manager::trail_shader = std::make_shared<Trail_Shader>();
	Object_Manager::trail_shader->CreateShader(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
	Object_Manager::trail_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);


#ifdef RENDER_PARTICLE
	particle_manager = new Particle_Manager();
	particle_manager->Create_Particle_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);

	Particle_Shape_Mesh* sphere_shape_mesh = new Sphere_Shape_Mesh(pd3dDevice, pd3dCommandList, 20.0f);
	Particle_Shape_Mesh* cube_shape_mesh = new Cube_Shape_Mesh(pd3dDevice, pd3dCommandList, 10.0f);
	Particle_Shape_Mesh* cube_dust_shape_mesh = new Cube_Shape_Mesh(pd3dDevice, pd3dCommandList, 2.0f);

	Particle_Format test_snow_info;
	{
		test_snow_info.shader_type = Particle_Type::spread;
		test_snow_info.particle_type = 0;
		test_snow_info.max_particles = 1000;
		test_snow_info.MaxLifetime = 3.0f;

		test_snow_info.center = XMFLOAT3(1250.0f, 100.0f, 1250.0f);
		test_snow_info.area_xyz = XMFLOAT3(1250.0f, 100.0f, 1250.0f);
		test_snow_info.EmitFaceIndex = 3;


		test_snow_info.main_direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
		test_snow_info.init_velocity_value = 0.0f;
		test_snow_info.acceleration = XMFLOAT3(0.0f, -9.8f, 0.0f);

		test_snow_info.size = 1.0f;
		test_snow_info.color = XMFLOAT3(0.5f, 0.5f, 1.0f);
	}

	Particle_Format test_dragon_fire_info;
	{
		test_dragon_fire_info.shader_type = Particle_Type::spread;
		test_dragon_fire_info.particle_type = 5;
		test_dragon_fire_info.max_particles = 3000;
		test_dragon_fire_info.MaxLifetime = 1.0f;

		test_dragon_fire_info.center = XMFLOAT3(10.0f, 10.0f, 10.0f);
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
		test_sand_storm_info.max_particles = 50000;
		test_sand_storm_info.MaxLifetime = 10.0f;

		test_sand_storm_info.center = XMFLOAT3(1250.0f, 1000.0f, 1250.0f);
		test_sand_storm_info.area_xyz = XMFLOAT3(1250.0f, 1000.0f, 1250.0f);
		test_sand_storm_info.EmitFaceIndex = 5;

		test_sand_storm_info.main_direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
		test_sand_storm_info.init_velocity_value = 100.0f;
		test_sand_storm_info.acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);

		test_sand_storm_info.size = 1.0f;
		test_sand_storm_info.color = XMFLOAT3(0.925f, 0.902f, 0.8f);
	}

	//particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, cube_shape_mesh, test_snow_info);
	test_dragonfire = particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, cube_shape_mesh, test_dragon_fire_info);
	test_sand = particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, cube_dust_shape_mesh, test_sand_storm_info);

	
#endif


	obj_manager = new Object_Manager();


#ifdef RENDER_OBB
	obj_manager->Create_OBB_Drawers(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);

#endif

//	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);


	XMFLOAT3 xmf3Scale(10.0f, 0.0f, 10.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);
	m_pTerrain = make_shared<CHeightMapTerrain>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 257, 257, xmf3Scale, xmf4Color, 8, 3);
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


		std::string_view name_view = obj_name_1;
		std::shared_ptr<CMonsterObject> AnubisObject = std::make_shared<CAnubisObject>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);
		AnubisObject->SetPosition(10.0f, m_pTerrain->Get_Mesh_Height(10.0f, 0.0f), 0.0f);
		AnubisObject->Set_Name(obj_name_1);
		AnubisObject->test_num = 1;
		obj_manager->Add_Object(AnubisObject, Object_Type::skinned);
    
		std::shared_ptr<CMonsterObject> Dragon = std::make_shared<CDragonObject>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);
		Dragon->SetPosition(120.0f, m_pTerrain->Get_Mesh_Height(120.0f, 120.0f), 120.0f);
		Dragon->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 0.0f));
		XMFLOAT3 tt2 = { 0.0f, 1.0f, 0.0f };
		Dragon->Rotate(&tt2, 180.0f);
		Dragon->test_num = 5;
		obj_manager->Add_Object(Dragon, Object_Type::skinned);


		for (int i = 0; i < 10; i++)
		{
			std::shared_ptr<CMonsterObject> m = std::make_shared<CFishManObject>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);
			m->SetPosition(10.0f * i, m_pTerrain->Get_Mesh_Height(10.0f * i, 10.0f * i), 10.0f * i);
			m->Set_Name(obj_name_3);
			m->test_num = i + 4;
			obj_manager->Add_Object(m, Object_Type::skinned);
		}
#ifdef LOAD_SCENE
	// Load Scene

	//	CLoadedModelInfo* Test_Scene_Model = CGameObject::Load_Scene_File(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, "Scene/Scene_File/TST.bin", NULL);
		CLoadedModelInfo* Test_Scene_Model = CGameObject::Load_Scene_File(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, "Scene/Scene_File_2/OBB_Test_Scene.bin", NULL);


		std::shared_ptr<CGameObject> test_scene = std::make_shared<CGameObject>();
		test_scene->Set_Name("test_scene");
		test_scene = Test_Scene_Model->m_pModelRootObject;
		test_scene->SetPosition(1300.0f, m_pTerrain->Get_Mesh_Height(1300.0f, 800.0f), 800.0f);
		test_scene->SetScale({ 10.0f, 10.0f ,10.0f }, true);
		obj_manager->Add_Object(test_scene, Object_Type::fixed);
#endif
		//=====================================================

		unordered_map<std::string, Fixed_Object_Info>* temp_list_map = obj_manager->Get_Object_List_Map(Object_Type::fixed);

		// 씬에 있는 모든 fixed 객체들을 지형에 따라 재배치하기

		for (auto& [mesh_name, instance_info] : *temp_list_map)
		{
			m_pTerrain->Reset_Obj_List_Height(instance_info.fixed_obj_list);
			m_pTerrain->Reset_Obj_List_Up_Vector(instance_info.fixed_obj_list);
		}

		// 씬에 있는 모든 fixed 객체들을 타일에 맞게 분류하기
		obj_manager->Classify_Objects_By_Tile();

		Object_Manager::Reserve_Update();

		obj_manager->Update_OBB_Drawer(Object_Type::skinned, pd3dDevice, pd3dCommandList);
		obj_manager->Update_OBB_Drawer(Object_Type::fixed, pd3dDevice, pd3dCommandList);

	}
	/*if (pGargoyleModel)
		delete pGargoyleModel;
	if (pGargoyleModel2)
		delete pGargoyleModel2;
	if (pGargoyleModel3)
		delete pGargoyleModel3;*/

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
		int tile_n = m_pTerrain->Get_Tile(xmf3Position.x, xmf3Position.z, m_pPlayer->Get_Last_Tile());
		XMFLOAT3 tile_normal = m_pTerrain->Get_Mesh_Normal(xmf3Position.x, xmf3Position.z);
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
	if (m_MRT_GraphicsRootSignature) m_MRT_GraphicsRootSignature->Release();


	obj_manager->Clear_Object_List_All();
#ifdef WRITE_TEXT_UI
	delete text_ui_manager;
#endif

	if (Shader_list.size())
		for (std::shared_ptr<CShader> shader_ptr : Shader_list)
			shader_ptr.reset();
		
		if (m_pSkyBox) delete m_pSkyBox;


	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}


void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256 * N
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
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
			test_button = !test_button;
		}	
		break;

		case 'E':
		{
			particle_test_button = !particle_test_button;
			auto* mon = obj_manager->Get_Object_List(Object_Type::skinned);
			if (mon && !mon->empty())
			{
				std::shared_ptr<CGameObject> baseObj2 = (*mon)[1];

				CGameObject* base2 = baseObj2.get();

				auto* dra = dynamic_cast<CDragonObject*>(base2);
				if (dra)
				{
					if (particle_test_button) {
						dra->GetStateMachine()->changeState(State::Attack2, Key_Value::None);
					}
					else {
						dra->GetStateMachine()->changeState(State::Idle, Key_Value::None);
					}
					//dra->MoveUp(30.0f);
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

			if (test_sand->Update_Func_Index != 2)
			{
				test_sand->Set_Main_Direction(XMFLOAT3(0.0f, 0.0f, -1.0f));
			}
			else
			{
				test_sand->Set_Main_Direction(XMFLOAT3(0.0f, 1.0f, 0.0f));

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
			m_pPlayer->GetStateMachine()->changeState(State::Select_Idle, Key_Value::None);
			//m_pPlayer->GetStateMachine()->changeState(State::Get_Up, Key_Value::None);
			m_pPlayer->SetStateElapsedTime(0.0f);
		}		break;
		case 'C':
		{
			obj_manager->Clear_Object_List(Object_Type::skinned);

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

void CScene::Animate_Objects(ID3D12GraphicsCommandList *pd3dCommandList, float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	obj_manager->Animate_Objects_All(fTimeElapsed);


	if (Shader_list.size())
		for (std::shared_ptr<CShader> shader_ptr : Shader_list)
			shader_ptr->AnimateObjects(fTimeElapsed);


	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Position.y += 10.0f;
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}
	
	auto list = obj_manager->Get_Object_List(Object_Type::skinned);
	if (list) {
		for (const std::shared_ptr<CGameObject>& obj_ptr : *list) {
			if (!obj_ptr || !obj_ptr->Get_Active()) continue;
			if (auto monster_ptr = std::dynamic_pointer_cast<CMonsterObject>(obj_ptr)) {
				monster_ptr->GetStateMachine()->SetTargetPos(m_pPlayer->GetPosition());
			}
		}
	}

	if (particle_test_button)
	{
		auto list = obj_manager->Get_Object_List(Object_Type::skinned);
		XMFLOAT3 test_pos{};
		if (list) {
			for (auto& obj : *list) {
				const char* objName = obj->Get_Name();
				CMonsterObject* monster = dynamic_cast<CMonsterObject*>(obj.get());
				if (strcmp(objName, "Dragon") == 0 && monster->GetStateMachine()->Get_State() == State::Attack2) {
					CGameObject* weapon = obj->FindFrame(obj->WeaponName);

					if (weapon) {
						XMMATRIX worldMatrix = XMLoadFloat4x4(&weapon->WeaponMatrix);

						XMVECTOR scale, rotQuat, trans;
						if (!XMMatrixDecompose(&scale, &rotQuat, &trans, worldMatrix)) {
							trans = XMVectorZero();
						}

						XMFLOAT3 position;
						XMStoreFloat3(&position, trans);
						position.y -= 5.0f;
						position.z -= 5.0f;
						test_dragonfire->Set_Center(position);

						XMVECTOR forward = XMVector3Normalize(worldMatrix.r[2]);
						XMFLOAT3 look;
						XMStoreFloat3(&look, forward);
						test_dragonfire->Set_Main_Direction(look);
					}
				}
			}
		}


	}
}

void CScene::Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
#ifdef RENDER_OBB

	// Update every frame
	obj_manager->Update_OBB_Drawer(Object_Type::skinned, pd3dDevice, pd3dCommandList);

#endif
	obj_manager->Update(pd3dDevice, pd3dCommandList);
	Light_Material_Manager::Update(pd3dDevice, pd3dCommandList);


	if (m_pPlayer->GetTrailOn())
	{
		if (!m_pPlayer->GetTrailStart()) {
			CGameObject* trail_target = m_pPlayer->FindFrame("SM_Wep_Cutlass_01");
			std::shared_ptr<Trail_Object> trail_obj = std::make_shared<Trail_Object>(pd3dDevice, pd3dCommandList);
			trail_obj->Set_Trail_Target(trail_target, false);
			trail_obj->Set_Trail_LocalOffset(XMFLOAT3(0.0f, 9.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f));
			obj_manager->Add_Object(trail_obj, Object_Type::trail);
			m_pPlayer->SetTrailObj(trail_obj);
			m_pPlayer->GetTrailObj()->Set_Active(true);
			m_pPlayer->Trail_Start();
		}

		if (!m_pPlayer->GetTrailObj()->Get_Active()) {
			m_pPlayer->GetTrailObj()->GetTrailMesh()->ResetTrail();
			m_pPlayer->GetTrailObj()->Set_Active(true);
		}
	}


#ifdef RENDER_PARTICLE
	if (particle_test_button)
	{
		XMFLOAT3 position = m_pPlayer->GetPosition();
		XMFLOAT3 look = m_pPlayer->GetLook();

		test_dragonfire->Set_Main_Direction(look);
		test_dragonfire->Set_Center(position);
	}
#endif

}

void CScene::After_Update_Objects()
{

}

void CScene::Prepare_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_MRT_GraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_MRT_GraphicsRootSignature);

	pCamera->Update_Render_ShaderVariables(pd3dCommandList);
	pCamera->Update_Last_Frame_Info(pd3dCommandList);

	//씬의 객체들 프러스텀 컬링
	//obj_manager->Check_Culling_All(pCamera);

	// Light Update
	UpdateShaderVariables(pd3dCommandList);

}

void CScene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
//	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);

	obj_manager->Render_Objects_All(pd3dCommandList, pCamera);
	

	if (Shader_list.size())
		for (std::shared_ptr<CShader> shader_ptr : Shader_list)
			shader_ptr->Render_Objects(pd3dCommandList, pCamera);
}

void CScene::Prepare_Transparent_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_Transparent_GraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_Transparent_GraphicsRootSignature);

	pCamera->Update_Render_ShaderVariables(pd3dCommandList);
}

void CScene::Transparent_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	obj_manager->Render_Transparent_Objects_All(pd3dCommandList, pCamera);

#ifdef RENDER_PARTICLE
	if (particle_manager)
	{
		particle_manager->Render_All(pd3dCommandList, pCamera);
	}
#endif

#ifdef RENDER_OBB
	obj_manager->Render_OBB_Drawers(pd3dCommandList, pCamera);
#endif

	// For UI
	//if (Shader_list.size())
	//	for (std::shared_ptr<CShader> shader_ptr : Shader_list)
	//		shader_ptr->Render_Objects(pd3dCommandList, pCamera);

}


void CScene::Post_Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	obj_manager->Post_Update_All();
}


void Test_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	BuildDefaultLightsAndMaterials();

	m_MRT_GraphicsRootSignature = Create_MRT_GraphicsRootSignature(pd3dDevice);
	m_Plane_GraphicsRootSignature = Create_Plane_GraphicsRootSignature(pd3dDevice);
	m_Transparent_GraphicsRootSignature = Create_Transparent_GraphicsRootSignature(pd3dDevice);
	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);

	//Object_Manager::trail_shader = std::make_shared<Trail_Shader>();
	//Object_Manager::trail_shader->CreateShader(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
	//Object_Manager::trail_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);


#ifdef RENDER_PARTICLE
	particle_manager = new Particle_Manager();
	particle_manager->Create_Particle_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
	particle_manager->BuildObjects(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif


	obj_manager = new Object_Manager();

	//=====================================================

	Object_Manager::Reserve_Update();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void Test_Scene::Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Position.y += 10.0f;
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}

}

void Test_Scene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	obj_manager->Render_Objects_All(pd3dCommandList, pCamera);
}

//==========================================================================================

void Character_Select_Scene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 3;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 50.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.9f, 0.4f, 0.1f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.9f, 0.4f, 0.1f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.2f, 0.1f, 0.05f, 0.1f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(-2.0f, 2.0f, -6.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.1f, 0.01f);

	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = POINT_LIGHT;
	m_pLights[1].m_fRange = 10.0f; 
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(1.0f, 0.5f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(1.0f, 0.5f, 0.1f, 1.0f); 
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.2f, 0.1f, 0.05f, 0.1f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-2.0f, 2.0f, -6.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.2f, 0.05f);

	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = POINT_LIGHT;
	m_pLights[2].m_fRange = 5.0f;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.1f);
	m_pLights[2].m_xmf3Position = XMFLOAT3(-2.0f, 2.0f, -6.0f);
	m_pLights[2].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.2f, 0.05f);
}

void Character_Select_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	BuildDefaultLightsAndMaterials();
	m_pLights[0].m_bEnable = true;
	m_pLights[1].m_bEnable = true;

	m_MRT_GraphicsRootSignature = Create_MRT_GraphicsRootSignature(pd3dDevice);
	m_Plane_GraphicsRootSignature = Create_Plane_GraphicsRootSignature(pd3dDevice);
	m_Transparent_GraphicsRootSignature = Create_Transparent_GraphicsRootSignature(pd3dDevice);
	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);

#ifdef RENDER_PARTICLE
	particle_manager = new Particle_Manager();
	particle_manager->Create_Particle_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
	particle_manager->BuildObjects(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif


	obj_manager = new Object_Manager();

#ifdef RENDER_OBB
	obj_manager->Create_OBB_Drawers(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
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

		std::shared_ptr<CTerrainPlayer> player = std::make_shared<CTerrainPlayer>(
			pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, (void*)NULL, i
		);
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
	if (!m_pPlayer) return;

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
	m_fElapsedTime = fTimeElapsed;

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

void Character_Select_Scene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::Render(pd3dDevice, pd3dCommandList, pCamera);
	//obj_manager->Render_Objects_All(pd3dCommandList, pCamera);
}

bool Character_Select_Scene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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

		default:
			break;
		}
	default:
		break;
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
	m_pLights[8].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[8].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[8].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
}

void Board_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	BuildDefaultLightsAndMaterials();

	m_pLights[8].m_bEnable = true;
	m_pLights[8].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	m_MRT_GraphicsRootSignature = Create_MRT_GraphicsRootSignature(pd3dDevice);
	m_Plane_GraphicsRootSignature = Create_Plane_GraphicsRootSignature(pd3dDevice);
	m_Transparent_GraphicsRootSignature = Create_Transparent_GraphicsRootSignature(pd3dDevice);
	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);

	//Object_Manager::trail_shader = std::make_shared<Trail_Shader>();
	//Object_Manager::trail_shader->CreateShader(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
	//Object_Manager::trail_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);


#ifdef RENDER_PARTICLE
	particle_manager = new Particle_Manager();
	particle_manager->Create_Particle_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
	particle_manager->BuildObjects(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif

	obj_manager = new Object_Manager();

#ifdef RENDER_OBB
	obj_manager->Create_OBB_Drawers(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif


	XMFLOAT3 xmf3Scale(10.0f, 0.0f, 10.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);

	wave_plane = std::make_shared<Wave_Object>(pd3dDevice, pd3dCommandList, m_Plane_GraphicsRootSignature, 3000);
	wave_plane->Set_Name("wave_1");
	wave_plane->SetPosition(XMFLOAT3(0.0f, 10.0f, 0.0f));

	wave_plane->Set_BaseTexture(pd3dDevice, pd3dCommandList, L"Terrain/Water_Detail_Texture_0.dds");
	wave_plane->Set_DetailTexture(pd3dDevice, pd3dCommandList, L"Terrain/Water_Detail_Texture_0.dds");
	obj_manager->Add_Object(wave_plane, Object_Type::plane);
	obj_manager->wave_obj = wave_plane.get();

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
		pirate_ship->Obj_Info();


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
		water_splashes_info.shader_type = Particle_Type::spread;
		water_splashes_info.particle_type = 2;
		water_splashes_info.max_particles = 300;

		water_splashes_info.center = XMFLOAT3(0.0f, 0.0f, 0.0f);
		water_splashes_info.area_xyz = XMFLOAT3(1000.0f, 100.0f, 1000.0f);

		water_splashes_info.MaxLifetime = 0.3f;

		water_splashes_info.main_direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
		water_splashes_info.init_velocity_value = 100.0f;
		water_splashes_info.acceleration = XMFLOAT3(0.0f, 10.0f, 0.0f);

		water_splashes_info.size = 1.0f;
		water_splashes_info.color = XMFLOAT3(0.0f, 0.0f, 1.0f);
	}

	water_particle_1 = particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, cube_shape_mesh, water_splashes_info);
	water_particle_2 = particle_manager->Add_Particle(pd3dDevice, pd3dCommandList, cube_shape_mesh, water_splashes_info);
#endif
	//=====================================================


	Object_Manager::Reserve_Update();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	{
		CS_Wave_Shader::update_wave_info->g_WaveSpeed = 0.5f;                            // Wave propagation speed
		CS_Wave_Shader::update_wave_info->g_HeightDamping = 0.15f;                           // Damping factor for height interpolation
		CS_Wave_Shader::update_wave_info->g_WaveMin = 0.45f;                            // Minimum wave height
		CS_Wave_Shader::update_wave_info->g_WaveMax = 0.55f;                            // Maximum wave height
		CS_Wave_Shader::update_wave_info->g_BaseSpacing = 0.01f;                           // Base spacing for wave pattern
		CS_Wave_Shader::update_wave_info->g_BaseSharpness = 0.9f;                            // Wave sharpness (peak shaping)
		CS_Wave_Shader::update_wave_info->g_BandSize = 30.0f;                         // Vertical layer height (band size)
		CS_Wave_Shader::update_wave_info->g_AngleOffsetPerBand = XMConvertToRadians(5.1f);       // Direction offset per band in radians

		// === Boat Wake Parameters ===
		CS_Wave_Shader::update_wave_info->g_WakeMaxDist = 50.0f;                          // Maximum distance the wake affects
		CS_Wave_Shader::update_wave_info->g_WakeMaxAngle = XMConvertToRadians(30.0f);      // Maximum spread angle (Kelvin-like wake)
		CS_Wave_Shader::update_wave_info->g_WakeDepthStrength = 5.0f;                            // Strength of depth indentation
		CS_Wave_Shader::update_wave_info->g_WakeDecay = 5.0f;                            // Decay factor for lateral falloff

		// === Time ===
		CS_Wave_Shader::update_wave_info->g_TotalTime = 0.0f;							// Total accumulated time (in seconds)
		CS_Wave_Shader::update_wave_info->_padding = 0.0f;                                      // Padding for 16-byte alignment
	}

}

void Board_Scene::Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
	Deferred_Plane_Shader::Update(fTimeElapsed);


	wave_plane->Synchronize_Wave_to_Boat(pirate_ship.get());
	wave_plane->Animate(pd3dCommandList, fTimeElapsed);

	pirate_ship->Animate(fTimeElapsed);


	if (m_pLights)
	{
		m_pLights[7].m_xmf3Position = pirate_ship->GetPosition();
		m_pLights[7].m_xmf3Position.y += 100.0f;
	}


#ifdef RENDER_PARTICLE
	XMFLOAT3 bottom_head_particle_pos;
	pirate_ship->GetMarkerWorldPosition("Head", bottom_head_particle_pos);

	water_particle_1->Set_Center(bottom_head_particle_pos);
	water_particle_1->Set_Main_Direction(Vector3::ScalarProduct(pirate_ship->GetLook(), -1.0f, false));

	XMFLOAT3 bottom_tail_particle_pos;
	pirate_ship->GetMarkerWorldPosition("Tail", bottom_tail_particle_pos);

	water_particle_2->Set_Center(bottom_tail_particle_pos);
	water_particle_2->Set_Main_Direction(Vector3::ScalarProduct(pirate_ship->GetLook(), -1.0f, false));
#endif

	if (m_pPlayer && m_pPlayer->GetCamera())
	{
		auto* camera = m_pPlayer->GetCamera();

		camera->UpdateMouseHold(fTimeElapsed);
	}
}

void Board_Scene::Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	wave_plane->Copy_Buffer_Data(pd3dCommandList);

	CScene::Update_Objects(pd3dDevice, pd3dCommandList);

	bool isShipMoving = pirate_ship->Is_Moving(); 
	bool isSailMode = pirate_ship->Get_Sail_Mode(); 

	if (isShipMoving && !isSailMode)
	{
		// Ship is moving but sail mode is OFF
		pirate_ship->Set_Sail_Mode(true); // Turn ON sail mode
		pirate_ship->Change_Model(false); // Switch to sailing model
	}
	else if (!isShipMoving && isSailMode)
	{
		// Ship is stopped but sail mode is ON
		pirate_ship->Set_Sail_Mode(false); // Turn OFF sail mode
		pirate_ship->Change_Model(true);   // Switch to idle model
	}



	if (focus_button)
	{
		CCamera* player_camera = m_pPlayer->GetCamera();

		XMFLOAT3 new_camera_pos;
		pirate_ship->UpdateTransform(NULL);
		pirate_ship->GetMarkerWorldPosition(camera_position, new_camera_pos);

		player_camera->UpdateFocusTracking(new_camera_pos);
	}
	else
	{
		XMFLOAT3 Fixed_Position = { -50.0f, 1400.0f, 1750.0f };
		m_pPlayer->SetPosition(Fixed_Position);
		auto pCamera = m_pPlayer->GetCamera();
		
		pCamera->SetPosition(Fixed_Position);
		pCamera->SetLookAtPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
		pCamera->GenerateViewMatrix();
	}


}

void Board_Scene::After_Update_Objects()
{
	wave_plane->Readback_Buffer_Data();

	CScene::After_Update_Objects();
}

void Board_Scene::SetCameraTarget(std::string_view target)
{
	CCamera* camera_ptr = m_pPlayer->GetCamera();
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

void Board_Scene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	obj_manager->Render_Objects_All(pd3dCommandList, pCamera);

	pd3dCommandList->SetGraphicsRootSignature(m_Plane_GraphicsRootSignature);
	obj_manager->Render_Objects(Object_Type::plane, pd3dCommandList, pCamera);
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

		case 'I':		case 'i':
		{
			pirate_ship->MoveForward(20);
		}
		break;

		case 'J':		case 'j':
		{
			pirate_ship->Add_Rotate(-10.0f);
		}
		break;

		case 'L':		case 'l':
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
	default:
		break;
	}
	return(false);
}


//==========================================================================================

void Weapon_Select_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

