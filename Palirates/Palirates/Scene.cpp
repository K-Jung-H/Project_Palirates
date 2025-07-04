#include "stdafx.h"
#include "Scene.h"

//=============================================================================================


Shadow_Camera::Shadow_Camera() : CCamera()
{
	SetViewport(0, 0, _SHADOWMAP_WIDTH, _SHADOWMAP_HEIGHT, 0.0f, 1.0f);
	SetScissorRect(0, 0, _SHADOWMAP_WIDTH, _SHADOWMAP_HEIGHT);
}

Shadow_Camera::~Shadow_Camera()
{

}

std::vector<XMFLOAT3> Shadow_Camera::CalcFrustumCornersWorld(CCamera* mainCamera, float nearZ, float farZ)
{
	std::vector<XMFLOAT3> corners(8);

	XMFLOAT3 eye = mainCamera->GetPosition();
	XMFLOAT3 lookVec = mainCamera->GetLookVector();
	XMVECTOR eyeV = XMLoadFloat3(&eye);
	XMVECTOR atV = eyeV + XMLoadFloat3(&lookVec);
	XMVECTOR upV = XMVectorSet(0, 1, 0, 0);
	XMMATRIX view = XMMatrixLookAtLH(eyeV, atV, upV);



	float fov = XMConvertToRadians(60.0f);
	float aspect = ASPECT_RATIO;
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

	//XMMATRIX view = XMLoadFloat4x4(&mainCamera->GetViewMatrix());
	//XMMATRIX proj = XMLoadFloat4x4(&mainCamera->GetProjectionMatrix());
	


	XMMATRIX invViewProj = XMMatrixInverse(nullptr, view * proj);

	float ndc[8][3] = {
		{-1, -1, 0}, { 1, -1, 0}, { 1,  1, 0}, {-1,  1, 0},
		{-1, -1, 1}, { 1, -1, 1}, { 1,  1, 1}, {-1,  1, 1}
	};


	for (int i = 0; i < 8; ++i)
	{
		XMVECTOR pt = XMVectorSet(ndc[i][0], ndc[i][1], ndc[i][2], 1.0f);
		pt = XMVector4Transform(pt, invViewProj);
		pt = XMVectorScale(pt, 1.0f / XMVectorGetW(pt));
		XMFLOAT3 outCorner;
		XMStoreFloat3(&outCorner, pt);
		corners[i] = outCorner;
	}

	return corners;
}

void Shadow_Camera::SetupCSMCascades(const XMFLOAT3& light_direction, const std::vector<float>& splitDepths, CCamera* mainCamera)
{
	m_CascadeView.clear();
	m_CascadeProj.clear();
	m_light_direction = light_direction;

	constexpr float SHADOW_Z_MARGIN = 1.0f;
	constexpr float LIGHT_DIST = 5000.0f;
	constexpr int SHADOWMAP_RESOLUTION = 2048;

	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		float nearZ = splitDepths[i];
		float farZ = splitDepths[i + 1];
		std::vector<XMFLOAT3> frustumCorners = CalcFrustumCornersWorld(mainCamera, nearZ, farZ);

		XMFLOAT3 frustumCenter = { 0.f, 0.f, 0.f };
		for (const auto& c : frustumCorners)
		{
			frustumCenter.x += c.x;
			frustumCenter.y += c.y;
			frustumCenter.z += c.z;
		}
		frustumCenter.x /= 8.0f;
		frustumCenter.y /= 8.0f;
		frustumCenter.z /= 8.0f;


		XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&light_direction));
		XMVECTOR frustumCenterV = XMLoadFloat3(&frustumCenter);
		XMVECTOR lightPos = frustumCenterV - lightDir * LIGHT_DIST;
		XMMATRIX lightView = XMMatrixLookAtLH(lightPos, frustumCenterV, XMVectorSet(0, 1, 0, 0));

		XMFLOAT3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
		XMFLOAT3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
		for (const auto& c : frustumCorners)
		{
			XMVECTOR lightSpace = XMVector3TransformCoord(XMLoadFloat3(&c), lightView);
			XMFLOAT3 p;
			XMStoreFloat3(&p, lightSpace);
			min.x = std::min(min.x, p.x);
			min.y = std::min(min.y, p.y);
			min.z = std::min(min.z, p.z);
			max.x = std::max(max.x, p.x);
			max.y = std::max(max.y, p.y);
			max.z = std::max(max.z, p.z);
		}

		min.z -= SHADOW_Z_MARGIN;
		max.z += SHADOW_Z_MARGIN;

		float width = max.x - min.x;
		float height = max.y - min.y;
		float maxSize = std::max(width, height);
		float texelSize = (2.0f * maxSize) / static_cast<float>(SHADOWMAP_RESOLUTION);

		float cx = (min.x + max.x) * 0.5f;
		float cy = (min.y + max.y) * 0.5f;

		constexpr float SNAP_UNIT = 32.0f;

		cx = floor(cx / SNAP_UNIT) * SNAP_UNIT;
		cy = floor(cy / SNAP_UNIT) * SNAP_UNIT;

		//  Snap to texel grid (world-grid 기준 정렬)
		cx = floorf(cx / texelSize) * texelSize;
		cy = floorf(cy / texelSize) * texelSize;

		XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(
			cx - maxSize * 0.5f, cx + maxSize * 0.5f,
			cy - maxSize * 0.5f, cy + maxSize * 0.5f,
			min.z, max.z
		);



		XMFLOAT4X4 viewMat, projMat;
		XMStoreFloat4x4(&viewMat, lightView);
		XMStoreFloat4x4(&projMat, lightProj);
		m_CascadeView.push_back(viewMat);
		m_CascadeProj.push_back(projMat);
		m_CascadeSplits[i] = farZ;

	}
}

std::vector<float> Shadow_Camera::GenerateCSMSplitDepths(float nearZ, float farZ, int numCascades, float lambda)
{
	std::vector<float> splits(numCascades + 1);
	splits[0] = nearZ;
	for (int i = 1; i <= numCascades; ++i)
	{
		float p = float(i) / float(numCascades);
		float logSplit = nearZ * powf(farZ / nearZ, p);
		float linearSplit = nearZ + (farZ - nearZ) * p;
		splits[i] = lambda * logSplit + (1.0f - lambda) * linearSplit;
	}
	return splits;
}

void Shadow_Camera::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CCamera::CreateShaderVariables(pd3dDevice, pd3dCommandList); 

	UINT ncbElementBytes = ((sizeof(LightCamera_Info) + 255) & ~255); //256 * N
	m_pd3dcb_LightCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcb_LightCamera->Map(0, NULL, (void**)&m_pcb_MappedLightCamera);

	//===============================================================

	shadow_map = make_shared<CMaterial>(1);
	shared_ptr<CTexture> shadowTexture = make_shared<CTexture>(NUM_CASCADES, RESOURCE_TEXTURE2D, 0, 1, 0, 0, NUM_CASCADES, 0, 0, NUM_CASCADES);

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil = { 1.0f, 0 };


	for (int i = 0; i < NUM_CASCADES; i++)
	{
		shadowTexture->CreateTexture(pd3dDevice, pd3dCommandList, i, RESOURCE_TEXTURE2D, _SHADOWMAP_WIDTH, _SHADOWMAP_HEIGHT, 1, 1, DXGI_FORMAT_R32_TYPELESS, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue);
	}

	for (int i = 0; i < NUM_CASCADES; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = CDescriptor_Heap::Get_Instance()->CreateDsv(pd3dDevice, shadowTexture.get(), i);
		shadowTexture->SetDSV(i, dsvHandle);
	}


	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, shadowTexture.get(), 0, ROOT_PARAMETER_FIXED_SHADOWMAP_TEXTURE_SRV_INDEX);

	shadow_map->SetTexture(shadowTexture, 0);


}

void Shadow_Camera::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_CascadeView.empty() || m_CascadeProj.empty())
		return;

	shadow_map->Update_TextureShaderVariables(pd3dCommandList);

	XMMATRIX texScaleBias = XMMatrixScaling(0.5f, -0.5f, 1.0f) * XMMatrixTranslation(0.5f, 0.5f, 0.0f);

	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		XMMATRIX view = XMLoadFloat4x4(&m_CascadeView[i]);
		XMMATRIX proj = XMLoadFloat4x4(&m_CascadeProj[i]);

		XMMATRIX viewProjTex = XMMatrixTranspose(view * proj * texScaleBias);

		XMFLOAT4X4 viewProjTexFloat4x4;
		XMStoreFloat4x4(&viewProjTexFloat4x4, viewProjTex);
		m_pcb_MappedLightCamera->LightViewProjTex[i] = viewProjTexFloat4x4;
		m_pcb_MappedLightCamera->cascadeSplits[i] = m_CascadeSplits[i];
	}

	m_pcb_MappedLightCamera->shadow_pass = 1;
	m_pcb_MappedLightCamera->light_type = LIGHT_CAMERA_TYPE_DIRECTIONAL;
	m_pcb_MappedLightCamera->LightDirectionWS = m_light_direction;
	m_pcb_MappedLightCamera->shadow_bias = 0.0f;
	m_pcb_MappedLightCamera->shadow_map_size = XMFLOAT2(static_cast<float>(_SHADOWMAP_WIDTH), static_cast<float>(_SHADOWMAP_HEIGHT));
	m_pcb_MappedLightCamera->inv_shadow_map_size = XMFLOAT2(1.0f / _SHADOWMAP_WIDTH, 1.0f / _SHADOWMAP_HEIGHT);

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_POST_SHADOW_INFO_CBV_INDEX, m_pd3dcb_LightCamera->GetGPUVirtualAddress());
}
void Shadow_Camera::Update_Render_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, int cascadeIdx)
{
	XMMATRIX view = XMLoadFloat4x4(&m_CascadeView[cascadeIdx]);
	XMMATRIX proj = XMLoadFloat4x4(&m_CascadeProj[cascadeIdx]);
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4Projection, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4InverseView, XMMatrixTranspose(invView));

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_CAMERA_CBV_INDEX, m_pd3dcbCamera->GetGPUVirtualAddress());
}


D3D12_CPU_DESCRIPTOR_HANDLE Shadow_Camera::Get_Shadow_Map_DSV(int n) const
{
	if (shadow_map)
	{
		shared_ptr<CTexture> shadowTex = shadow_map->m_ppTextures[0];
		if (shadowTex)
			return shadowTex->GetDSVDescriptorHandle(n);
	}

	return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
}

ID3D12Resource* Shadow_Camera::Get_Shadow_Map_Resource(int n) const
{
	if (shadow_map)
	{
		shared_ptr<CTexture> shadowTex = shadow_map->m_ppTextures[0];
		if (shadowTex)
			return shadowTex->GetResource(n);
	}

	return 0;
}

//=============================================================================================

std::shared_ptr<ID3D12RootSignature> CScene::m_MRT_GraphicsRootSignature = NULL;
std::shared_ptr<ID3D12RootSignature> CScene::m_Transparent_GraphicsRootSignature = NULL;
std::shared_ptr<ID3D12RootSignature> CScene::m_Plane_GraphicsRootSignature = NULL;
std::shared_ptr<ID3D12RootSignature> CScene::m_UI_GraphicsRootSignature = NULL;

bool CScene::bOBBRender = false;
bool CScene::Mouse_Lock = false;
bool CScene::Screen_Fade = false;
UINT CScene::select_index = 0;


CScene::CScene()
{
	scene_type = Scene_Type::etc;
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
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.Num32BitValues = 32;
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
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.Num32BitValues = 32;
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
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.Num32BitValues = 32;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.ShaderRegister = 1;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 2, b2 = Camera
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].Descriptor.ShaderRegister = 2;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 3, t0 = Base_Texture
		pd3dRootParameters[ROOT_PARAMETER_PLANE_BASE_TEXTURE_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_PLANE_BASE_TEXTURE_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_PLANE_BASE_TEXTURE_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
		pd3dRootParameters[ROOT_PARAMETER_PLANE_BASE_TEXTURE_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 4, t1 = Detail_Texture
		pd3dRootParameters[ROOT_PARAMETER_PLANE_DETAIL_TEXTURE_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_PLANE_DETAIL_TEXTURE_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_PLANE_DETAIL_TEXTURE_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
		pd3dRootParameters[ROOT_PARAMETER_PLANE_DETAIL_TEXTURE_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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

ID3D12RootSignature* CScene::Create_UI_GraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	// SRV Descriptor Table: t0 (Base Texture)
	D3D12_DESCRIPTOR_RANGE descriptorRanges[1] = {};
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRanges[0].NumDescriptors = 1;
	descriptorRanges[0].BaseShaderRegister = 0; // t0
	descriptorRanges[0].RegisterSpace = 0;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3] = {};

	pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].Descriptor.ShaderRegister = 0; //Frame_Info
	pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 12;
	pd3dRootParameters[2].Constants.ShaderRegister = 1; // b1
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Static Samplers
	D3D12_STATIC_SAMPLER_DESC samplerDescs[2] = {};

	samplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].MipLODBias = 0;
	samplerDescs[0].MaxAnisotropy = 1;
	samplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[0].MinLOD = 0;
	samplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDescs[0].ShaderRegister = 0;
	samplerDescs[0].RegisterSpace = 0;
	samplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	samplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].MipLODBias = 0;
	samplerDescs[1].MaxAnisotropy = 1;
	samplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[1].MinLOD = 0;
	samplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDescs[1].ShaderRegister = 1;
	samplerDescs[1].RegisterSpace = 0;
	samplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Root Signature Description
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	rootSignatureDesc.pParameters = pd3dRootParameters;
	rootSignatureDesc.NumStaticSamplers = _countof(samplerDescs);
	rootSignatureDesc.pStaticSamplers = samplerDescs;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return nullptr;
	}

	hr = pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dGraphicsRootSignature));

	if (FAILED(hr)) {
		OutputDebugStringA("[UI RS] Root Signature creation failed!\n");
	}

	if (signatureBlob) signatureBlob->Release();

	return pd3dGraphicsRootSignature;
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

}

void CScene::Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	BuildDefaultLightsAndMaterials();
	CS_Wave_Shader::Prepare_WaveParams();

	obj_manager = make_shared<Object_Manager>();

	auto com_deleter = [](ID3D12RootSignature* p) { if (p) p->Release(); };


	if (!m_MRT_GraphicsRootSignature)
		m_MRT_GraphicsRootSignature = std::shared_ptr<ID3D12RootSignature>(Create_MRT_GraphicsRootSignature(pd3dDevice), com_deleter);

	if (!m_Transparent_GraphicsRootSignature)
		m_Transparent_GraphicsRootSignature = std::shared_ptr<ID3D12RootSignature>(Create_Transparent_GraphicsRootSignature(pd3dDevice), com_deleter);

	if (!m_Plane_GraphicsRootSignature)
		m_Plane_GraphicsRootSignature = std::shared_ptr<ID3D12RootSignature>(Create_Plane_GraphicsRootSignature(pd3dDevice), com_deleter);

	if (!m_UI_GraphicsRootSignature)
		m_UI_GraphicsRootSignature = std::shared_ptr<ID3D12RootSignature>(Create_UI_GraphicsRootSignature(pd3dDevice), com_deleter);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);
	CSkyBox::CreateShader(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);

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


	shadow_camera = std::make_shared<Shadow_Camera>();

	if (shadow_camera)
		shadow_camera->CreateShaderVariables(pd3dDevice, pd3dCommandList);

#ifdef RENDER_PARTICLE
	particle_manager = make_shared<Particle_Manager>();
	particle_manager->Create_Particle_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif


#ifdef USING_OBB
	obj_manager->Create_OBB_Manager(pd3dDevice, pd3dCommandList, m_Transparent_GraphicsRootSignature);
#endif

}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);

	m_pSkyBox = make_shared<CSkyBox>(pd3dDevice, pd3dCommandList);
	m_pSkyBox->Set_BaseTexture(pd3dDevice, pd3dCommandList, L"SkyBox/Fluffball.dds");

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

	XMFLOAT3 xmf3Scale(10.0f, 0.0f, 10.0f); // y = 0 -> flat
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
		Dragon->SetObject_Type_ID(MATERIAL_Object_Type_ID_Monster);

		Dragon->SetupWeaponCollider();
		Dragon->SetPosition(1550.0f, m_pTerrain->Get_Mesh_Height(1550.0f, 680.0f), 680.0f);
		Dragon->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 0.0f));
		XMFLOAT3 tt2 = { 0.0f, 1.0f, 0.0f };
		Dragon->Rotate(&tt2, 180.0f);
		Dragon->test_num = 5;
		obj_manager->Add_Object(Dragon, Object_Type::skinned);


		for (int i = 0; i < 5; i++)
		{
			std::shared_ptr<CMonsterObject> m = std::make_shared<CFishManObject>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature);
			m->Set_Child(m->m_pRootModel);
			m->SetObject_Type_ID(MATERIAL_Object_Type_ID_Monster);
			m->SetupWeaponCollider();
			m->SetPosition(10.0f * i + 1450.0f, m_pTerrain->Get_Mesh_Height(10.0f * i + 1450.0f, 10.0f * i + 700.0f), 10.0f * i + 700.0f);
			m->Set_Name(obj_name_3);
			m->test_num = i + 4;
			obj_manager->Add_Object(m, Object_Type::skinned);
		}

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
		Light_Material_Manager::Update(pd3dDevice, pd3dCommandList);

#ifdef USING_OBB
		obj_manager->Update_OBB_Data(pd3dDevice, pd3dCommandList, Object_Type::etc);
		obj_manager->Update_OBB_Data(pd3dDevice, pd3dCommandList, Object_Type::fixed);
#endif
	}


#ifdef RENDER_PARTICLE
	obj_manager->Update(pd3dDevice, pd3dCommandList); // Forward Update for ParticleManager's fixed obb data
	obj_manager->Update_Fixed_OBBs();
	particle_manager->Create_OBB_Data_ShaderVariables(pd3dDevice, pd3dCommandList, obj_manager->Get_Fixed_OBBs());
#endif

	Build_Texture_UI(pd3dDevice, pd3dCommandList, m_UI_GraphicsRootSignature);

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
		State LastState = m_pPlayer->GetStateMachine()->Get_State();
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

void CScene::Build_Texture_UI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::shared_ptr<ID3D12RootSignature> pRootSignature)
{
	texture_ui_manager = new Texture_UI_Manager();
	if (!texture_ui_manager) return;

	texture_ui_manager->SetRenderer(make_unique<Texture_UI_Renderer>(pd3dDevice));
	texture_ui_manager->GetRenderer()->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	std::unique_ptr<CTextureToScreenShader> pShader = std::make_unique<CTextureToScreenShader>();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pRootSignature);
	texture_ui_manager->SetShader(std::move(pShader));
	texture_ui_manager->SetRootSignature(pRootSignature);
	std::shared_ptr<CTextureMesh> mesh = std::make_shared<CTextureMesh>(pd3dDevice, pd3dCommandList, 2.0f, 2.0f);

	CTexture* BDSCR = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	BDSCR->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/bloodscreen.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, BDSCR, 0, 0);
	D2D1_RECT_F BDSCRscreenRect = MakeNormalizedRect(0.5f, 0.5f, 1.0f, BDSCR);
	std::unique_ptr<TextureBlock> BDSCRblock = std::make_unique<TextureBlock>(BDSCR, BDSCRscreenRect, mesh, UILayer::screen);
	BDSCRblock->ui_type = UI_EFFECT_FADE_OUT;
	BDSCRblock->hp = 1.5f;
	texture_ui_manager->Add_TextureBlock(std::move(BDSCRblock));

	CTexture* HpBack = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	HpBack->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/Healthbar-Empty.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, HpBack, 0, 0);
	D2D1_RECT_F HBscreenRect = MakeNormalizedRect(0.28f, 0.9f, 0.36f, HpBack);
	std::unique_ptr<TextureBlock> HBblock = std::make_unique<TextureBlock>(HpBack, HBscreenRect, mesh);
	texture_ui_manager->Add_TextureBlock(std::move(HBblock));

	CTexture* HpFront = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	HpFront->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/Healthbar-Filled-Red.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, HpFront, 0, 0);
	D2D1_RECT_F HFscreenRect = MakeNormalizedRect(0.28f, 0.9f, 0.36f, HpFront);
	std::unique_ptr<TextureBlock> HFblock = std::make_unique<TextureBlock>(HpFront, HFscreenRect, mesh, UILayer::HP_bar);
	HFblock->ui_type = UI_EFFECT_CUT_HP;
	texture_ui_manager->Add_TextureBlock(std::move(HFblock));

	for (int i = 0; i < MONSTER_HP_UI_MAX_NUM; ++i)
	{
		D2D1_RECT_F HBscreenRect = MakeNormalizedRect(0.28f, 0.9f, 0.36f, HpBack);
		TextureBlock* HBblock = new TextureBlock(HpBack, HBscreenRect, mesh);
		HBblock->bActive = false;
		texture_ui_manager->AddMonsterHPBlock(HBblock);

		D2D1_RECT_F HFscreenRect = MakeNormalizedRect(0.28f, 0.9f, 0.36f, HpFront);
		TextureBlock* HFblock = new TextureBlock(HpFront, HFscreenRect, mesh, UILayer::HP_bar);
		HFblock->bActive = false;
		HFblock->ui_type = UI_EFFECT_CUT_HP;
		texture_ui_manager->AddMonsterHPBlock(HFblock);
	}

	auto t = texture_ui_manager->GetMonsterHPBlocks();

	D2D1_RECT_F CMscreenRect = MakeNormalizedRect(0.07f, 0.86f, 0.13f, Texture_UI_Manager::s_MugTextures[select_index].get());
	std::unique_ptr<TextureBlock> CMblock = std::make_unique<TextureBlock>(Texture_UI_Manager::s_MugTextures[select_index].get(), CMscreenRect, mesh);
	texture_ui_manager->Add_TextureBlock(std::move(CMblock));

	CTexture* replay = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	replay->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/undo-arrow.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, replay, 0, 0);
	D2D1_RECT_F REscreenRect = MakeNormalizedRect(0.9f, 0.1f, 0.05f, replay);
	std::unique_ptr<TextureBlock> REblock = std::make_unique<TextureBlock>(replay, REscreenRect, mesh, UILayer::Interactable | UILayer::Screen_Fade);
	REblock->onClick = [this]()
		{
			c_signal.change = true;
			c_signal.scene_name = "Character_Select";
			c_signal.type = Scene_Type::Lobby;

			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Screen_Fade, false);
				Screen_Fade = false;

			}
		};
	REblock->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	REblock->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	REblock->bActive = false;
	texture_ui_manager->Add_TextureBlock(std::move(REblock));


	CTexture* return_to_game = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	return_to_game->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/remove-symbol.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, return_to_game, 0, 0);
	D2D1_RECT_F RTG_screenRect = MakeNormalizedRect(0.8f, 0.1f, 0.05f, return_to_game);
	std::unique_ptr<TextureBlock> RTG_block = std::make_unique<TextureBlock>(return_to_game, RTG_screenRect, mesh, UILayer::Interactable | UILayer::Screen_Fade);
	RTG_block->onClick = [this]()
		{
			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Screen_Fade, false);
				Mouse_Lock = true;
				Screen_Fade = false;
			}
		};
	RTG_block->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	RTG_block->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	RTG_block->bActive = false;
	texture_ui_manager->Add_TextureBlock(std::move(RTG_block));

	
}

std::vector<TextureBlock*> CScene::Get_Texture_List()
{
	if (texture_ui_manager)
		return texture_ui_manager->GetTextureBlockPtrs(); 
	else
		return {};
}

void CScene::Update_Texture_UI(float currentTime, float elapsedTime)
{
	if (bUpdateUI_HP || bUpdateUI_Screen || bMenuActive) {

		std::vector<TextureBlock*>& blocks = texture_ui_manager->GetTextureBlockPtrs();

		for (auto& block : blocks)
		{
			if (block->bPendingActivation) {
				float elapsed = currentTime - block->start_time;
				if (elapsed >= block->hp)
				{
					block->bActive = true;
					block->bPendingActivation = false;
				}
			}
			if (!block) continue;

			if (bUpdateUI_HP)
			{
				uint32_t layerMask = static_cast<uint32_t>(UILayer::HP_bar);
				if ((static_cast<uint32_t>(block->layer) & layerMask) != 0)
				{
					float targetHP = m_pPlayer->currentHP / m_pPlayer->maxHP;
					float speed = 15.0f;
					block->hp = block->hp + (targetHP - block->hp) * (elapsedTime * speed);

					if (abs(block->hp - targetHP) < 0.001f) {
						block->hp = targetHP;
						bUpdateUI_HP = false;
					}
				}
			}
			if (bUpdateUI_Screen)
			{
				uint32_t layerMask = static_cast<uint32_t>(UILayer::screen);
				if ((static_cast<uint32_t>(block->layer) & layerMask) != 0)
				{
					float elapsed = currentTime - block->start_time;
					if (elapsed >= block->hp)
					{
						block->bActive = false;
						bUpdateUI_Screen = false;
					}
				}
			}
		}
	}
	Update_Monster_HP_bar(currentTime, elapsedTime, 100.0f);
}

std::shared_ptr<std::vector<MonsterUIData>> CScene::GetNearbyMonstersUIData(float maxDistance)
{
	if (!obj_manager || !m_pPlayer) return nullptr;

	const std::vector<std::shared_ptr<CGameObject>>* allObjects = obj_manager->Get_Object_List(Object_Type::skinned);
	if (!allObjects) return nullptr;

	auto filtered = std::make_shared<std::vector<MonsterUIData>>();

	XMVECTOR playerPos = XMLoadFloat3(&m_pPlayer->GetPosition());

	for (const auto& obj : *allObjects)
	{
		if (!obj || !obj->HasType(EObjectType::Monster)) continue;

		if (obj->currentHP <= 0) continue;

		XMVECTOR monsterPosVec = XMLoadFloat3(&obj->GetPosition());
		float distSq = XMVectorGetX(XMVector3LengthSq(playerPos - monsterPosVec));
		float normDist = 1.0f - (distSq / (maxDistance * maxDistance));
		normDist = std::clamp(normDist, 0.0f, 1.0f);

		if (distSq <= maxDistance * maxDistance)
		{
			MonsterUIData data;
			data.position = obj->GetPosition();
			data.hp = float(obj->currentHP) / obj->maxHP;
			data.dist = normDist;
			if (std::string(obj->Get_Name()) == "FishMan") {
				data.yOffset = 20.0f;
			}
			
			filtered->emplace_back(data);
		}
	}

	return filtered;
}

void CScene::Update_Monster_HP_bar(float currentTime, float elapsedTime, float maxDistance)
{
	auto mList = GetNearbyMonstersUIData(maxDistance);
	if (!mList) return;

	texture_ui_manager->DeactivateAllMonsterHPBlocks();
	auto& hpBlocks = texture_ui_manager->GetMonsterHPBlocks();

	size_t blockIndex = 0;

	for (const auto& monster : *mList)
	{
		if (blockIndex + 1 >= hpBlocks.size()) break;

		XMFLOAT3 headWorldPos = monster.position;
		headWorldPos.y += monster.yOffset;

		XMFLOAT2 screenPos = main_Camera->WorldToNormalizedScreen(
			headWorldPos,
			main_Camera->GetViewMatrix(),
			main_Camera->GetProjectionMatrix(),
			main_Camera->GetViewport());

		if (screenPos.x < 0.0f || screenPos.x > 1.0f || screenPos.x < 0.0f || screenPos.y > 1.0f)
			continue; 

		float scale = 0.25f + 0.75f * monster.dist;

		TextureBlock* back = hpBlocks[blockIndex++];
		back->UpdateScreenRect(screenPos.x, screenPos.y, 0.1f * scale, 1.0f);
		back->bActive = true;

		TextureBlock* front = hpBlocks[blockIndex++];
		front->UpdateScreenRect(screenPos.x, screenPos.y, 0.1f * scale, 1.0f);
		float targetHP = monster.hp;
		float speed = 15.0f;
		front->hp = front->hp + (targetHP - front->hp) * (elapsedTime * speed);

		if (abs(front->hp - targetHP) < 0.001f) {
			front->hp = targetHP;
		}
		front->bActive = true;
	}
}

void CScene::ReleaseObjects()
{
	obj_manager->Clear_Object_List_All();
#ifdef WRITE_TEXT_UI
	delete text_ui_manager;
#endif

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
	shared_ptr<CTexture> noise_texture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
	noise_texture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Test_Noise.dds", RESOURCE_TEXTURE2D, 0);

	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, noise_texture.get(), 0, ROOT_PARAMETER_FOG_NOISE_TEXTURE_SRV_INDEX);

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
	if(shadow_camera)
		shadow_camera->UpdateShaderVariables(pd3dCommandList);

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

	
	std::vector<std::shared_ptr<CGameObject>>* skinned_obj_container = obj_manager->Get_Object_List(Object_Type::skinned);
	std::vector<std::shared_ptr<CGameObject>>* non_skinned_obj_container = obj_manager->Get_Object_List(Object_Type::non_skinned);

	for (std::shared_ptr<CGameObject> obj_ptr : *skinned_obj_container)
		obj_ptr->ReleaseUploadBuffers();
	
	for (std::shared_ptr<CGameObject> obj_ptr : *non_skinned_obj_container)
		obj_ptr->ReleaseUploadBuffers();

}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (nMessageID == WM_LBUTTONDOWN)
	{
		std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
		if (blocks.empty()) return false;

		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);
		float fMouseX = static_cast<float>(mouseX);
		float fMouseY = static_cast<float>(mouseY);

		
		uint32_t mask = static_cast<uint32_t>(UILayer::Interactable);

		for (auto& block : blocks)
		{
			if (block && block->bActive) {
				if ((static_cast<uint32_t>(block->layer) & mask) != 0) {
					if (IsPointInRect(block->hitboxRect, fMouseX, fMouseY))
					{
						if (block->onClick) block->onClick();
						return true;
					}
				}
			}
		}


		std::vector<TextureBlock*> readyCheckblocks = texture_ui_manager->GetReadyCheckBlocks();
		if (readyCheckblocks.empty()) return false;

		for (auto& block : readyCheckblocks)
		{
			if (block && block->bActive) {
				if (block->ui_type != UI_EFFECT_TRANSLUCENT)
					if ((static_cast<uint32_t>(block->layer) & mask) != 0)
						if (IsPointInRect(block->hitboxRect, fMouseX, fMouseY))
						{
							if (block->onClick) block->onClick();
							return true;
						}
			}
		}
	}
	return false;
}


void CScene::UpdateUIHoverState(HWND hWnd)
{
	POINT ptMouse;
	GetCursorPos(&ptMouse);
	ScreenToClient(hWnd, &ptMouse);

	float fMouseX = static_cast<float>(ptMouse.x);
	float fMouseY = static_cast<float>(ptMouse.y);

	std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
	if (blocks.empty()) return;

	uint32_t mask = static_cast<uint32_t>(UILayer::Interactable);

	for (auto& block : blocks)
	{
		if (block && block->bActive) {
			if ((static_cast<uint32_t>(block->layer) & mask) != 0)
				block->bHovered = IsPointInRect(block->hitboxRect, fMouseX, fMouseY);
		}
	}

	std::vector<TextureBlock*> readyCheckblocks = texture_ui_manager->GetReadyCheckBlocks();
	if (readyCheckblocks.empty()) return;

	for (auto& block : readyCheckblocks)
	{
		if (block && block->bActive) {
			if (block->ui_type != UI_EFFECT_TRANSLUCENT) {
				if ((static_cast<uint32_t>(block->layer) & mask) != 0)
					block->bHovered = IsPointInRect(block->hitboxRect, fMouseX, fMouseY);
			}
		}
	}
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
		{
			if (Screen_Fade == true && Mouse_Lock == false)
				::PostQuitMessage(0);

			Mouse_Lock = false;
			Screen_Fade = true;

			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Screen_Fade, true);
			}
		}
		break;

		case 'Q':
			{
			blur_effect = !blur_effect;
			m_pPlayer->SetBlurMask(blur_effect);
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

							// focus_point
							test_sand->SetPosition(XMFLOAT3(1200.0f, 1000.0f, 1200.0f));
							test_sand->Set_Focus_Point(anubisPos);
							test_sand->Set_Speed(0.0f);

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
	obj_manager->Check_Player_Collision(m_pPlayer);

	obj_manager->Check_Fixed_OBB_Camera_Culling(pd3dDevice, pd3dCommandList, main_Camera.get());
#endif

	obj_manager->Update(pd3dDevice, pd3dCommandList);


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

	if (bHitSignal)
	{
		bHitSignal = false;

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


		m_pPlayer->Set_Color_Blending(XMFLOAT3(1.0f, 0.0f, 0.0f), 1.0f);
	}

	m_pPlayer->Update_Color_Blending(-0.01f);
}

void CScene::After_Update_Objects()
{
#ifdef RENDER_WAVE
	shared_ptr<Wave_Object> wave_obj = obj_manager->Get_Wave_Object();
	if (wave_obj)
		wave_obj->Readback_Buffer_Data();

#endif

}


void CScene::Prepare_Shadow_Map_Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (!shadow_camera || shadow_camera->update_shadow == false)
		return;

	for (int i = 0; i < m_nLights; ++i)
	{
		if (m_pLights[i].m_bEnable && m_pLights[i].m_nType == DIRECTIONAL_LIGHT)
		{
			int numCascades = NUM_CASCADES;
			float lambda = 0.7f;
			std::vector<float> splits = shadow_camera->GenerateCSMSplitDepths(CAMERA_NEAR, CAMERA_FAR, numCascades, lambda);

			shadow_camera->SetupCSMCascades(m_pLights[i].m_xmf3Direction, splits, main_Camera.get());
			shadow_camera->update_shadow = true;
			break;
		}
	}


}

void CScene::Shadow_Map_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int n)
{
	if (!shadow_camera || shadow_camera->update_shadow == false)
		return;

	if (m_MRT_GraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_MRT_GraphicsRootSignature.get());

	shadow_camera->SetViewportsAndScissorRects(pd3dCommandList);


	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = shadow_camera->Get_Shadow_Map_DSV(n);

	pd3dCommandList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);
	pd3dCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	shadow_camera->Update_Render_ShaderVariables(pd3dCommandList, n);

	obj_manager->Render_Objects_Shadow_All(pd3dCommandList, shadow_camera.get());
	m_pPlayer->Render_Shadow(pd3dCommandList, shadow_camera.get()); 

}

void CScene::Prepare_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_MRT_GraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_MRT_GraphicsRootSignature.get());

	main_Camera.get()->Update_Render_ShaderVariables(pd3dCommandList);
	main_Camera.get()->Update_Last_Frame_Info(pd3dCommandList);


	// Light Update
	UpdateShaderVariables(pd3dCommandList);

}

void CScene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	obj_manager->Render_Objects_All(pd3dCommandList, main_Camera.get());
}

void CScene::Render_SkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_MRT_GraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_MRT_GraphicsRootSignature.get());

	main_Camera.get()->Update_Render_ShaderVariables(pd3dCommandList);


	if (m_pSkyBox) 
		m_pSkyBox->Render(pd3dCommandList, main_Camera.get());

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
}

void CScene::Post_Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	obj_manager->Post_Update_All();
}

void CScene::Set_UI_Layer_Active(std::vector<TextureBlock*>& blocks, UILayer targetLayer, bool bEnable)
{
	uint32_t targetMask = static_cast<uint32_t>(targetLayer);

	for (auto& block : blocks)
	{
		if (block && (static_cast<uint32_t>(block->layer) & targetMask) != 0)
		{
			block->start_time = current_time;
			if (bEnable)
			{
				if ((static_cast<uint32_t>(block->layer) & static_cast<uint32_t>(UILayer::Dialogue_Button)) != 0)
				{
					block->bPendingActivation = true;
					continue;
				}
				block->bActive = true;
				block->bPendingActivation = false;
			}
			else
			{
				block->bActive = false;
				block->bPendingActivation = false;
			}
		}
	}
}

void CScene::Bind_Player_UI_Callback()
{
	if (m_pPlayer && m_pPlayer->GetStateMachine())
	{
		m_pPlayer->GetStateMachine()->onGetHitEffect = [this](bool bEnable) {
			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			this->Set_UI_Layer_Active(blocks, UILayer::screen, bEnable);
			bUpdateUI_Screen = bEnable;
			bUpdateUI_HP = bEnable;
			bHitSignal = bEnable;
			};
	}
}


Change_Signal CScene::Get_Change_Signal()
{
	if (c_signal.change)
	{
		Change_Signal temp = c_signal;
		c_signal.change = false;
		return temp;
	}
	return c_signal;
}

void CScene::Add_Multi_Player(shared_ptr<CPlayer> new_player_ptr)
{
	obj_manager->Add_Player(new_player_ptr);
}

void CScene::Remove_Multi_Player(int player_id)
{
	obj_manager->Remove_Player(player_id);
}


void CScene::Sync_Player_Data(int player_id, ServerSyncData sync_data)
{
	obj_manager->Sync_Player_Data(player_id, sync_data);
}

//==========================================================================================

void Character_Select_Scene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 5;
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

	m_pLights[4].m_bEnable = true;
	m_pLights[4].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[4].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights[4].m_xmf3Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

}

void Character_Select_Scene::Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);

	m_pSkyBox = make_shared<CSkyBox>(pd3dDevice, pd3dCommandList);
	m_pSkyBox->Set_BaseTexture(pd3dDevice, pd3dCommandList, L"SkyBox/Fluffball.dds");

	m_pLights[0].m_bEnable = true;
	m_pLights[1].m_bEnable = true;
	m_pLights[2].m_bEnable = true;
	m_pLights[3].m_bEnable = false;

}


void Character_Select_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);

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
		player->SetID(i);
		player->SetPosition(XMFLOAT3(rotatedX, y, rotatedZ));
		player->type = EObjectType::SelectPlayer;
		player->GetStateMachine()->changeState(State::Select_Idle, Key_Value::None);
		obj_manager->Add_Object(player, Object_Type::skinned);
	}

	CLoadedModelInfo* Test_Island_Model = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, "Model/Island_0.bin", NULL);
	std::shared_ptr<CGameObject> test_Island = CGameObject::Make_Instance(Test_Island_Model->m_pModelRootObject, true);
	test_Island->Set_Name("Island");
	test_Island->SetScale({ 2.0f, 3.0f ,2.0f }, true);
	test_Island->SetPosition(0.0f, 0.0f, 0.0f);

	obj_manager->Add_Object(test_Island, Object_Type::fixed);


	//=====================================================
	Object_Manager::Reserve_Update();
	Light_Material_Manager::Update(pd3dDevice, pd3dCommandList);

	Build_Texture_UI(pd3dDevice, pd3dCommandList, m_UI_GraphicsRootSignature);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void Character_Select_Scene::Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	CScene::Animate_Objects(pd3dCommandList, fTimeElapsed);

	if (!m_pPlayer) 
		return;

	XMFLOAT3 targetPos = m_pPlayer->GetPosition();
	targetPos.y = 0.0f;



	auto playerList = obj_manager->Get_Object_List(Object_Type::skinned);
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

	static float enterTime = 0.0f;

	if (bStartAnimation)
	{
		enterTime += fTimeElapsed;
		float duration = 1.0f; 
		float t = min(enterTime / duration, 1.0f);

		XMFLOAT3 startPos = m_pPlayer->GetPosition();
		XMFLOAT3 endPos = { 47.0f, -5.0f, 25.0f };     

		XMVECTOR vStart = XMLoadFloat3(&startPos);
		XMVECTOR vEnd = XMLoadFloat3(&endPos);
		XMVECTOR vInterp = XMVectorLerp(vStart, vEnd, t);

		XMFLOAT3 newPos;
		XMStoreFloat3(&newPos, vInterp);
		m_pPlayer->SetPosition(newPos);

	/*	XMFLOAT3 targetLook = { 1000.0f, 0.0f, 0.0f };
		XMVECTOR lookDir = XMVectorSubtract(XMLoadFloat3(&targetLook), vInterp);
		lookDir = XMVector3Normalize(lookDir);

		XMFLOAT3 finalLook;
		XMStoreFloat3(&finalLook, lookDir);
		m_pPlayer->SetLookDirection(finalLook);*/

		if (t >= 1.0f)
		{
			bStartAnimation = false;
			enterTime = 0.0f;
		}
	}
}

void Character_Select_Scene::Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	obj_manager->Update(pd3dDevice, pd3dCommandList);
	UpdatePlayerSelection();
}


void Character_Select_Scene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::Render(pd3dDevice, pd3dCommandList);
}


void Character_Select_Scene::UpdatePlayerSelection()
{
	auto player_list = obj_manager->Get_Object_List(Object_Type::skinned);
	if (!player_list) return;


	for (auto& player : *player_list)
	{
		if (!player) continue;
		player->SetOutlineColor(0);
	}

	if (isRunning)
	{
		if (bSelectStart) {
			auto MugList = texture_ui_manager->GetMugBlocks();
			auto readyList = texture_ui_manager->GetReadyCheckBlocks();

			texture_ui_manager->DeactivateAllMugBlocks();
			texture_ui_manager->DeactivateReadyCheckBlocks();

			for (int charId = 0; charId < MaxPlayer; ++charId)
			{
				if (charId >= player_list->size()) continue;
				auto& player = (*player_list)[charId];
				if (!player) continue;

				int readyClient = readyClientIds[charId];   
				if (readyClient != -1)
				{
					MugList[readyClient]->bActive = true;
					MugList[readyClient]->ui_type = UI_EFFECT_NONE;
					MugList[readyClient]->pTexture = Texture_UI_Manager::s_MugTextures[charId].get();

					if (!readyList.empty()) {
						readyList[readyClient]->bActive = true;
						/*if (readyClient == GetClientNum())
							SetEnableCharactorSelectButton(false, readyList);*/
					}

					player->SetOutlineColor(readyClient + 1); 
				}
				else {
					/*if (GetClientNum() != -1)
						if (characterSelections[charId].test(GetClientNum())) {
							is_Ready = 0;
							SetEnableCharactorSelectButton(true, readyList);
						}*/
				}

				for (int clientId = 0; clientId < MaxPlayer; ++clientId)
				{
					if (!characterSelections[charId].test(clientId)) continue;  

					if (clientId == readyClient) continue;

					MugList[clientId]->bActive = true;
					MugList[clientId]->ui_type = UI_EFFECT_TRANSLUCENT;
					MugList[clientId]->pTexture = Texture_UI_Manager::s_MugTextures[charId].get();

					player->SetOutlineColor(clientId + 1);
				}
			}
		}
	}
	else
	{
		if (select_index >= 0 && select_index < player_list->size())
		{
			auto& selected = (*player_list)[select_index];
			if (selected) 
				selected->SetOutlineColor(1);

			XMFLOAT3 character_pos = (*player_list)[select_index]->GetPosition();
			m_pLights[3].m_bEnable = true;
			m_pLights[3].m_xmf3Position = character_pos;
			m_pLights[3].m_xmf3Position.y += 15.0f;
		}

		if (bSelectStart) {
			auto MugList = texture_ui_manager->GetMugBlocks();
			if (!MugList[0]->bActive) MugList[0]->bActive = true;
			MugList[0]->pTexture = Texture_UI_Manager::s_MugTextures[select_index].get();
		}
	}


}


bool Character_Select_Scene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
		{
			if (Screen_Fade == true && Mouse_Lock == false)
				::PostQuitMessage(0);

			Screen_Fade = true;
			Mouse_Lock = false;

			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Screen_Fade, true);
			}
		}
		break;

		case 'Z':
			if (is_Ready == 0)
				select_index = (select_index - 1 + 6) % 6;
			break;

		case 'C':
			if (is_Ready == 0)
				select_index = (select_index + 1 + 6) % 6;
			break;

		case 'F':		case 'f':
		{
			// Toggle Fog On/Off
			fog_info->Fog_Trigger ^= 1;
		}
		break;

		case VK_CONTROL:
		{
			c_signal.change = true;
			c_signal.scene_name = "Stage_1";
			c_signal.type = Scene_Type::Stage_1;

			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Screen_Fade, false);
				Screen_Fade = false;
				Mouse_Lock = true;
			}
		}
				
		default:
			break;
		}
	}
	return(false);
}

void Character_Select_Scene::Build_Texture_UI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::shared_ptr<ID3D12RootSignature> pRootSignature)
{
	texture_ui_manager = new Texture_UI_Manager();
	if (!texture_ui_manager) return;

	texture_ui_manager->SetRenderer(make_unique<Texture_UI_Renderer>(pd3dDevice));
	texture_ui_manager->GetRenderer()->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	std::unique_ptr<CTextureToScreenShader> pShader = std::make_unique<CTextureToScreenShader>();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pRootSignature);
	texture_ui_manager->SetShader(std::move(pShader));
	texture_ui_manager->SetRootSignature(pRootSignature);
	std::shared_ptr<CTextureMesh> mesh = std::make_shared<CTextureMesh>(pd3dDevice, pd3dCommandList, 2.0f, 2.0f);

	Texture_UI_Manager::s_MugTextures[0] = std::make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	Texture_UI_Manager::s_MugTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/Captain_mug.dds", RESOURCE_TEXTURE2D, 0);
	Texture_UI_Manager::s_MugTextures[1] = std::make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	Texture_UI_Manager::s_MugTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/Deckhand_mug.dds", RESOURCE_TEXTURE2D, 0);
	Texture_UI_Manager::s_MugTextures[2] = std::make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	Texture_UI_Manager::s_MugTextures[2]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/Female_Pirate_mug.dds", RESOURCE_TEXTURE2D, 0);
	Texture_UI_Manager::s_MugTextures[3] = std::make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	Texture_UI_Manager::s_MugTextures[3]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/First_Mate_mug.dds", RESOURCE_TEXTURE2D, 0);
	Texture_UI_Manager::s_MugTextures[4] = std::make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	Texture_UI_Manager::s_MugTextures[4]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/Seaman_mug.dds", RESOURCE_TEXTURE2D, 0);
	Texture_UI_Manager::s_MugTextures[5] = std::make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	Texture_UI_Manager::s_MugTextures[5]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/Skeleton_mug.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, Texture_UI_Manager::s_MugTextures[0].get(), 0, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, Texture_UI_Manager::s_MugTextures[1].get(), 0, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, Texture_UI_Manager::s_MugTextures[2].get(), 0, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, Texture_UI_Manager::s_MugTextures[3].get(), 0, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, Texture_UI_Manager::s_MugTextures[4].get(), 0, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, Texture_UI_Manager::s_MugTextures[5].get(), 0, 0);
	float xStart = 0.06f;
	float xGap = 0.08f;
	float yStart = 0.12f;
	float xSize = 0.06f;
	for (int i = 0; i < MaxPlayer; i++) {
		D2D1_RECT_F mug_Rect = MakeNormalizedRect(xStart + xGap * i, yStart, xSize, Texture_UI_Manager::s_MugTextures[0].get());
		std::shared_ptr<TextureBlock> mug_block = std::make_shared<TextureBlock>(Texture_UI_Manager::s_MugTextures[0].get(), mug_Rect, mesh, UILayer::Menu);
		mug_block->ui_type = UI_EFFECT_TRANSLUCENT;
		mug_block->bActive = false;
		texture_ui_manager->AddMugBlock(mug_block);
	}

	CTexture* BackGround = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	BackGround->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/backGround.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, BackGround, 0, 0);
	D2D1_RECT_F CharSelectBG = MakeNormalizedRect(0.26f, 0.18f, 0.5f, BackGround, 0.2f);
	std::unique_ptr<TextureBlock> CSBGblock = std::make_unique<TextureBlock>(BackGround, CharSelectBG, mesh, UILayer::Menu);
	CSBGblock->bActive = false;
	CSBGblock->ui_type = UI_EFFECT_FADE_IN;
	texture_ui_manager->Add_TextureBlock(std::move(CSBGblock));

	CTexture* SelectbutttonTexture = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	SelectbutttonTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/downPart.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, SelectbutttonTexture, 0, 0);
	D2D1_RECT_F SlbTscreenRect = MakeNormalizedRect(0.85f, 0.12f, 0.3f, SelectbutttonTexture);
	std::unique_ptr<TextureBlock> SlbTblock = std::make_unique<TextureBlock>(SelectbutttonTexture, SlbTscreenRect, mesh, UILayer::Menu);
	SlbTblock->bActive = false;
	SlbTblock->ui_type = UI_EFFECT_FADE_IN;
	texture_ui_manager->Add_TextureBlock(std::move(SlbTblock));

	CTexture* SelectTxtTexture = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	SelectTxtTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/SelectTxt.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, SelectTxtTexture, 0, 0);
	D2D1_RECT_F SlTTscreenRect = MakeNormalizedRect(0.85f, 0.105f, 0.15f, SelectTxtTexture);
	std::unique_ptr<TextureBlock> SlTTblock = std::make_unique<TextureBlock>(SelectTxtTexture, SlTTscreenRect, mesh, UILayer::Menu);
	SlTTblock->bActive = false;
	SlTTblock->ui_type = UI_EFFECT_FADE_IN;
	texture_ui_manager->Add_TextureBlock(std::move(SlTTblock));

	CTexture* check = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	check->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/correct-symbol.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, check, 0, 0);
	for (int i = 0; i < MaxPlayer; i++) {
		D2D1_RECT_F ready_Rect = MakeNormalizedRect(xStart + xGap * i + 0.002, yStart + 0.12f, xSize, check);
		TextureBlock* ready_block = new TextureBlock(check, ready_Rect, mesh);
		ready_block->bActive = false;
		texture_ui_manager->AddReadyCheckBlock(ready_block);
	}

	D2D1_RECT_F check_Rect = MakeNormalizedRect(0.81f, 0.23f, 0.05f, check);
	TextureBlock* check_block = new TextureBlock(check, check_Rect, mesh, UILayer::Interactable | UILayer::Menu);
	check_block->onClick = [this]()
		{
			if (isRunning) {
				if (readyClientIds[CScene::select_index] == -1) {
					is_Ready = 1;
					c_signal.change = true;
					SetEnableCharactorSelectButton(false);
				}
			}
			else {
				is_Ready = 1;
				c_signal.change = true;
			}
			c_signal.scene_name = "Game_Stage_Board";
			c_signal.type = Scene_Type::Board;
		};
	check_block->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	check_block->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	check_block->bActive = false;
	texture_ui_manager->AddReadyCheckBlock(check_block);

	CTexture* return_to_select = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	return_to_select->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/remove-symbol.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, return_to_select, 0, 0);
	D2D1_RECT_F RTS_screenRect = MakeNormalizedRect(0.9f, 0.23f, 0.05f, return_to_select);
	TextureBlock* RTS_block = new TextureBlock(return_to_select, RTS_screenRect, mesh, UILayer::Interactable | UILayer::Menu);
	RTS_block->onClick = [this]()
		{
			is_Ready = 0;
			c_signal.change = false;
			SetEnableCharactorSelectButton(true);
		};
	RTS_block->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	RTS_block->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	RTS_block->bActive = false;
	RTS_block->ui_type = UI_EFFECT_TRANSLUCENT;
	texture_ui_manager->AddReadyCheckBlock(RTS_block);

	CTexture* StartbutttonTexture = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	StartbutttonTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/upper-login.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, StartbutttonTexture, 0, 0);
	D2D1_RECT_F SbTscreenRect = MakeNormalizedRect(0.5f, 0.5f, 0.5f, StartbutttonTexture);
	std::unique_ptr<TextureBlock> SbTblock = std::make_unique<TextureBlock>(StartbutttonTexture, SbTscreenRect, mesh, UILayer::Interactable | UILayer::Start);
	SbTblock->onClick = [this]() {
		std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
		if (!blocks.empty())
		{
			Set_UI_Layer_Active(blocks, UILayer::Start, false);
			Set_UI_Layer_Active(blocks, UILayer::Menu, true);
			bStartAnimation = true;
			bSelectStart = true;
		}
		std::vector<TextureBlock*> Readyblocks = texture_ui_manager->GetReadyCheckBlocks();
		if (!Readyblocks.empty())
		{
			Set_UI_Layer_Active(Readyblocks, UILayer::Menu, true);
		}
		};
	SbTblock->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	SbTblock->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	texture_ui_manager->Add_TextureBlock(std::move(SbTblock));

	CTexture* StartTxtTexture = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	StartTxtTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/StartTxt.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, StartTxtTexture, 0, 0);
	D2D1_RECT_F STTscreenRect = MakeNormalizedRect(0.5f, 0.47f, 0.28f, StartTxtTexture);
	std::unique_ptr<TextureBlock> STTblock = std::make_unique<TextureBlock>(StartTxtTexture, STTscreenRect, mesh, UILayer::Interactable | UILayer::Start);
	STTblock->hitboxRect = SbTscreenRect;
	STTblock->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	STTblock->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	texture_ui_manager->Add_TextureBlock(std::move(STTblock));
}

void Character_Select_Scene::SetEnableCharactorSelectButton(bool Enable)
{
	std::vector<TextureBlock*> blocks = texture_ui_manager->GetReadyCheckBlocks();
	if (Enable) {
		blocks[CHARAACTER_SELECT_CHECK_INDEX]->ui_type = UI_EFFECT_NONE;
		blocks[CHARAACTER_SELECT_CANCEL_INDEX]->ui_type = UI_EFFECT_TRANSLUCENT;
		blocks[CHARAACTER_SELECT_CANCEL_INDEX]->bHovered = false;
	}
	else {
		blocks[CHARAACTER_SELECT_CHECK_INDEX]->ui_type = UI_EFFECT_TRANSLUCENT;
		blocks[CHARAACTER_SELECT_CHECK_INDEX]->bHovered = false;
		blocks[CHARAACTER_SELECT_CANCEL_INDEX]->ui_type = UI_EFFECT_NONE;
	}
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
	m_pLights[8].m_xmf3Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);


}

void Board_Scene::Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);

	m_pSkyBox = make_shared<CSkyBox>(pd3dDevice, pd3dCommandList);
	m_pSkyBox->Set_BaseTexture(pd3dDevice, pd3dCommandList, L"SkyBox/Fluffball.dds");

	m_pLights[0].m_bEnable = true;
	m_pLights[1].m_bEnable = true;
	m_pLights[2].m_bEnable = true;
	m_pLights[3].m_bEnable = false;

}


void Board_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);

	island_points = {
	XMFLOAT3(-1200.0f, 40.0f, -1200.0f),
	XMFLOAT3(-1200.0f, 40.0f, 0.0f),
	XMFLOAT3(-1200.0f, 40.0f, 1200.0f),
	XMFLOAT3(0.0f,     40.0f, 1200.0f),
	XMFLOAT3(1200.0f,  40.0f, 1200.0f),
	XMFLOAT3(1200.0f,  40.0f, 0.0f),
	XMFLOAT3(1200.0f,  40.0f, -1200.0f)
	}; 

#ifdef RENDER_WAVE
	std::shared_ptr<Wave_Object> wave_obj = std::make_shared<Wave_Object>(pd3dDevice, pd3dCommandList, m_Plane_GraphicsRootSignature, 3000, 100, true);
	wave_obj->Set_Name("board_scene_wave");
	wave_obj->SetPosition(XMFLOAT3(0.0f, 10.0f, 0.0f));

	wave_obj->Set_BaseTexture(pd3dDevice, pd3dCommandList, L"Terrain/Water_Base_Texture_0.dds");
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


		test_Island_0->SetPosition(island_points[0]);
		m_pLights[0].m_xmf3Position = island_points[0];

		test_Island_1->SetPosition(island_points[1]);
		m_pLights[1].m_xmf3Position = island_points[1];

		
		test_Island_2->SetPosition(island_points[2]);
		m_pLights[2].m_xmf3Position = island_points[2];

		test_Island_3->SetPosition(island_points[3]);
		m_pLights[3].m_xmf3Position = island_points[3];


		test_Island_4->SetPosition(island_points[4]);
		m_pLights[4].m_xmf3Position = island_points[4];


		test_Island_5->SetPosition(island_points[5]);
		m_pLights[5].m_xmf3Position = island_points[5];

		test_Island_6->SetPosition(island_points[6]);
		m_pLights[6].m_xmf3Position = island_points[6];


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
	Light_Material_Manager::Update(pd3dDevice, pd3dCommandList);

	Build_Texture_UI(pd3dDevice, pd3dCommandList, m_UI_GraphicsRootSignature);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void Board_Scene::Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	fog_info->time += fTimeElapsed;

#ifdef RENDER_WAVE

	CS_Wave_Shader::update_wave_info->g_WaveMin = 0.35f;
	CS_Wave_Shader::update_wave_info->g_WaveMax = 0.75f;
	CS_Wave_Shader::update_wave_info->g_HeightDamping = 0.05f;

	Deferred_Plane_Shader::Update(fTimeElapsed);

	shared_ptr<Wave_Object> wave_obj = obj_manager->Get_Wave_Object();

	if (wave_obj)
	{
		wave_obj->Synchronize_Wave_to_Boat(pirate_ship.get());
		wave_obj->Animate(pd3dCommandList, fTimeElapsed);
	}

#endif

	if (!isRunning)
	{
		pirate_ship->Animate(fTimeElapsed);
		pirate_ship->HandleBoundaryReflection(1500.0f);
	}

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
		auto pCamera = m_pPlayer->GetCamera();
		XMFLOAT3 currentCamPos = pCamera->GetPosition();
		XMFLOAT3 targetPos = pirate_ship->GetPosition();
		targetPos.y += 1000.0f + cameraYOffset;
		targetPos.z += 500.0f + cameraYOffset / 2.0f;

		float lerpAlpha = 0.1f;

		XMVECTOR vCurrent = XMLoadFloat3(&currentCamPos);
		XMVECTOR vTarget = XMLoadFloat3(&targetPos);
		XMVECTOR vInterpolated = XMVectorLerp(vCurrent, vTarget, lerpAlpha);

		XMFLOAT3 interpolatedPos;
		XMStoreFloat3(&interpolatedPos, vInterpolated);

		pCamera->SetPosition(interpolatedPos);

		pCamera->SetLookDirection(XMFLOAT3(0.0f, -0.866f, -0.5f));
	}


	std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
	if (!blocks.empty())
	{
		int is_nearby = Get_Closest_Island_Index(300.0f);

		if (is_nearby != -1)
		{
			if (!bMenuActive && !bClosedByUser)
			{
				bMenuActive = true;
				Screen_Fade = true;
				Set_UI_Layer_Active(blocks, UILayer::Dialogue | UILayer::Dialogue_Button, true);
			}

			if (isRunning)
			{
				nearest_stage_index = is_nearby;
			}
		}
		else
		{
			bMenuActive = false;
			bClosedByUser = false;
			Set_UI_Layer_Active(blocks, UILayer::Dialogue | UILayer::Dialogue_Button, false);

			if (isRunning)
			{
				nearest_stage_index = -1;
				is_stage_select = false;
			}
		}
	}
}

void Board_Scene::After_Update_Objects()
{
	CScene::After_Update_Objects();

}

int Board_Scene::Get_Closest_Island_Index(float range)
{
	XMFLOAT3 boat_pos = pirate_ship->GetPosition();


	int closest_index = -1;
	float min_distance = range + 1.0f;  // 초기값: range보다 약간 큼

	for (int i = 0; i < island_points.size(); ++i)
	{
		XMVECTOR v1 = XMLoadFloat3(&boat_pos);
		XMVECTOR v2 = XMLoadFloat3(&island_points[i]);
		XMVECTOR diff = XMVectorSubtract(v1, v2);
		float distance = XMVectorGetX(XMVector3Length(diff));

		if (distance <= range && distance < min_distance)
		{
			min_distance = distance;
			closest_index = i;
		}
	}

	return closest_index;
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


void Board_Scene::Transparent_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
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

}


bool Board_Scene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
		{
			if (Screen_Fade == true && Mouse_Lock == false)
				::PostQuitMessage(0);

			Screen_Fade = true;
			Mouse_Lock = false;

			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Screen_Fade, true);
			}
		}
		break;

		case 'Q':
		{
			if (test_button)
				break;

			test_button = true;
		}	break;

		case 'W':		case 'w':
		{
			if(!isRunning)
				pirate_ship->MoveForward(20);
		}
		break;

		case 'A':		case 'a':
		{
			if (!isRunning)
				pirate_ship->Add_Rotate(-10.0f);
		}
		break;

		case 'D':		case 'd':
		{
			if (!isRunning)
				pirate_ship->Add_Rotate(10.0f);
		}
		break;

		case 'F':		case 'f':
		{
			// Toggle Fog On/Off
			fog_info->Fog_Trigger ^= 1;
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

		case VK_RETURN: {
			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Dialogue | UILayer::Dialogue_Button, !bMenuActive);

				bMenuActive = !bMenuActive;
			}
		}
			break;


		default:
			break;
		}
	}
	return(false);
}

bool Board_Scene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	{

		std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
		if (blocks.empty()) return false;

		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);
		float fMouseX = static_cast<float>(mouseX);
		float fMouseY = static_cast<float>(mouseY);


		uint32_t mask = static_cast<uint32_t>(UILayer::Interactable);

		for (auto& block : blocks)
		{
			if (block && block->bActive) {
				if ((static_cast<uint32_t>(block->layer) & mask) != 0) {
					if (IsPointInRect(block->hitboxRect, fMouseX, fMouseY))
					{
						if (block->onClick) block->onClick();
						return true;
					}
				}
			}
		}

	}	break;

	case WM_MOUSEWHEEL:
	{
		short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam); 

		const float scrollSpeed = 50.0f; 
		if (wheelDelta > 0)
		{
			cameraYOffset -= scrollSpeed;
		}
		else if (wheelDelta < 0)
		{
			cameraYOffset += scrollSpeed;
		}

		cameraYOffset = std::clamp(cameraYOffset, -500.0f, 500.0f);

		return true; 
	}

	default:
		break;
	}

	return false;
}

void Board_Scene::Build_Texture_UI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::shared_ptr<ID3D12RootSignature> pRootSignature)
{
	texture_ui_manager = new Texture_UI_Manager();
	if (!texture_ui_manager) return;

	texture_ui_manager->SetRenderer(make_unique<Texture_UI_Renderer>(pd3dDevice));
	texture_ui_manager->GetRenderer()->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	std::unique_ptr<CTextureToScreenShader> pShader = std::make_unique<CTextureToScreenShader>();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pRootSignature);
	texture_ui_manager->SetShader(std::move(pShader));
	texture_ui_manager->SetRootSignature(pRootSignature);
	std::shared_ptr<CTextureMesh> mesh = std::make_shared<CTextureMesh>(pd3dDevice, pd3dCommandList, 2.0f, 2.0f);

	CTexture* BackGround = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	BackGround->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/downboardName.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, BackGround, 0, 0);
	D2D1_RECT_F BGscreenRect = MakeNormalizedRect(0.5f, 0.5f, 0.68f, BackGround);
	std::unique_ptr<TextureBlock> BGblock = std::make_unique<TextureBlock>(BackGround, BGscreenRect, mesh, UILayer::Dialogue);
	BGblock->ui_type = UI_EFFECT_SLIDE_DOWN;
	BGblock->hp = 1;
	BGblock->bActive = false;
	texture_ui_manager->Add_TextureBlock(std::move(BGblock));

	CTexture* YesButton = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	YesButton->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/correct-symbol.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, YesButton, 0, 0);
	D2D1_RECT_F YesscreenRect = MakeNormalizedRect(0.68f, 0.85f, 0.05f, YesButton);
	std::unique_ptr<TextureBlock> Yesblock = std::make_unique<TextureBlock>(YesButton, YesscreenRect, mesh, UILayer::Interactable | UILayer::Dialogue | UILayer::Dialogue_Button);
	Yesblock->onClick = [this]()
		{
			c_signal.change = true;
			c_signal.scene_name = "Stage_1";
			c_signal.type = Scene_Type::Stage_1;
			is_stage_select = true;
			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Screen_Fade, false);
				Screen_Fade = false;
				Mouse_Lock = true;
			}
		};
	Yesblock->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	Yesblock->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	Yesblock->hp = 1;
	Yesblock->bActive = false;
	texture_ui_manager->Add_TextureBlock(std::move(Yesblock));


	CTexture* NoButton = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	NoButton->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/remove-symbol.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, NoButton, 0, 0);
	D2D1_RECT_F NoscreenRect = MakeNormalizedRect(0.75f, 0.85f, 0.05f, NoButton);
	std::unique_ptr<TextureBlock> Noblock = std::make_unique<TextureBlock>(NoButton, NoscreenRect, mesh, UILayer::Interactable | UILayer::Dialogue | UILayer::Dialogue_Button);
	Noblock->onClick = [this]()
		{
			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Dialogue | UILayer::Dialogue_Button, false);
				bMenuActive = false;
				bClosedByUser = true;
				Screen_Fade = false;
			}
		};
	Noblock->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	Noblock->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	Noblock->hp = 1;
	Noblock->bActive = false;
	texture_ui_manager->Add_TextureBlock(std::move(Noblock));



	CTexture* return_to_game = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	return_to_game->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/remove-symbol.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, return_to_game, 0, 0);
	D2D1_RECT_F RTG_screenRect = MakeNormalizedRect(0.7f, 0.1f, 0.05f, return_to_game);
	std::unique_ptr<TextureBlock> RTG_block = std::make_unique<TextureBlock>(return_to_game, RTG_screenRect, mesh, UILayer::Interactable | UILayer::Screen_Fade);
	RTG_block->onClick = [this]()
		{
			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Screen_Fade, false);
				Screen_Fade = false;

			}
		};
	RTG_block->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	RTG_block->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	RTG_block->bActive = false;
	texture_ui_manager->Add_TextureBlock(std::move(RTG_block));


	CTexture* replay = new CTexture(1, RESOURCE_TEXTURE2D, 1, 1, 0, 0, 1, 0, 0);
	replay->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UITexture/undo-arrow.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, replay, 0, 0);
	D2D1_RECT_F REscreenRect = MakeNormalizedRect(0.8f, 0.1f, 0.05f, replay);
	std::unique_ptr<TextureBlock> REblock = std::make_unique<TextureBlock>(replay, REscreenRect, mesh, UILayer::Interactable | UILayer::Screen_Fade);
	REblock->onClick = [this]()
		{
			c_signal.change = true;
			c_signal.scene_name = "Character_Select";
			c_signal.type = Scene_Type::Lobby;

			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Screen_Fade, false);
				Screen_Fade = false;
			}
		};
	REblock->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	REblock->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	REblock->bActive = false;
	texture_ui_manager->Add_TextureBlock(std::move(REblock));


	D2D1_RECT_F TestSkipscreenRect = MakeNormalizedRect(0.9f, 0.1f, 0.05f, YesButton);
	std::unique_ptr<TextureBlock> TestSkipblock = std::make_unique<TextureBlock>(YesButton, TestSkipscreenRect, mesh, UILayer::Interactable | UILayer::Screen_Fade);
	TestSkipblock->onClick = [this]()
		{
			c_signal.change = true;
			c_signal.scene_name = "Stage_1";
			c_signal.type = Scene_Type::Stage_1;

			std::vector<TextureBlock*> blocks = texture_ui_manager->GetTextureBlockPtrs();
			if (!blocks.empty())
			{
				Set_UI_Layer_Active(blocks, UILayer::Screen_Fade, false);
				Screen_Fade = false;
				Mouse_Lock = true;
			}
		};
	TestSkipblock->tintColor = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
	TestSkipblock->hoverGlowColor = XMFLOAT4(1.0f, 0.4f, 0.4f, 1.0f);
	TestSkipblock->hp = 1;
	TestSkipblock->bActive = false;
	texture_ui_manager->Add_TextureBlock(std::move(TestSkipblock));
}

void Board_Scene::OnMenuCloseButtonClicked(std::vector<TextureBlock*>& blocks)
{
	bMenuActive = false;
	Screen_Fade = false;
	bClosedByUser = true;
	Set_UI_Layer_Active(blocks, UILayer::Dialogue | UILayer::Dialogue_Button, false);
}

void Board_Scene::Sync_Boat_Server(XMFLOAT3 pos, XMFLOAT3 look)
{
	pirate_ship->SetPosition(pos); 
	pirate_ship->SetLookDirection(look); 
}

pair<int, bool> Board_Scene::Get_Sail_Status()
{
	return { nearest_stage_index, is_stage_select };
}

//==========================================================================================



void Test_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Prepare_Basic_Elements(pd3dDevice, pd3dCommandList);

	m_pSkyBox = make_shared<CSkyBox>(pd3dDevice, pd3dCommandList);
	m_pSkyBox->Set_BaseTexture(pd3dDevice, pd3dCommandList, L"SkyBox/Fluffball.dds");


	XMFLOAT3 xmf3Scale(10.0f, 0.0f, 10.0f); // y = 0 -> flat
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f); // HeightMap
	m_pTerrain = make_shared<CHeightMapTerrain>(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 257, 257, xmf3Scale, xmf4Color, 8, 3);
	m_pTerrain->DivideIntoChildren(pd3dDevice, pd3dCommandList, m_MRT_GraphicsRootSignature, _T("Terrain/HeightMap.raw"), xmf3Scale, 8);
	m_pTerrain->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	obj_manager->Set_Terrain_Object(m_pTerrain);



#ifdef RENDER_PARTICLE
	obj_manager->Update(pd3dDevice, pd3dCommandList); // Forward Update for ParticleManager's fixed obb data
	obj_manager->Update_Fixed_OBBs();
	particle_manager->Create_OBB_Data_ShaderVariables(pd3dDevice, pd3dCommandList, obj_manager->Get_Fixed_OBBs());
#endif

	Build_Texture_UI(pd3dDevice, pd3dCommandList, m_UI_GraphicsRootSignature);

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
