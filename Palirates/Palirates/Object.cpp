 //-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Scene.h"

std::unordered_map<std::string, std::shared_ptr<CAnimationSet>> CAnimationSets::s_GlobalAnimationSetCache;

CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers,
	int nGraphicsSrvRootParameters, int nComputeUavRootParameters, int nComputeSrvRootParameters,
	int nGraphicsSrvGpuHandles, int nComputeUavGpuHandles, int nComputeSrvGpuHandles, int nDsvHandles) : m_nTextureType(nTextureType)
{
	m_pnResourceTypes.resize(nTextures, 0);
	m_ppd3dTextures.resize(nTextures, nullptr);
	m_ppd3dTextureUploadBuffers.resize(nTextures, nullptr);
	m_pdxgiBufferFormats.resize(nTextures, DXGI_FORMAT_UNKNOWN);
	m_pnBufferElements.resize(nTextures, 0);
	m_pnBufferStrides.resize(nTextures, 0);
	m_nTextureWidths.resize(nTextures, 0);
	m_nTextureHeights.resize(nTextures, 0);

	m_pd3dGraphicsSrvGpuDescriptorHandles.resize(nGraphicsSrvGpuHandles, { 0 });
	m_pd3dComputeUavGpuDescriptorHandles.resize(nComputeUavGpuHandles, { 0 });
	m_pd3dComputeSrvGpuDescriptorHandles.resize(nComputeSrvGpuHandles, { 0 });

	m_pnGraphicsSrvRootParameterIndices.resize(nGraphicsSrvRootParameters, -1);
	m_pnGraphicsSrvRootParameterDescriptors.resize(nGraphicsSrvRootParameters, -1);
	m_GraphicsRootParameter_Srv_GpuDescriptorHandles.resize(nGraphicsSrvRootParameters, { 0 });

	m_pnComputeUavRootParameterIndices.resize(nComputeUavRootParameters, -1);
	m_pnComputeUavRootParameterDescriptors.resize(nComputeUavRootParameters, -1);
	m_pd3dComputeUavRootParameterGpuDescriptorHandles.resize(nComputeUavRootParameters, { 0 });

	m_pnComputeSrvRootParameterIndices.resize(nComputeSrvRootParameters, -1);
	m_pnComputeSrvRootParameterDescriptors.resize(nComputeSrvRootParameters, -1);
	m_pd3dComputeSrvRootParameterGpuDescriptorHandles.resize(nComputeSrvRootParameters, { 0 });

	m_pd3dSamplerGpuDescriptorHandles.resize(nSamplers, { 0 });
	m_d3dDsvCPUDescriptorHandles.resize(nDsvHandles, { 0 });

}


CTexture::~CTexture()
{
	for (auto& tex : m_ppd3dTextures)
		if (tex)
			tex->Release();

	for (auto& upload : m_ppd3dTextureUploadBuffers)
		if (upload)
			upload->Release();

	DebugOutput("\nDeleted Texture: ", m_pstrTextureName);
}

void CTexture::SetGraphicsSrvGpuDescriptorHandle(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
	m_pd3dGraphicsSrvGpuDescriptorHandles[index] = handle;
}

void CTexture::SetComputeUavGpuDescriptorHandle(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
	m_pd3dComputeUavGpuDescriptorHandles[index] = handle;
}

void CTexture::SetComputeSrvGpuDescriptorHandle(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
	m_pd3dComputeSrvGpuDescriptorHandles[index] = handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CTexture::GetGraphicsSrvGpuDescriptorHandle(int index) const
{
	return m_pd3dGraphicsSrvGpuDescriptorHandles[index];
}

D3D12_GPU_DESCRIPTOR_HANDLE CTexture::GetComputeUavGpuDescriptorHandle(int index) const
{
	return m_pd3dComputeUavGpuDescriptorHandles[index];
}

D3D12_GPU_DESCRIPTOR_HANDLE CTexture::GetComputeSrvGpuDescriptorHandle(int index) const
{
	return m_pd3dComputeSrvGpuDescriptorHandles[index];
}

void CTexture::SetGraphicsSrvRootParameter(int index, int rootParamIndex, int gpuHandleIndex, int srvDescriptors)
{
	m_pnGraphicsSrvRootParameterIndices[index] = rootParamIndex;
	m_GraphicsRootParameter_Srv_GpuDescriptorHandles[index] = m_pd3dGraphicsSrvGpuDescriptorHandles[gpuHandleIndex];
	m_pnGraphicsSrvRootParameterDescriptors[index] = srvDescriptors;
}

void CTexture::SetComputeUavRootParameter(int index, int rootParamIndex, int gpuHandleIndex, int uavDescriptors)
{
	m_pnComputeUavRootParameterIndices[index] = rootParamIndex;
	m_pd3dComputeUavRootParameterGpuDescriptorHandles[index] = m_pd3dComputeUavGpuDescriptorHandles[gpuHandleIndex];
	m_pnComputeUavRootParameterDescriptors[index] = uavDescriptors;
}

void CTexture::SetComputeSrvRootParameter(int index, int rootParamIndex, int gpuHandleIndex, int srvDescriptors)
{
	m_pnComputeSrvRootParameterIndices[index] = rootParamIndex;
	m_pd3dComputeSrvRootParameterGpuDescriptorHandles[index] = m_pd3dComputeSrvGpuDescriptorHandles[gpuHandleIndex];
	m_pnComputeSrvRootParameterDescriptors[index] = srvDescriptors;
}

void CTexture::UpdateGraphicsSrvShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pnGraphicsSrvRootParameterIndices.size() == m_ppd3dTextures.size())
	{
		for (int i = 0; i < m_pnGraphicsSrvRootParameterIndices.size(); ++i)
			UpdateGraphicsSrvShaderVariable(pd3dCommandList, i, i);
	}
	else
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnGraphicsSrvRootParameterIndices[0], m_GraphicsRootParameter_Srv_GpuDescriptorHandles[0]);
	}
}

void CTexture::UpdateGraphicsSrvShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int paramIndex, int textureIndex)
{
	if (m_pnGraphicsSrvRootParameterIndices[paramIndex] != -1 && m_GraphicsRootParameter_Srv_GpuDescriptorHandles[textureIndex].ptr != 0)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnGraphicsSrvRootParameterIndices[paramIndex],
			m_GraphicsRootParameter_Srv_GpuDescriptorHandles[textureIndex]);
	}
}

void CTexture::BindGraphicsSrvToRootParameter(ID3D12GraphicsCommandList* pd3dCommandList, int rootParamIndex, int textureIndex)
{
	if (textureIndex >= m_pd3dGraphicsSrvGpuDescriptorHandles.size())
		return;

	if (m_pd3dGraphicsSrvGpuDescriptorHandles[textureIndex].ptr == 0)
		return;

	pd3dCommandList->SetGraphicsRootDescriptorTable(rootParamIndex, m_pd3dGraphicsSrvGpuDescriptorHandles[textureIndex]);
}


void CTexture::UpdateComputeSrvShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pnComputeSrvRootParameterIndices.size() == m_ppd3dTextures.size())
	{
		for (int i = 0; i < m_pnComputeSrvRootParameterIndices.size(); ++i)
			UpdateComputeSrvShaderVariable(pd3dCommandList, i, i);
	}
	else if (!m_pnComputeSrvRootParameterIndices.empty())
	{
		pd3dCommandList->SetComputeRootDescriptorTable(
			m_pnComputeSrvRootParameterIndices[0],
			m_pd3dComputeSrvRootParameterGpuDescriptorHandles[0]);
	}
}

void CTexture::UpdateComputeSrvShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int paramIndex, int textureIndex)
{
	if (m_pnComputeSrvRootParameterIndices[paramIndex] != -1 &&
		m_pd3dComputeSrvRootParameterGpuDescriptorHandles[textureIndex].ptr != 0)
	{
		pd3dCommandList->SetComputeRootDescriptorTable(
			m_pnComputeSrvRootParameterIndices[paramIndex],
			m_pd3dComputeSrvRootParameterGpuDescriptorHandles[textureIndex]);
	}
}

void CTexture::BindComputeSrvToRootParameter(ID3D12GraphicsCommandList* pd3dCommandList, int rootParamIndex, int textureIndex)
{
	if (textureIndex >= m_pd3dComputeSrvGpuDescriptorHandles.size())
		return;

	if (m_pd3dComputeSrvGpuDescriptorHandles[textureIndex].ptr == 0)
		return;

	pd3dCommandList->SetComputeRootDescriptorTable(rootParamIndex, m_pd3dComputeSrvGpuDescriptorHandles[textureIndex]);
}



void CTexture::UpdateComputeUavShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pnComputeUavRootParameterIndices.size() == m_ppd3dTextures.size())
	{
		for (int i = 0; i < m_pnComputeUavRootParameterIndices.size(); ++i)
			UpdateComputeUavShaderVariable(pd3dCommandList, i, i);
	}
	else if (!m_pnComputeUavRootParameterIndices.empty())
	{
		pd3dCommandList->SetComputeRootDescriptorTable(m_pnComputeUavRootParameterIndices[0], m_pd3dComputeUavRootParameterGpuDescriptorHandles[0]);
	}
}

void CTexture::UpdateComputeUavShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int paramIndex, int textureIndex)
{
	if (m_pnComputeUavRootParameterIndices[paramIndex] != -1 && m_pd3dComputeUavRootParameterGpuDescriptorHandles[textureIndex].ptr != 0)
	{
		pd3dCommandList->SetComputeRootDescriptorTable(m_pnComputeUavRootParameterIndices[paramIndex], m_pd3dComputeUavRootParameterGpuDescriptorHandles[textureIndex]);
	}
}

void CTexture::BindComputeUavToRootParameter(ID3D12GraphicsCommandList* pd3dCommandList, int rootParamIndex, int textureIndex)
{
	if (textureIndex >= m_pd3dComputeUavGpuDescriptorHandles.size())
		return;
	if (m_pd3dComputeUavGpuDescriptorHandles[textureIndex].ptr == 0)
		return;

	pd3dCommandList->SetComputeRootDescriptorTable(rootParamIndex, m_pd3dComputeUavGpuDescriptorHandles[textureIndex]);
}

int CTexture::GetGraphicsSrvRootParameterIndex(int index) const
{
	return m_pnGraphicsSrvRootParameterIndices[index];
}

int CTexture::GetComputeSrvRootParameterIndex(int index) const
{
	return m_pnComputeSrvRootParameterIndices[index];
}

int CTexture::GetComputeUavRootParameterIndex(int index) const
{
	return m_pnComputeUavRootParameterIndices[index];
}

void CTexture::ReleaseShaderVariables()
{
}

void CTexture::ReleaseUploadBuffers()
{
	for (auto& buffer : m_ppd3dTextureUploadBuffers)
	{
		if (buffer)
			buffer->Release();
		buffer = nullptr;
	}
	m_ppd3dTextureUploadBuffers.clear();
}

void CTexture::SetSampler(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
	m_pd3dSamplerGpuDescriptorHandles[index] = handle;
}

void CTexture::LoadTextureFromDDSFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, wchar_t* filename, UINT resourceType, UINT index)
{
	Get_File_Name_From_Address(filename, m_pstrTextureName);
	m_pnResourceTypes[index] = resourceType;
	m_ppd3dTextures[index] = CreateTextureResourceFromDDSFile(device, commandList, filename, &m_ppd3dTextureUploadBuffers[index], D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12_RESOURCE_DESC texDesc = m_ppd3dTextures[index]->GetDesc();
	m_nTextureWidths[index] = static_cast<UINT>(texDesc.Width);
	m_nTextureHeights[index] = static_cast<UINT>(texDesc.Height);
}

void CTexture::LoadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, void* data, UINT elements, UINT stride, DXGI_FORMAT format, UINT index)
{
	m_pnResourceTypes[index] = RESOURCE_BUFFER;
	m_pdxgiBufferFormats[index] = format;
	m_pnBufferElements[index] = elements;

	m_ppd3dTextures[index] = CreateBufferResource(device, commandList, data, elements * stride, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, &m_ppd3dTextureUploadBuffers[index]);
}

void CTexture::CreateBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT index, void* data, UINT elements, UINT stride, DXGI_FORMAT format, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES state)
{
	m_pnResourceTypes[index] = RESOURCE_BUFFER;
	m_pdxgiBufferFormats[index] = format;
	m_pnBufferElements[index] = elements;
	m_pnBufferStrides[index] = stride;

	m_ppd3dTextures[index] = CreateBufferResource(device, commandList, data, elements * stride, heapType, D3D12_RESOURCE_FLAG_NONE, state, &m_ppd3dTextureUploadBuffers[index]);
}

void CTexture::CreateStructuredBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT index, void* data, UINT elements, UINT stride, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES state)
{
	m_pnResourceTypes[index] = RESOURCE_STRUCTURED_BUFFER;
	m_pdxgiBufferFormats[index] = DXGI_FORMAT_UNKNOWN;
	m_pnBufferElements[index] = elements;
	m_pnBufferStrides[index] = stride;

	m_ppd3dTextures[index] = CreateBufferResource(device, commandList, data, elements * stride, heapType, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, state, &m_ppd3dTextureUploadBuffers[index]);
}
ID3D12Resource* CTexture::CreateTexture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT index, UINT resourceType, UINT width, UINT height, UINT elements, UINT mips, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, D3D12_CLEAR_VALUE* clearValue)
{
	m_pnResourceTypes[index] = resourceType;
	m_pdxgiBufferFormats[index] = format;
	m_ppd3dTextures[index] = CreateTexture2DResource(device, commandList, width, height, elements, mips, format, flags, state, clearValue);
	return m_ppd3dTextures[index];
}

void CTexture::SetRootParameterIndex(int index, UINT rootParameterIndex)
{
	m_pnGraphicsSrvRootParameterIndices[index] = rootParameterIndex;
}

DXGI_FORMAT CTexture::GetBufferFormat(int index) const
{
	return m_pdxgiBufferFormats[index];
}

int CTexture::GetBufferElements(int index) const
{
	return m_pnBufferElements[index];
}

int CTexture::GetBufferStrides(int index) const
{
	return m_pnBufferStrides[index];
}

D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc(int index)
{
	ID3D12Resource* resource = GetResource(index);
	auto desc = resource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
	srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	DXGI_FORMAT originalFormat = desc.Format;
	if (originalFormat == DXGI_FORMAT_R32_TYPELESS)
		srv.Format = DXGI_FORMAT_R32_FLOAT;
	else if (originalFormat == DXGI_FORMAT_R24G8_TYPELESS)
		srv.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	else
		srv.Format = originalFormat;

	int type = GetTextureType(index);
	switch (type)
	{
	case RESOURCE_TEXTURE2D:
	case RESOURCE_TEXTURE2D_ARRAY:
		//srv.Format = desc.Format;
		srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv.Texture2D.MipLevels = -1;
		srv.Texture2D.MostDetailedMip = 0;
		srv.Texture2D.PlaneSlice = 0;
		srv.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY:
		//srv.Format = desc.Format;
		srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srv.Texture2DArray.MipLevels = -1;
		srv.Texture2DArray.MostDetailedMip = 0;
		srv.Texture2DArray.PlaneSlice = 0;
		srv.Texture2DArray.ResourceMinLODClamp = 0.0f;
		srv.Texture2DArray.FirstArraySlice = 0;
		srv.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE:
		//srv.Format = desc.Format;
		srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srv.TextureCube.MipLevels = 1;
		srv.TextureCube.MostDetailedMip = 0;
		srv.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER:
		srv.Format = m_pdxgiBufferFormats[index];
		srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv.Buffer.FirstElement = 0;
		srv.Buffer.NumElements = m_pnBufferElements[index];
		srv.Buffer.StructureByteStride = 0;
		srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	case RESOURCE_STRUCTURED_BUFFER:
		srv.Format = DXGI_FORMAT_UNKNOWN;
		srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv.Buffer.FirstElement = 0;
		srv.Buffer.NumElements = m_pnBufferElements[index];
		srv.Buffer.StructureByteStride = m_pnBufferStrides[index];
		srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return srv;
}
D3D12_UNORDERED_ACCESS_VIEW_DESC CTexture::GetUnorderedAccessViewDesc(int index)
{
	ID3D12Resource* resource = GetResource(index);
	auto desc = resource->GetDesc();
	D3D12_UNORDERED_ACCESS_VIEW_DESC uav{};

	int type = GetTextureType(index);
	switch (type) {
	case RESOURCE_TEXTURE2D:
	case RESOURCE_TEXTURE2D_ARRAY:
		uav.Format = desc.Format;
		uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uav.Texture2D.MipSlice = 0;
		uav.Texture2D.PlaneSlice = 0;
		break;
	case RESOURCE_TEXTURE2DARRAY:
		uav.Format = desc.Format;
		uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uav.Texture2DArray.MipSlice = 0;
		uav.Texture2DArray.FirstArraySlice = 0;
		uav.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		uav.Texture2DArray.PlaneSlice = 0;
		break;
	case RESOURCE_BUFFER:
		uav.Format = m_pdxgiBufferFormats[index];
		uav.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uav.Buffer.FirstElement = 0;
		uav.Buffer.NumElements = m_pnBufferElements[index];
		uav.Buffer.StructureByteStride = 0;
		uav.Buffer.CounterOffsetInBytes = 0;
		uav.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		break;
	case RESOURCE_STRUCTURED_BUFFER:
		uav.Format = DXGI_FORMAT_UNKNOWN;
		uav.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uav.Buffer.FirstElement = 0;
		uav.Buffer.NumElements = m_pnBufferElements[index];
		uav.Buffer.StructureByteStride = m_pnBufferStrides[index];

		uav.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		break;
	}
	return uav;
}


//==================================================================================
CShader* CMaterial::m_pSkinnedAnimationShader = NULL;
CShader* CMaterial::m_pStandardShader = NULL;

CMaterial::CMaterial(int nTextures)
{
	m_nTextures = nTextures;
	m_ppTextures.resize(nTextures);
	m_ppstrTextureNames.resize(nTextures);
	for (auto& name : m_ppstrTextureNames)
		name[0] = '\0';
}


CMaterial::~CMaterial()
{
	DebugOutput("\nDelete Material");
}

CMaterial::CMaterial(const CMaterial& other)
{
	m_cAlbedo = other.m_cAlbedo;
	m_fGlossiness = other.m_fGlossiness;
	m_fGlossyReflection = other.m_fGlossyReflection;


	m_pShader = other.m_pShader;
	m_nType = other.m_nType;
	m_nTextures = other.m_nTextures;
	m_Material_ID = other.m_Material_ID;
	Outline_Color_ID = other.Outline_Color_ID;
	Object_Type_ID = other.Object_Type_ID;

	m_ppstrTextureNames = other.m_ppstrTextureNames;
	m_ppTextures.resize(other.m_nTextures);
	for (int i = 0; i < other.m_nTextures; ++i)
	{
		if (other.m_ppTextures[i])
			m_ppTextures[i] = std::make_shared<CTexture>(*other.m_ppTextures[i]);
	}
}

std::shared_ptr<CMaterial> CMaterial::CloneWithSharedTextures() const
{
	auto clone = std::make_shared<CMaterial>(m_nTextures);

	clone->m_cAlbedo = m_cAlbedo;
	clone->m_fGlossiness = m_fGlossiness;
	clone->m_fGlossyReflection = m_fGlossyReflection;
	clone->m_pShader = m_pShader;
	clone->m_nType = m_nType;
	clone->m_Material_ID = m_Material_ID;
	clone->Outline_Color_ID = Outline_Color_ID;
	clone->Object_Type_ID = Object_Type_ID;

	clone->m_ppstrTextureNames = m_ppstrTextureNames;
	clone->m_ppTextures = m_ppTextures;

	return clone;
}

void CMaterial::SetShader(CShader* pShader)
{
	if (pShader)
	{
		if (pShader != m_pStandardShader && pShader != m_pSkinnedAnimationShader)
			if (m_pShader != NULL)
				m_pShader->Release();

		m_pShader = pShader;

		if (pShader != m_pStandardShader && pShader != m_pSkinnedAnimationShader)
			if (m_pShader != NULL)
				m_pShader->AddRef();
	}

}

void CMaterial::SetTexture(shared_ptr<CTexture> pTexture, UINT nTexture)
{
	m_ppTextures[nTexture] = pTexture;
}


void CMaterial::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->ReleaseUploadBuffers();
	}
}


void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature)
{
	if (!m_pStandardShader)
	{
		m_pStandardShader = new Deferred_CStandard_Shader();
		m_pStandardShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, RenderTarget_Config::RTV_FORMAT_num, RenderTarget_Config::RTV_FORMATS, RenderTarget_Config::DSV_FORMAT);
		m_pStandardShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	if (!m_pSkinnedAnimationShader)
	{
		m_pSkinnedAnimationShader = new Deferred_CSkinnedAnimationStandardShader();
		m_pSkinnedAnimationShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, RenderTarget_Config::RTV_FORMAT_num, RenderTarget_Config::RTV_FORMATS, RenderTarget_Config::DSV_FORMAT);
		m_pSkinnedAnimationShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	if (!Object_Manager::instance_shader)
	{
		Object_Manager::instance_shader = std::make_shared<Deferred_CStandard_Instance_Shader>();
		Object_Manager::instance_shader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, RenderTarget_Config::RTV_FORMAT_num, RenderTarget_Config::RTV_FORMATS, RenderTarget_Config::DSV_FORMAT);
		Object_Manager::instance_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

}

void CMaterial::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList)
{
	Material_GPU_Packet material_packet{};
	material_packet.gAlbedoColor = m_cAlbedo;
	material_packet.light_material_ID = m_Material_ID;
	material_packet.Blur_Mask_ID = Blur_Mask_ID;
	material_packet.Object_Type_ID = Object_Type_ID;
	material_packet.Outline_Color_ID = Outline_Color_ID;

	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 8, &material_packet, 16); // 16~23
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 1, &m_nType, 24);       // 24

	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i])
			m_ppTextures[i]->UpdateGraphicsSrvShaderVariables(pd3dCommandList);
	}
}

void CMaterial::Update_TextureShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i])
			m_ppTextures[i]->UpdateGraphicsSrvShaderVariables(pd3dCommandList);
	}
}

void CMaterial::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, std::vector<std::shared_ptr<CTexture>>& textures, UINT textureIndex, std::shared_ptr<CGameObject> pParent, FILE* pInFile, CShader* pShader)
{
	char pstrTextureName[64] = { '\0' };

	BYTE nStrLength = 64;
	UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrTextureName, sizeof(char), nStrLength, pInFile);
	pstrTextureName[nStrLength] = '\0';

	bool bDuplicated = false;
	if (strcmp(pstrTextureName, "null"))
	{
		SetMaterialType(nType);

		char pstrFilePath[64] = { '\0' };
		strcpy_s(pstrFilePath, 64, "Model/Textures/");

		bDuplicated = (pstrTextureName[0] == '@');
		strcpy_s(pstrFilePath + 15, 64 - 15, (bDuplicated) ? (pstrTextureName + 1) : pstrTextureName);
		strcpy_s(pstrFilePath + 15 + ((bDuplicated) ? (nStrLength - 1) : nStrLength), 64 - 15 - ((bDuplicated) ? (nStrLength - 1) : nStrLength), ".dds");

		size_t nConverted = 0;
		mbstowcs_s(&nConverted, pwstrTextureName, 64, pstrFilePath, _TRUNCATE);

#ifdef _WITH_DISPLAY_TEXTURE_NAME
		static int nTextures = 0, nRepeatedTextures = 0;
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("\nTexture Name: %d %c %s"), (pstrTextureName[0] == '@') ? nRepeatedTextures++ : nTextures++, (pstrTextureName[0] == '@') ? '@' : ' ', pwstrTextureName);
		OutputDebugString(pstrDebug);
#endif
		if (!bDuplicated)
		{
			textures[textureIndex] = std::make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
			textures[textureIndex]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pwstrTextureName, RESOURCE_TEXTURE2D, 0);
			CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, textures[textureIndex].get(), 0, nRootParameter);
		}
		else
		{
			if (pParent)
			{
				while (pParent)
				{
					if (!pParent->m_pParent) break;
					pParent = pParent->m_pParent;
				}
				std::shared_ptr<CGameObject> pRootGameObject = pParent;
				textures[textureIndex] = pRootGameObject->FindReplicatedTexture(pwstrTextureName);
			}
		}
	}
}

//==========================================================

UINT Light_Material_Manager::index = 0;
std::vector<Light_Material_Info> Light_Material_Manager::light_material_list;
bool Light_Material_Manager::reserved_update = false;
CTexture* Light_Material_Manager::material_info_buffer = NULL;

// Initialize the material manager
void Light_Material_Manager::Initialize()
{
	light_material_list.clear();
	index = 0;

	Light_Material_Info null_pixel_material;
	null_pixel_material.gEmissive = XMFLOAT4{};
	null_pixel_material.gSpecular = XMFLOAT4{};
	null_pixel_material.gMetallic = 0.0f;
	null_pixel_material.gRoughness = 0.0f;
	null_pixel_material.padding0 = 0.0f;
	null_pixel_material.padding1 = 0.0f;

	Add_Material(null_pixel_material);



}

// Add a new material and return its ID
UINT Light_Material_Manager::Add_Material(const Light_Material_Info& material)
{
	light_material_list.push_back(material);

	reserved_update = true;
	return index++;
}

// Update an existing material
void Light_Material_Manager::Update_Material_Info(UINT idx, const Light_Material_Info& material)
{
	if (idx >= light_material_list.size())
	{
		throw std::out_of_range("Light_Material_Manager::UpdateMaterial - Index out of range");
	}
	light_material_list[idx] = material;
	reserved_update = true;
}

void Light_Material_Manager::CreateStructuredBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (material_info_buffer)
	{
		material_info_buffer->Release();
		material_info_buffer = nullptr;
	}

	material_info_buffer = new CTexture(1, RESOURCE_STRUCTURED_BUFFER, 0, 1, 0, 0, 1, 0, 0);

	UINT stride = sizeof(Light_Material_Info);
	UINT count = static_cast<UINT>(light_material_list.size());

	material_info_buffer->CreateStructuredBuffer(pd3dDevice, pd3dCommandList, 0, light_material_list.data(), count, stride,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, material_info_buffer, 0, ROOT_PARAMETER_MATERIAL_REFLECTANCE_INFO_SRV_INDEX);

	reserved_update = false;
}

void Light_Material_Manager::Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (!reserved_update)
		return;

	if (material_info_buffer)
	{
		material_info_buffer->Release();
		material_info_buffer = nullptr;
	}

	CreateStructuredBuffer(pd3dDevice, pd3dCommandList);

	reserved_update = false;
}

void Light_Material_Manager::UpdateGraphicsShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (material_info_buffer)
	{
		material_info_buffer->UpdateGraphicsSrvShaderVariables(pd3dCommandList);
	}
}

// Get material info by ID
const Light_Material_Info& Light_Material_Manager::Get_Material(UINT idx)
{
	if (idx >= light_material_list.size())
	{
		throw std::out_of_range("Light_Material_Manager::GetMaterial - Index out of range");
	}
	return light_material_list[idx];
}

// Get total number of materials
size_t Light_Material_Manager::Get_Material_Count()
{
	return light_material_list.size();
}

// Release all materials
void Light_Material_Manager::Release()
{
	if (material_info_buffer)
	{
		delete material_info_buffer;
		material_info_buffer = nullptr;
	}


	light_material_list.clear();
	index = 0;
}

// Find a similar material and return its ID, otherwise return -1
int Light_Material_Manager::Find_Similar_Material(const Light_Material_Info& material, float tolerance)
{
	for (int i = 0; i < static_cast<int>(light_material_list.size()); ++i)
	{
		const Light_Material_Info& existing = light_material_list[i];

		if (fabs(existing.gRoughness - material.gRoughness) < tolerance
			&& fabs(existing.gMetallic - material.gMetallic) < tolerance
			&& Compare_XMFLOAT4(existing.gSpecular, material.gSpecular, tolerance)
			&& Compare_XMFLOAT4(existing.gEmissive, material.gEmissive, tolerance))
		{
			return i; // Found similar material
		}
	}
	return -1; // No similar material found
}
//==========================================================


void CRootMotionCallbackHandler::HandleCallback(void* pCallbackData, float fTrackPosition)
{
	float* pfData = (float*)pCallbackData;
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Data: %.2f, Position: %.2f\n"), *pfData, fTrackPosition);
	OutputDebugString(pstrDebug);
}


CAnimationSet::CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrames, int nAnimatedBones, char* pstrName)
{
	m_fLength = fLength;
	m_nFramesPerSecond = nFramesPerSecond;
	m_nKeyFrames = nKeyFrames;

	strcpy_s(m_pstrAnimationSetName, 64, pstrName);

#ifdef _WITH_ANIMATION_SRT
	m_nKeyFrameTranslations = nKeyFrames;
	m_pfKeyFrameTranslationTimes = new float[m_nKeyFrameTranslations];
	m_ppxmf3KeyFrameTranslations = new XMFLOAT3 * [m_nKeyFrameTranslations];
	for (int i = 0; i < m_nKeyFrameTranslations; i++) m_ppxmf3KeyFrameTranslations[i] = new XMFLOAT4X4[nAnimatedBones];

	m_nKeyFrameScales = nKeyFrames;
	m_pfKeyFrameScaleTimes = new float[m_nKeyFrameScales];
	m_ppxmf3KeyFrameScales = new XMFLOAT3 * [m_nKeyFrameScales];
	for (int i = 0; i < m_nKeyFrameScales; i++) m_ppxmf3KeyFrameScales[i] = new XMFLOAT4X4[nAnimatedBones];

	m_nKeyFrameRotations = nKeyFrames;
	m_pfKeyFrameRotationTimes = new float[m_nKeyFrameRotations];
	m_ppxmf4KeyFrameRotations = new XMFLOAT3 * [m_nKeyFrameRotations];
	for (int i = 0; i < m_nKeyFrameRotations; i++) m_ppxmf4KeyFrameRotations[i] = new XMFLOAT4X4[nAnimatedBones];
#else
	m_pfKeyFrameTimes = new float[nKeyFrames];
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4 * [nKeyFrames];
	for (int i = 0; i < nKeyFrames; i++) m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nAnimatedBones];
#endif
}

CAnimationSet::~CAnimationSet()
{
#ifdef _WITH_ANIMATION_SRT
	if (m_pfKeyFrameTranslationTimes) delete[] m_pfKeyFrameTranslationTimes;
	for (int j = 0; j < m_nKeyFrameTranslations; j++) if (m_ppxmf3KeyFrameTranslations[j]) delete[] m_ppxmf3KeyFrameTranslations[j];
	if (m_ppxmf3KeyFrameTranslations) delete[] m_ppxmf3KeyFrameTranslations;

	if (m_pfKeyFrameScaleTimes) delete[] m_pfKeyFrameScaleTimes;
	for (int j = 0; j < m_nKeyFrameScales; j++) if (m_ppxmf3KeyFrameScales[j]) delete[] m_ppxmf3KeyFrameScales[j];
	if (m_ppxmf3KeyFrameScales) delete[] m_ppxmf3KeyFrameScales;

	if (m_pfKeyFrameRotationTimes) delete[] m_pfKeyFrameRotationTimes;
	for (int j = 0; j < m_nKeyFrameRotations; j++) if (m_ppxmf4KeyFrameRotations[j]) delete[] m_ppxmf4KeyFrameRotations[j];
	if (m_ppxmf4KeyFrameRotations) delete[] m_ppxmf4KeyFrameRotations;
#else
	if (m_pfKeyFrameTimes)
		delete[] m_pfKeyFrameTimes;

	for (int j = 0; j < m_nKeyFrames; j++)
		if (m_ppxmf4x4KeyFrameTransforms[j])
			delete[] m_ppxmf4x4KeyFrameTransforms[j];

	if (m_ppxmf4x4KeyFrameTransforms)
		delete[] m_ppxmf4x4KeyFrameTransforms;
#endif

	DebugOutput("\nDelete AnimationSet: ", m_pstrAnimationSetName);
}

XMFLOAT4X4 CAnimationSet::GetSRT(int nBone, float fPosition)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();
#ifdef _WITH_ANIMATION_SRT
	XMVECTOR S, R, T;
	for (int i = 0; i < (m_nKeyFrameTranslations - 1); i++)
	{
		if ((m_pfKeyFrameTranslationTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameTranslationTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameTranslationTimes[i]) / (m_pfKeyFrameTranslationTimes[i + 1] - m_pfKeyFrameTranslationTimes[i]);
			T = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i + 1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameScales - 1); i++)
	{
		if ((m_pfKeyFrameScaleTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameScaleTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameScaleTimes[i]) / (m_pfKeyFrameScaleTimes[i + 1] - m_pfKeyFrameScaleTimes[i]);
			S = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameScales[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameScales[i + 1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameRotations - 1); i++)
	{
		if ((m_pfKeyFrameRotationTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameRotationTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameRotationTimes[i]) / (m_pfKeyFrameRotationTimes[i + 1] - m_pfKeyFrameRotationTimes[i]);
			R = XMQuaternionSlerp(XMQuaternionConjugate(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i][nBone])), XMQuaternionConjugate(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i + 1][nBone])), t);
			break;
		}
	}

	XMStoreFloat4x4(&xmf4x4Transform, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
#else   

	if (fPosition >= m_pfKeyFrameTimes[m_nKeyFrames - 1])
		return m_ppxmf4x4KeyFrameTransforms[m_nKeyFrames - 1][nBone];

	for (int i = 0; i < (m_nKeyFrames - 1); i++)
	{
		if ((m_pfKeyFrameTimes[i] <= fPosition) && (fPosition < m_pfKeyFrameTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameTimes[i]) / (m_pfKeyFrameTimes[i + 1] - m_pfKeyFrameTimes[i]);
			xmf4x4Transform = Matrix4x4::Interpolate(m_ppxmf4x4KeyFrameTransforms[i][nBone], m_ppxmf4x4KeyFrameTransforms[i + 1][nBone], t);
			break;
		}
	}

#endif
	return(xmf4x4Transform);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationSets::CAnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	//m_pAnimationSet_list = new CAnimationSet * [nAnimationSets];
	m_pAnimationSet_list.resize(nAnimationSets);
}

CAnimationSets::~CAnimationSets()
{
	/*for (int i = 0; i < m_nAnimationSets; i++)
		if (m_pAnimationSet_list[i])
			delete m_pAnimationSet_list[i];

	if (m_pAnimationSet_list)
		delete[] m_pAnimationSet_list;*/
	m_pAnimationSet_list.clear();

	//	if (m_ppBoneFrameCaches) delete[] m_ppBoneFrameCaches;
}

void CAnimationSets::Bone_Info()
{
	for (int i = 0; i < m_nBoneFrames; ++i)
	{
		if (strcmp(m_ppBoneFrameCaches[i]->m_pstrFrameName, "Mesh") == 0)
			continue;

		TCHAR tFrameName[64];
		MultiByteToWideChar(CP_ACP, 0, m_ppBoneFrameCaches[i]->m_pstrFrameName, -1, tFrameName, 64);


		TCHAR pstrDebug[256] = { 0 };

		_stprintf_s(pstrDebug, 256, _T("----------------------------------------------- [Bone: %s]\n"), tFrameName);
		OutputDebugString(pstrDebug);

		m_ppBoneFrameCaches[i]->Obj_Info();

		_stprintf_s(pstrDebug, 2, _T("\n"));
		OutputDebugString(pstrDebug);
	}
}

std::string CAnimationSets::GetBoneName(int index)
{
	if (index < 0 || index >= m_nBoneFrames) return "";

	if (m_ppBoneFrameCaches[index] && m_ppBoneFrameCaches[index]->m_pstrFrameName)
	{
		return std::string(m_ppBoneFrameCaches[index]->m_pstrFrameName);
	}

	return "";
}

void CAnimationSets::ClassifyBones()
{
	/*for (int i = 0; i < m_nBoneFrames; i++)
	{
		std::string boneName = m_ppBoneFrameCaches[i]->GetBoneName();

		if (boneName.find("Spine") != std::string::npos ||
			boneName.find("Chest") != std::string::npos ||
			boneName.find("Neck") != std::string::npos)
		{
			m_vecUpperBodyBoneIndices.push_back(i);
		}
		else if (boneName.find("Pelvis") != std::string::npos ||
			boneName.find("Thigh") != std::string::npos ||
			boneName.find("Leg") != std::string::npos)
		{
			m_vecLowerBodyBoneIndices.push_back(i);
		}
	}*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CAnimationTrack::CAnimationTrack()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.0f, 3.0f);

	m_fPosition = dist(gen);
}

CAnimationTrack::~CAnimationTrack()
{
	if (m_pCallbackKeys) delete[] m_pCallbackKeys;
	if (m_pAnimationCallbackHandler) delete m_pAnimationCallbackHandler;
}

void CAnimationTrack::SetCallbackKeys(int nCallbackKeys)
{
	m_nCallbackKeys = nCallbackKeys;
	m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationTrack::SetCallbackKey(int nKeyIndex, float fKeyTime, void* pData)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void CAnimationTrack::SetAnimationCallbackHandler(CAnimationCallbackHandler* pCallbackHandler)
{
	m_pAnimationCallbackHandler = pCallbackHandler;
}

void CAnimationTrack::HandleCallback()
{
	if (m_pAnimationCallbackHandler)
	{
		for (int i = 0; i < m_nCallbackKeys; i++)
		{
			if (::IsEqual(m_pCallbackKeys[i].m_fTime, m_fPosition, ANIMATION_CALLBACK_EPSILON))
			{
				if (m_pCallbackKeys[i].m_pCallbackData)
					m_pAnimationCallbackHandler->HandleCallback(m_pCallbackKeys[i].m_pCallbackData, m_fPosition);
				break;
			}
		}
	}
}

float CAnimationTrack::UpdatePosition(float fTrackPosition, float fElapsedTime, float fAnimationLength)
{
	float fTrackElapsedTime = fElapsedTime * m_fSpeed;
	switch (m_nType)
	{
	case ANIMATION_TYPE_LOOP:
	{
		if (m_fPosition < 0.0f)
			m_fPosition = 0.0f;
		else
		{
			m_fPosition = fTrackPosition + fTrackElapsedTime;
			if (m_fPosition > fAnimationLength)
			{
				//m_fPosition = -ANIMATION_CALLBACK_EPSILON;
				m_fPosition = 0;
				return(fAnimationLength);
			}
		}
		//			m_fPosition = fmod(fTrackPosition, m_pfKeyFrameTimes[m_nKeyFrames-1]); 
		//			m_fPosition = fTrackPosition - int(fTrackPosition / m_pfKeyFrameTimes[m_nKeyFrames-1]) * m_pfKeyFrameTimes[m_nKeyFrames-1];
		//			m_fPosition = fmod(fTrackPosition, m_fLength); //if (m_fPosition < 0) m_fPosition += m_fLength;
		//			m_fPosition = fTrackPosition - int(fTrackPosition / m_fLength) * m_fLength;
		break;
	}
	case ANIMATION_TYPE_ONCE:
		if (m_fPosition < 0.0f)
			m_fPosition = 0.0f;
		else {
			if (!m_bFinished) {
				m_fPosition = fTrackPosition + fTrackElapsedTime;
				if (m_fPosition > fAnimationLength) {
					m_fPosition = fAnimationLength;
					//m_fPosition = -ANIMATION_CALLBACK_EPSILON;
					m_bFinished = true;
					return(fAnimationLength);
				}
			}
		}
		break;
	case ANIMATION_TYPE_PINGPONG:
		break;
	}

	return(m_fPosition);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationController::CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel)
{
	m_nAnimationTracks = nAnimationTracks;
	m_pAnimationTracks = new CAnimationTrack[nAnimationTracks];

	m_pAnimationSets = pModel->m_pAnimationSets;
	m_pAnimationSets->AddRef();

	m_pModelRootObject = pModel->m_pModelRootObject;

	m_nSkinnedMeshes = pModel->m_nSkinnedMeshes;
	m_ppSkinnedMeshes.resize(m_nSkinnedMeshes);
	//m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];

	for (int i = 0; i < m_nSkinnedMeshes; i++)
		m_ppSkinnedMeshes[i] = pModel->m_ppSkinnedMeshes[i];

	m_ppd3dcbSkinningBoneTransforms = new ID3D12Resource * [m_nSkinnedMeshes]();
	m_ppcbxmf4x4MappedSkinningBoneTransforms = new XMFLOAT4X4 * [m_nSkinnedMeshes]();

	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255);
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Map(0, NULL, (void**)&m_ppcbxmf4x4MappedSkinningBoneTransforms[i]);
	}
}

CAnimationController::~CAnimationController()
{
	if (m_pAnimationTracks)
		delete[] m_pAnimationTracks;

	if (m_ppd3dcbSkinningBoneTransforms) {
		for (int i = 0; i < m_nSkinnedMeshes; i++)
		{
			if (m_ppd3dcbSkinningBoneTransforms[i]) {
				m_ppd3dcbSkinningBoneTransforms[i]->Unmap(0, NULL);
				//			m_ppd3dcbSkinningBoneTransforms[i]->Release();
				m_ppd3dcbSkinningBoneTransforms[i] = NULL;
			}
		}
		delete[] m_ppd3dcbSkinningBoneTransforms;
		m_ppd3dcbSkinningBoneTransforms = NULL;
	}

	if (m_ppcbxmf4x4MappedSkinningBoneTransforms)
		delete[] m_ppcbxmf4x4MappedSkinningBoneTransforms;

	if (m_pAnimationSets)
		m_pAnimationSets->Release();

	m_ppSkinnedMeshes.clear();
	//if (m_ppSkinnedMeshes)
	//	delete[] m_ppSkinnedMeshes;
}

void CAnimationController::SetCallbackKeys(int nAnimationTrack, int nCallbackKeys)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetCallbackKeys(nCallbackKeys);
}

void CAnimationController::SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fKeyTime, void* pData)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetCallbackKey(nKeyIndex, fKeyTime, pData);
}

void CAnimationController::SetAnimationCallbackHandler(int nAnimationTrack, CAnimationCallbackHandler* pCallbackHandler)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetAnimationCallbackHandler(pCallbackHandler);
}

void CAnimationController::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pAnimationTracks)
		m_pAnimationTracks[nAnimationTrack].m_nAnimationSet = nAnimationSet;
}

void CAnimationController::SetTrackEnable(int nAnimationTrack, bool bEnable)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetEnable(bEnable);
}

void CAnimationController::SetTrackPosition(int nAnimationTrack, float fPosition)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetPosition(fPosition);
}

void CAnimationController::SetTrackSpeed(int nAnimationTrack, float fSpeed)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
}

void CAnimationController::SetTrackWeight(int nAnimationTrack, float fWeight)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetWeight(fWeight);
}

void CAnimationController::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppSkinnedMeshes[i]->m_pd3dcbSkinningBoneTransforms = m_ppd3dcbSkinningBoneTransforms[i];
		m_ppSkinnedMeshes[i]->m_pcbxmf4x4MappedSkinningBoneTransforms = m_ppcbxmf4x4MappedSkinningBoneTransforms[i];
	}
}

XMFLOAT3 Lerp(XMFLOAT3 a, XMFLOAT3 b, float t) {
	return XMFLOAT3(
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t,
		a.z + (b.z - a.z) * t
	);
}

XMFLOAT4 Slerp(XMFLOAT4 q1, XMFLOAT4 q2, float t) {
	XMVECTOR v1 = XMLoadFloat4(&q1);
	XMVECTOR v2 = XMLoadFloat4(&q2);
	XMVECTOR result = XMQuaternionSlerp(v1, v2, t);

	XMFLOAT4 out;
	XMStoreFloat4(&out, result);
	return out;
}

XMFLOAT3 GetTranslation(const XMFLOAT4X4& matrix) {
	return XMFLOAT3(matrix._41, matrix._42, matrix._43);
}

XMFLOAT4 GetRotation(const XMFLOAT4X4& matrix) {
	XMVECTOR scale, rotation, translation;
	XMMATRIX mat = XMLoadFloat4x4(&matrix);
	XMMatrixDecompose(&scale, &rotation, &translation, mat);

	XMFLOAT4 rot;
	XMStoreFloat4(&rot, rotation);
	return rot;
}

XMFLOAT3 GetScale(const XMFLOAT4X4& matrix) {
	XMVECTOR scale, rotation, translation;
	XMMATRIX mat = XMLoadFloat4x4(&matrix);
	XMMatrixDecompose(&scale, &rotation, &translation, mat);

	XMFLOAT3 scl;
	XMStoreFloat3(&scl, scale);
	return scl;
}

XMFLOAT4X4 ComposeTransform(XMFLOAT3 pos, XMFLOAT4 rot, XMFLOAT3 scale) {
	XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&rot));
	XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);

	XMMATRIX finalMatrix = S * R * T;
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, finalMatrix);
	return result;
}



void CAnimationController::AdvanceTime(float fTimeElapsed, CGameObject* pRootGameObject)
{
	m_fTime += fTimeElapsed;

	if (m_pAnimationTracks)
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
		{
			m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = Matrix4x4::Zero();
		}

		float totalWeight = 0.0f;
		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_fWeight > ANIMATION_CALLBACK_EPSILON)
			{
				totalWeight += m_pAnimationTracks[k].m_fWeight;
			}
		}
		if (totalWeight == 0.0f) return;


		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_fWeight > ANIMATION_CALLBACK_EPSILON)
			{
				//CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet];
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet].get();

				float fPosition = m_pAnimationTracks[k].UpdatePosition(m_pAnimationTracks[k].m_fPosition, fTimeElapsed, pAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent;
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);

					float normalizedWeight = m_pAnimationTracks[k].m_fWeight / totalWeight;
					XMFLOAT4X4 blendedTransform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, normalizedWeight));

					//if (pRootGameObject->Object_type == OBJECT_TPYE_MAIN_PLAYER || pRootGameObject->Object_type == OBJECT_TPYE_PLAYER) {
					if (pRootGameObject->HasType(EObjectType::MainPlayer | EObjectType::Player)) {
						if (j == RootIndex)
						{
							if (!m_pAnimationTracks[k].m_bFinished && GetUpdateHipsTracks().contains(k)) {
								HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);

							}

							blendedTransform._41 = 0.0f;
							//blendedTransform._42 = 0.8762761f;
							blendedTransform._43 = 0.0f;

						}
					}
					else if (pRootGameObject->HasType(EObjectType::Monster)) {
						if (j == RootIndex) {
							if (!m_pAnimationTracks[k].m_bFinished && pRootGameObject->RootMotionTrackSet.contains(k)) {
								HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);
							}

							blendedTransform._41 = 0.0f;
							//blendedTransform._42 = 0.0f;
							blendedTransform._43 = 0.0f;
						}

					}

					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = blendedTransform;
				}

				m_pAnimationTracks[k].HandleCallback();
			}
			if (m_pAnimationTracks[k].m_fWeight >= 1.0f)
				break;
		}

		pRootGameObject->UpdateTransform(NULL);

		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}

void CAnimationController::ApplyCurrentAnimationPose(CGameObject* pRootGameObject)
{
	if (!m_pAnimationTracks || !m_pAnimationSets) return;

	for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
	{
		m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = Matrix4x4::Zero();
	}

	float totalWeight = 0.0f;
	for (int k = 0; k < m_nAnimationTracks; k++)
	{
		if (m_pAnimationTracks[k].m_fWeight > ANIMATION_CALLBACK_EPSILON)
		{
			totalWeight += m_pAnimationTracks[k].m_fWeight;
		}
	}

	for (int k = 0; k < m_nAnimationTracks; k++)
	{
		if (m_pAnimationTracks[k].m_fWeight > ANIMATION_CALLBACK_EPSILON)
		{
			//CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet];
			CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet].get();

			float fPosition = m_pAnimationTracks[k].m_fPosition;

			for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
			{
				XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent;
				XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);

				float normalizedWeight = m_pAnimationTracks[k].m_fWeight / totalWeight;
				XMFLOAT4X4 blendedTransform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, normalizedWeight));

				//if (pRootGameObject->Object_type == OBJECT_TPYE_MAIN_PLAYER || pRootGameObject->Object_type == OBJECT_TPYE_PLAYER) {
				if (pRootGameObject->HasType(EObjectType::MainPlayer | EObjectType::Player)) {
					if (j == RootIndex)
					{
						if (!m_pAnimationTracks[k].m_bFinished && GetUpdateHipsTracks().contains(k)) {
							HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);
						}
						blendedTransform._41 = 0.0f;
						//blendedTransform._42 = 0.8762761f;
						blendedTransform._43 = 0.0f;

					}
				}
				else if (pRootGameObject->HasType(EObjectType::Monster)) {
					if (j == RootIndex) {
						if (!m_pAnimationTracks[k].m_bFinished && pRootGameObject->RootMotionTrackSet.contains(k)) {
							HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);
						}

						blendedTransform._41 = 0.0f;
						//blendedTransform._42 = 0.0f;
						blendedTransform._43 = 0.0f;
					}

				}

				m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = blendedTransform;
			}
			m_pAnimationTracks[k].HandleCallback();
		}
		if (m_pAnimationTracks[k].m_fWeight >= 1.0f)
			break;
	}

	pRootGameObject->UpdateTransform(nullptr);
}

void CAnimationController::ServerAdvanceTime(const ServerSyncData& syncData)
{

}

std::vector<Animation_Sync> CAnimationController::MakeSyncData()
{
	std::vector<Animation_Sync> data;
	for (int i = 0; i < m_nAnimationTracks; ++i) {
		if (m_pAnimationTracks[i].m_fWeight > ANIMATION_CALLBACK_EPSILON) {
			Animation_Sync t;
			t.track_index = i;
			t.track_position = m_pAnimationTracks[i].m_fPosition;
			t.weight = m_pAnimationTracks[i].m_fWeight;
			data.push_back(t);
		}
	}
	return data;
}

void CAnimationController::ResetWeight()
{
	for (int i = 0; i < m_nAnimationTracks; ++i) {
		m_pAnimationTracks[i].m_fWeight = 0.0f;
	}
}

bool IsUpperBodyBone(const std::string& boneName)
{
	static const std::unordered_set<std::string> upperBodyBones =
	{
		"Hips", "Spine_01", "Spine_02", "Spine_03", "Neck", "Head", "Eyes", "Eyebrows", "Eyebrows", "Clavicle_L",
		"Shoulder_L", "Elbow_L", "Hand_L", /* tumb */ "Clavicle_R", "Shoulder_R", "Elbow_R", "Hand_R",  /* tumb */"SM_Wep_Sabre_01",

	};
	return upperBodyBones.find(boneName) != upperBodyBones.end();
}

bool IsLowerBodyBone(const std::string& boneName)
{
	static const std::unordered_set<std::string> lowerBodyBones =
	{
		"UpperLeg_R", "LowerLeg_R", "Ankle_R", "Ball_R", "Toes_R",
		"UpperLeg_L", "LowerLeg_L", "Ankle_L", "Ball_L", "Toes_L"
	};
	return lowerBodyBones.find(boneName) != lowerBodyBones.end();
}

void CAnimationController::AdvanceTime2(float fTimeElapsed, CGameObject* pRootGameObject)
{
	m_fTime += fTimeElapsed;

	if (m_pAnimationTracks)
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
		{
			m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = Matrix4x4::Zero();
		}

		float totalWeightUpper = 0.0f;
		float totalWeightLower = 0.0f;
		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable)
			{
				if (m_pAnimationTracks[k].m_nAnimationSet == 2)
					totalWeightUpper += m_pAnimationTracks[k].m_fWeight;
				else if (m_pAnimationTracks[k].m_nAnimationSet == 1)
					totalWeightLower += m_pAnimationTracks[k].m_fWeight;
			}
		}

		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable)
			{
				//CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet];
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet].get();

				float fPosition = m_pAnimationTracks[k].UpdatePosition(m_pAnimationTracks[k].m_fPosition, fTimeElapsed, pAnimationSet->m_fLength);

				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					const std::string& boneName = m_pAnimationSets->GetBoneName(j);

					bool isLowerBody = IsLowerBodyBone(boneName);

					float totalWeight = 0.0f;

					if (isLowerBody)
					{
						totalWeight = totalWeightLower;
					}
					else {
						totalWeight = totalWeightUpper;
					}

					if ((!isLowerBody && m_pAnimationTracks[k].m_nAnimationSet == 2) ||
						(isLowerBody && m_pAnimationTracks[k].m_nAnimationSet == 1))
					{
						XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent;
						XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);

						float normalizedWeight = m_pAnimationTracks[k].m_fWeight / totalWeight;
						XMFLOAT4X4 blendedTransform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, normalizedWeight));

						if (boneName == "Hips")
						{
							blendedTransform._41 = 0.0f;
							blendedTransform._42 = 0.8762761f;
							blendedTransform._43 = 0.0f;
						}

						m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = blendedTransform;
					}
				}

				m_pAnimationTracks[k].HandleCallback();
			}
		}

		pRootGameObject->UpdateTransform(NULL);

		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}

void CAnimationController::Bone_Info()
{
	m_pAnimationSets->Bone_Info();

}

//*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CLoadedModelInfo::~CLoadedModelInfo()
{
	m_ppSkinnedMeshes.clear();
	//if (m_ppSkinnedMeshes)
	//	delete[] m_ppSkinnedMeshes;
}

void CLoadedModelInfo::PrepareSkinning()
{
	int nSkinnedMesh = 0;
	m_ppSkinnedMeshes.clear();
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes);
//	m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
//	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nSkinnedMesh);

	for (int i = 0; i < m_nSkinnedMeshes; i++)
		m_ppSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject(const std::string_view& name)
{
	Set_Name(name);
	m_xmf4x4Parent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 0.0f;
	XMStoreFloat4x4(&WeaponMatrix, XMMatrixIdentity());
}

CGameObject::CGameObject(int nMaterials, const std::string_view& name) : CGameObject(name)
{
	Material_list.resize(nMaterials);
	for (std::shared_ptr<CMaterial> material_ptr : Material_list)
	{
		material_ptr.reset();
	}

}

CGameObject::~CGameObject()
{
	for (std::shared_ptr<CMaterial> material_ptr : Material_list)
	{
		material_ptr.reset();
	}
	Material_list.clear();
	Material_list.shrink_to_fit();

	int k = m_pSkinnedAnimationController.use_count();
	if (k == 1) {
		m_pSkinnedAnimationController.reset();
	}

	DebugOutput("\nDelete GameObject: ", m_pstrFrameName);
}

CGameObject::CGameObject(const CGameObject& other)
{
	m_pParent = other.m_pParent;
	m_xmf4x4Parent = other.m_xmf4x4Parent;
	m_xmf4x4World = other.m_xmf4x4World;
	m_xmf3RotationAxis = other.m_xmf3RotationAxis;
	m_fRotationSpeed = other.m_fRotationSpeed;
	Active = other.Active;


	std::memcpy(m_pstrFrameName, other.m_pstrFrameName, sizeof(m_pstrFrameName));


	m_pChild = (other.m_pChild) ? std::make_shared<CGameObject>(*other.m_pChild) : nullptr;
	m_pSibling = (other.m_pSibling) ? std::make_shared<CGameObject>(*other.m_pSibling) : nullptr;

	if (!other.Material_list.empty())
	{
		Material_list.clear();
		Material_list.reserve(other.Material_list.size());

		for (const auto& material : other.Material_list)
		{
			if (material)
				Material_list.push_back(std::make_shared<CMaterial>(*material));
			else
				Material_list.push_back(nullptr);
		}
	}


	if (other.m_pMesh != nullptr)
		m_pMesh = other.m_pMesh;


	if (other.m_pSkinnedAnimationController != nullptr)
		m_pSkinnedAnimationController = std::make_shared <CAnimationController>(*other.m_pSkinnedAnimationController);

}

CGameObject& CGameObject::operator=(const CGameObject& other)
{
	if (this == &other) return *this;


	m_pParent = other.m_pParent;
	m_xmf4x4Parent = other.m_xmf4x4Parent;
	m_xmf4x4World = other.m_xmf4x4World;
	m_xmf3RotationAxis = other.m_xmf3RotationAxis;
	m_fRotationSpeed = other.m_fRotationSpeed;
	Active = other.Active;

	std::memcpy(m_pstrFrameName, other.m_pstrFrameName, sizeof(m_pstrFrameName));


	if (other.m_pChild)
	{
		if (m_pChild)
			m_pChild.reset();
		m_pChild = std::make_shared<CGameObject>(*other.m_pChild);
	}
	else
		m_pChild = nullptr;


	if (other.m_pSibling)
	{
		if (m_pSibling)
			m_pSibling.reset();
		m_pSibling = std::make_shared<CGameObject>(*other.m_pSibling);
	}
	else
		m_pSibling = nullptr;


	if (!other.Material_list.empty())
	{
		Material_list.clear();
		Material_list.reserve(other.Material_list.size());

		for (const auto& material : other.Material_list)
		{
			if (material)
				Material_list.push_back(std::make_shared<CMaterial>(*material));
			else
				Material_list.push_back(nullptr);  // nullptr을 유지		
		}
	}


	if (other.m_pMesh != nullptr)
	{
		m_pMesh = other.m_pMesh;
	}

	if (other.m_pSkinnedAnimationController != nullptr)
	{
		if (m_pSkinnedAnimationController != nullptr)
			m_pSkinnedAnimationController.reset();
		m_pSkinnedAnimationController = std::make_shared <CAnimationController>(*other.m_pSkinnedAnimationController);
	}

	return *this;
}

std::shared_ptr<CGameObject> CGameObject::Clone(bool withHierarchy)
{
	std::shared_ptr<CGameObject> clone = std::make_shared<CGameObject>();

	clone->m_xmf4x4Parent = this->m_xmf4x4Parent;
	clone->m_xmf4x4World = this->m_xmf4x4World;
	clone->m_xmf3RotationAxis = this->m_xmf3RotationAxis;
	clone->m_fRotationSpeed = this->m_fRotationSpeed;
	clone->Active = this->Active;
	std::memcpy(clone->m_pstrFrameName, this->m_pstrFrameName, sizeof(this->m_pstrFrameName));

	if (this->m_pMesh)
		clone->m_pMesh = m_pMesh;

	for (const auto& material : this->Material_list)
	{
		if (material)
			clone->Material_list.push_back(std::make_shared<CMaterial>(*material));
		else
			clone->Material_list.push_back(nullptr);
	}

	if (this->m_pSkinnedAnimationController)
		clone->m_pSkinnedAnimationController = std::make_shared<CAnimationController>(*this->m_pSkinnedAnimationController);

	if (withHierarchy && this->m_pChild)
		clone->m_pChild = this->m_pChild->Clone(true);

	if (withHierarchy && this->m_pSibling)
		clone->m_pSibling = this->m_pSibling->Clone(true);

	return clone;
}

std::shared_ptr<CGameObject> CGameObject::GetWeapon(bool withHierarchy)
{
	auto clone = std::make_shared<CGameObject>(*this);

	clone->m_pParent = nullptr;
	clone->m_pChild = nullptr;
	clone->m_pSibling = nullptr;

	if (withHierarchy && this->m_pChild)
	{
		clone->m_pChild = this->m_pChild->Clone(true);
		clone->m_pChild->m_pParent = clone;

		auto srcSibling = this->m_pChild->m_pSibling;
		auto dstSibling = clone->m_pChild;
		while (srcSibling)
		{
			dstSibling->m_pSibling = srcSibling->Clone(true);
			dstSibling->m_pSibling->m_pParent = clone;

			dstSibling = dstSibling->m_pSibling;
			srcSibling = srcSibling->m_pSibling;
		}
	}

	return clone;
}

std::shared_ptr<CGameObject> CGameObject::Make_Instance(std::shared_ptr<CGameObject> modelRoot, bool withHierarchy)
{
	std::shared_ptr<CGameObject> instance = std::make_shared<CGameObject>();

	// Copy basic transform and state properties
	instance->Set_Name(modelRoot->m_pstrFrameName);
	instance->m_xmf4x4Parent = modelRoot->m_xmf4x4Parent;
	instance->m_xmf4x4World = modelRoot->m_xmf4x4World;
	instance->m_xmf3RotationAxis = modelRoot->m_xmf3RotationAxis;
	instance->m_fRotationSpeed = modelRoot->m_fRotationSpeed;
	instance->Active = modelRoot->Active;

	// Share resource references (not deep copy)
	instance->m_pMesh = modelRoot->m_pMesh;
	instance->m_pSkinnedAnimationController = modelRoot->m_pSkinnedAnimationController;

	// Clone materials: share textures but copy values like color, roughness, etc.
	instance->Material_list.clear();
	for (const auto& mat : modelRoot->Material_list)
	{
		if (mat)
			instance->Material_list.push_back(mat->CloneWithSharedTextures()); // Custom shallow clone
		else
			instance->Material_list.push_back(nullptr);
	}

	// Clone hierarchy recursively if required
	if (withHierarchy && modelRoot->m_pChild)
	{
		instance->m_pChild = Make_Instance(modelRoot->m_pChild, true);
		if (instance->m_pChild)
			instance->m_pChild->m_pParent = instance; // Set proper parent linkage
	}

	if (withHierarchy && modelRoot->m_pSibling)
	{
		instance->m_pSibling = Make_Instance(modelRoot->m_pSibling, true);
		if (instance->m_pSibling)
			instance->m_pSibling->m_pParent = instance->m_pParent; // Sibling shares the same parent
	}

	return instance;
}


std::shared_ptr<CGameObject> CGameObject::Get_Child()
{
	if (m_pChild != nullptr)
		return m_pChild;
	else
		return nullptr;
}

std::shared_ptr<CGameObject> CGameObject::Get_Sibling()
{
	if (m_pSibling != nullptr)
		return m_pSibling;
	else
		return nullptr;
}


void CGameObject::Obj_Info(int depth)
{
	if (strcmp(m_pstrFrameName, "Mesh") == 0)
		return;

	TCHAR indent[128] = _T("");
	for (int i = 0; i < depth; ++i)
		_tcscat_s(indent, _T("  "));

	TCHAR tFrameName[64];
	MultiByteToWideChar(CP_ACP, 0, m_pstrFrameName, -1, tFrameName, 64);

	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s%s\n"), indent, tFrameName);

	OutputDebugString(pstrDebug);


	if (m_pChild)
	{
		m_pChild->Obj_Info(depth + 1);
	}

	if (m_pSibling)
	{
		m_pSibling->Obj_Info(depth);
	}
}

void CGameObject::Set_Name(std::string_view name)
{
	std::strncpy(m_pstrFrameName, name.data(), sizeof(m_pstrFrameName) - 1);
	m_pstrFrameName[sizeof(m_pstrFrameName) - 1] = '\0';
}


void CGameObject::Set_Child(std::shared_ptr<CGameObject> pChild)
{
	if (!pChild) return;

	pChild->m_pParent = shared_from_this();

	if (!m_pChild)
	{
		m_pChild = pChild;
	}
	else
	{
		std::shared_ptr<CGameObject> current = m_pChild;
		while (current->m_pSibling)
		{
			current = current->m_pSibling;
		}
		current->m_pSibling = pChild;
	}
}


void CGameObject::Set_Active(bool active, bool IsRoot)
{
	Active = active;

	if (m_pChild != NULL)
		m_pChild->Set_Active(active, false);

	if (!IsRoot && m_pSibling != NULL)
		m_pSibling->Set_Active(active, false);
}

void CGameObject::SetMesh(std::shared_ptr<CMesh> pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetShader(CShader* pShader)
{

	std::shared_ptr<CMaterial> material_ptr = std::make_shared<CMaterial>(0);
	material_ptr->SetShader(pShader);
	Material_list.push_back(material_ptr);

}

void CGameObject::SetShader(int nMaterial, CShader* pShader)
{
	if (Material_list.size() > nMaterial)
		Material_list[nMaterial]->SetShader(pShader);
}

void CGameObject::SetMaterial(int nMaterial, CMaterial* pMaterial)
{
	std::shared_ptr<CMaterial> material_ptr(pMaterial);
	Material_list[nMaterial] = material_ptr;
}


void CGameObject::SetBlurMask(bool value)
{
	if (Material_list.size())
	{
		for (std::shared_ptr<CMaterial> material_ptr : Material_list)
			material_ptr->Blur_Mask_ID = value;
	}

	if (m_pSibling)
		m_pSibling->SetBlurMask(value);

	if (m_pChild)
		m_pChild->SetBlurMask(value);
}


void CGameObject::SetOutlineColor(int id)
{
	if (Material_list.size())
	{
		for (std::shared_ptr<CMaterial> material_ptr : Material_list)
			material_ptr->Outline_Color_ID = id;
	}

	if (m_pSibling)
		m_pSibling->SetOutlineColor(id);

	if (m_pChild)
		m_pChild->SetOutlineColor(id);
}

void CGameObject::SetObject_Type_ID(int id)
{
	if (Material_list.size())
	{
		for (std::shared_ptr<CMaterial> material_ptr : Material_list)
			material_ptr->Object_Type_ID = id;
	}

	if (m_pSibling)
		m_pSibling->SetObject_Type_ID(id);

	if (m_pChild)
		m_pChild->SetObject_Type_ID(id);
}


bool CGameObject::GetBlurMask() 
{
	if (Material_list.size())
	{
		return Material_list[0]->Blur_Mask_ID;
	}

	if (m_pSibling)
	{
		bool sibling_value = m_pSibling->GetBlurMask();
		if (sibling_value) return sibling_value;
	}

	if (m_pChild)
	{
		bool child_value = m_pChild->GetBlurMask();
		if (child_value) return child_value;
	}

	return false;
}

int CGameObject::GetOutlineColor() 
{
	if (Material_list.size())
	{
		if (Material_list[0]->Outline_Color_ID != -1)
		{
			return Material_list[0]->Outline_Color_ID;
		}
	}

	if (m_pSibling)
	{
		int sibling_id = m_pSibling->GetOutlineColor();
		if (sibling_id != -1)
		{
			return sibling_id;
		}
	}

	if (m_pChild)
	{
		int child_id = m_pChild->GetOutlineColor();
		if (child_id != -1)
		{
			return child_id;
		}
	}

	return -1;
}

int CGameObject::GetObject_Type_ID() 
{
	if (Material_list.size())
	{
		if (Material_list[0]->Object_Type_ID != -1)
		{
			return Material_list[0]->Object_Type_ID;
		}
	}

	if (m_pSibling)
	{
		int sibling_id = m_pSibling->GetObject_Type_ID();
		if (sibling_id != -1)
		{
			return sibling_id;
		}
	}

	if (m_pChild)
	{
		int child_id = m_pChild->GetObject_Type_ID();
		if (child_id != -1)
		{
			return child_id;
		}
	}

	return -1;
}



void CGameObject::Set_Color_Blending(XMFLOAT3& blending_color, float blending_value)
{
	blending_value = std::clamp(blending_value, 0.0f, 1.0f);
	
	Blending_value = blending_value;
	Blending_color = blending_color;

	if (m_pSibling)
		m_pSibling->Set_Color_Blending(blending_color, blending_value);

	if (m_pChild)
		m_pChild->Set_Color_Blending(blending_color, blending_value);
}

void CGameObject::Update_Color_Blending(float update_bleeding_value)
{
	Blending_value += update_bleeding_value;
	Blending_value = std::clamp(Blending_value, 0.0f, 1.0f);

	if (m_pSibling)
		m_pSibling->Update_Color_Blending(update_bleeding_value);

	if (m_pChild)
		m_pChild->Update_Color_Blending(update_bleeding_value);
}


void CGameObject::FindAndSetSkinnedMesh(std::vector<std::shared_ptr<CSkinnedMesh>>& outSkinnedMeshes)
{
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) {
		auto pSkinned = std::dynamic_pointer_cast<CSkinnedMesh>(m_pMesh);
		if (pSkinned) {
			outSkinnedMeshes.push_back(pSkinned);
		}
	}

	if (m_pSibling)
		m_pSibling->FindAndSetSkinnedMesh(outSkinnedMeshes);

	if (m_pChild)
		m_pChild->FindAndSetSkinnedMesh(outSkinnedMeshes);
}



std::shared_ptr<CGameObject> CGameObject::FindFrame(const char* pstrFrameName)
{
	if (m_pstrFrameName && strcmp(m_pstrFrameName, pstrFrameName) == 0)
		return shared_from_this();  

	std::shared_ptr<CGameObject> found;

	if (m_pSibling)
	{
		found = m_pSibling->FindFrame(pstrFrameName);
		if (found) return found;
	}

	if (m_pChild)
	{
		found = m_pChild->FindFrame(pstrFrameName);
		if (found) return found;
	}

	return nullptr;
}

void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Parent, *pxmf4x4Parent) : m_xmf4x4Parent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void CGameObject::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pSkinnedAnimationController)
		m_pSkinnedAnimationController->SetTrackAnimationSet(nAnimationTrack, nAnimationSet);
}

void CGameObject::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
{
	if (m_pSkinnedAnimationController)
		m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
}

void CGameObject::Animate(float fTimeElapsed)
{
	OnPrepareAnimate();

	if (HasType(EObjectType::DropWeapon)) {
		if (!m_bInAir)
		{
			// 1) 현재 위치
			XMVECTOR pos = XMLoadFloat4x4(&m_xmf4x4Parent).r[3];

			// 2) 바라볼 방향: 월드 업
			XMVECTOR lookDir = XMVectorSet(0, 1, 0, 0);

			// 3) 보조 up 벡터(lookDir과 절대 평행이 아니어야 함)
			XMVECTOR up = XMVectorSet(0, 0, 1, 0);

			// 4) view 매트릭스 생성 (LH 기준)
			XMMATRIX view = XMMatrixLookToLH(pos, lookDir, up);

			// 5) world 매트릭스 = view⁻¹
			XMMATRIX world = XMMatrixInverse(nullptr, view);

			// 6) 기존 스케일 유지
			XMFLOAT3 s = GetScale(m_xmf4x4Parent);
			XMMATRIX scaleM = XMMatrixScaling(s.x, s.y, s.z);

			// 7) 최종 합성
			XMMATRIX finalM = scaleM * world;
			XMStoreFloat4x4(&m_xmf4x4Parent, finalM);
			UpdateTransform(nullptr);
			return;
		}


		// === 회전(pivot 회전) ===
		// 축·각도
		XMFLOAT3 axis{ 1,0,0 };
		float angle = m_fRotationSpeed * fTimeElapsed;
		XMVECTOR vAxis = XMLoadFloat3(&axis);

		XMMATRIX R = XMMatrixRotationAxis(vAxis, XMConvertToRadians(angle));
		float h = -1.0f;            // 모델 높이에 맞춰 조절
		float pY = h * 0.5f;
		XMMATRIX T1 = XMMatrixTranslation(0, -pY, 0);
		XMMATRIX T2 = XMMatrixTranslation(0, pY, 0);
		XMMATRIX pivotRot = T2 * R * T1;

		// 2) 기존 부모 행렬 로드 → 자전 누적
		XMMATRIX parentMat = XMLoadFloat4x4(&m_xmf4x4Parent);
		XMMATRIX rotatedMat = pivotRot * parentMat;

		// 3) 회전 후 월드 위치 추출
		XMVECTOR worldPos = rotatedMat.r[3];

		// === 수평 이동 ===
		XMVECTOR dirNorm = XMVector3Normalize(target_dir);
		XMVECTOR horizOffset = XMVectorScale(dirNorm, m_fMoveSpeed * fTimeElapsed);

		// === 중력 적용 ===
		// 속도 갱신: v += (0, -g, 0) * dt
		XMVECTOR gravDelta = XMVectorSet(0, -m_fGravity * fTimeElapsed, 0, 0);
		m_vVelocity = XMVectorAdd(m_vVelocity, gravDelta);

		// 수직 이동량 = v.y * dt
		XMVECTOR vertOffset = XMVectorScale(m_vVelocity, fTimeElapsed);

		// === 최종 위치 업데이트 ===
		XMVECTOR totalOffset = XMVectorAdd(horizOffset, vertOffset);
		XMVECTOR newPos = XMVectorAdd(worldPos, totalOffset);

		// 착지 검사: y ≤ 0 이면 착지
		const float groundY = 0.0f;
		if (XMVectorGetY(newPos) <= groundY)
		{

			newPos = XMVectorSetY(newPos, groundY);
			m_bInAir = false;           // 착지!
			m_vVelocity = XMVectorZero(); // 속도 초기화
			Set_LookDirection_LookAt(XMFLOAT3(1, 0, 0));
			UpdateTransform(nullptr);
		}

		// 4) 행렬에 위치 반영
		rotatedMat.r[3] = newPos;
		XMStoreFloat4x4(&m_xmf4x4Parent, rotatedMat);

		// 5) 씬 그래프 갱신
		UpdateTransform(nullptr);
	}

	if (m_pSkinnedAnimationController)
		m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

	if (m_pSibling)
		if (m_pSibling->Get_Active())
			m_pSibling->Animate(fTimeElapsed);

	if (m_pChild)
		if (m_pChild->Get_Active())
			m_pChild->Animate(fTimeElapsed);
}

void CGameObject::Set_Last_Pos(XMFLOAT3 pos)
{
	previous_position.x = pos.x;
	previous_position.y = pos.y;
	previous_position.z = pos.z;
}

void CGameObject::Record_Last_Pos()
{
	if (m_pMesh)
	{
		XMFLOAT3 world_pos = GetPosition();

		previous_position.x = world_pos.x;
		previous_position.y = world_pos.y;
		previous_position.z = world_pos.z;
	}

	if (m_pSibling)
		m_pSibling->Record_Last_Pos();

	if (m_pChild)
		m_pChild->Record_Last_Pos();
}

bool CGameObject::IsVisible(CCamera* pCamera)
{
	bool bIsVisible = false;
	if (Get_Collider() == NULL)
		return true;
	BoundingOrientedBox xmBoundingBox(*Get_Collider());

	xmBoundingBox.Transform(xmBoundingBox, XMLoadFloat4x4(&m_xmf4x4World));

	if (pCamera)
		bIsVisible = pCamera->IsInFrustum(xmBoundingBox);
	return(bIsVisible);
}


void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pSkinnedAnimationController)
		m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);


	if (Active && m_pMesh)
	{
		//if (!IsVisible(pCamera))
			//return;

		// 객체의 셰이더 변수 업데이트
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

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
						pShader->Setting_Render(pd3dCommandList, 0);
						material_ptr->UpdateShaderVariable(pd3dCommandList);

						m_pMesh->Render(pd3dCommandList, i);
					}
					else
					{
						material_ptr->UpdateShaderVariable(pd3dCommandList);
						m_pMesh->Render(pd3dCommandList, i);
					}
				}
				++i;
			}
		}
	}

	if (m_pSibling)
		m_pSibling->Render(pd3dCommandList, pCamera);


	if (m_pChild)
		m_pChild->Render(pd3dCommandList, pCamera);

}

void CGameObject::Render_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pSkinnedAnimationController)
		m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (Active && m_pMesh)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

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
						pShader->Setting_Render(pd3dCommandList, 1);

						m_pMesh->Render(pd3dCommandList, i);

					}
				}
			}
		}
	}

		if (m_pSibling)
			m_pSibling->Render_Shadow(pd3dCommandList, pCamera);


		if (m_pChild)
			m_pChild->Render_Shadow(pd3dCommandList, pCamera);

}

void CGameObject::Render_Depth(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pSkinnedAnimationController)
		m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (Active && m_pMesh)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

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
						pShader->Setting_Render(pd3dCommandList, 2);
						material_ptr->UpdateShaderVariable(pd3dCommandList);

						m_pMesh->Render(pd3dCommandList, i);

					}
				}
			}
		}
	}

	if (m_pSibling)
		m_pSibling->Render_Depth(pd3dCommandList, pCamera);


	if (m_pChild)
		m_pChild->Render_Depth(pd3dCommandList, pCamera);

}


void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 1, &Blending_value, 28);
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 3, &Blending_color, 29);
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 16, &xmf4x4World, 0);   // 0~15

	//===============================================================================
	XMFLOAT3 now_position = GetPosition();
	XMFLOAT3 velocity = {
		now_position.x - previous_position.x,
		now_position.y - previous_position.y,
		now_position.z - previous_position.z
	};
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 3, &velocity, 25);      // 25~27

	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 1, &Blending_value, 28); 
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 3, &Blending_color, 29); 

}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial)
{
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (std::shared_ptr<CMaterial> material_ptr : Material_list)
	{
		if (material_ptr != NULL)
			material_ptr->ReleaseUploadBuffers();
	}

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CGameObject::Modify_World_Position(XMFLOAT3 newPosition)
{
	XMVECTOR scale, rotation, translation;


	XMMatrixDecompose(&scale, &rotation, &translation, XMLoadFloat4x4(&m_xmf4x4World));

	translation = XMLoadFloat3(&newPosition);

	XMMATRIX newWorldMatrix = XMMatrixScalingFromVector(scale) *
		XMMatrixRotationQuaternion(rotation) *
		XMMatrixTranslationFromVector(translation);

	XMStoreFloat4x4(&m_xmf4x4World, newWorldMatrix);
}

void CGameObject::Modify_World_Up_Vector(XMFLOAT3 newUpVector)
{
	XMVECTOR scale, rotation, translation;
	XMMATRIX worldMatrix = XMLoadFloat4x4(&m_xmf4x4World);

	// 기존 행렬을 Scale, Rotation, Translation으로 분해
	XMMatrixDecompose(&scale, &rotation, &translation, worldMatrix);

	// 기존 Forward 및 Right 벡터 추출
	XMVECTOR forward = XMVector3Normalize(worldMatrix.r[2]); // 기존 Z축 (Forward)
	XMVECTOR right = XMVector3Normalize(worldMatrix.r[0]);   // 기존 X축 (Right)

	// 입력받은 Up 벡터를 정규화
	XMVECTOR newUp = XMVector3Normalize(XMLoadFloat3(&newUpVector));

	// 기존 Forward 벡터와 새 Up 벡터가 같은 방향이면 처리 (기본적으로 Right 벡터 유지)
	if (XMVector3Equal(forward, newUp) || XMVector3Equal(XMVectorNegate(forward), newUp))
	{
		forward = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f); // 예외 처리: 기본 Forward 벡터 설정
	}

	// 새로운 Right 벡터 계산 (newUp과 Forward의 외적)
	XMVECTOR newRight = XMVector3Normalize(XMVector3Cross(newUp, forward));

	// 새로운 Forward 벡터 계산 (Right와 Up의 외적)
	XMVECTOR newForward = XMVector3Normalize(XMVector3Cross(newRight, newUp));

	// 새로운 회전 행렬 구성
	XMMATRIX newRotationMatrix = XMMATRIX(
		newRight,   // X축 (Right)
		newUp,      // Y축 (Up)
		newForward, // Z축 (Forward)
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)
	);

	// 최종 월드 행렬 계산 (Scale * Rotation * Translation)
	XMMATRIX newWorldMatrix = XMMatrixScalingFromVector(scale) * newRotationMatrix * XMMatrixTranslationFromVector(translation);

	// 변환된 월드 행렬 저장
	XMStoreFloat4x4(&m_xmf4x4World, newWorldMatrix);
}


void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4Parent._41 = x;
	m_xmf4x4Parent._42 = y;
	m_xmf4x4Parent._43 = z;

	UpdateTransform(NULL);
}


void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::Move(XMFLOAT3 xmf3Offset)
{
	m_xmf4x4Parent._41 += xmf3Offset.x;
	m_xmf4x4Parent._42 += xmf3Offset.y;
	m_xmf4x4Parent._43 += xmf3Offset.z;

	UpdateTransform(NULL);
}

void CGameObject::SetScale(XMFLOAT3 scale, bool keepPosition)
{
	SetScale(scale.x, scale.y, scale.z, keepPosition);
}

void CGameObject::SetScale(float x, float y, float z, bool keepPosition)
{
	XMVECTOR worldPosBefore = XMVectorZero();
	if (keepPosition)
	{
		XMMATRIX worldMatrixBefore = XMLoadFloat4x4(&m_xmf4x4World);
		worldPosBefore = XMVector3TransformCoord(XMVectorZero(), worldMatrixBefore);
	}

	XMVECTOR scale, rotation, translation;
	XMMATRIX parentMat = XMLoadFloat4x4(&m_xmf4x4Parent);
	XMMatrixDecompose(&scale, &rotation, &translation, parentMat);

	scale = XMVectorSet(x, y, z, 0.0f);
	XMMATRIX newParent = XMMatrixScalingFromVector(scale) *
		XMMatrixRotationQuaternion(rotation) *
		XMMatrixTranslationFromVector(translation);
	XMStoreFloat4x4(&m_xmf4x4Parent, newParent);

	UpdateTransform(m_pParent ? &m_pParent->m_xmf4x4World : nullptr);

	if (keepPosition)
	{
		XMMATRIX worldMatrixAfter = XMLoadFloat4x4(&m_xmf4x4World);
		XMVECTOR worldPosAfter = XMVector3TransformCoord(XMVectorZero(), worldMatrixAfter);

		XMVECTOR offset = worldPosBefore - worldPosAfter;

		XMMATRIX correction = XMMatrixTranslationFromVector(offset);
		worldMatrixAfter = correction * worldMatrixAfter;

		XMMATRIX parentInv = XMMatrixIdentity();
		if (m_pParent)
		{
			parentInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_pParent->m_xmf4x4World));
		}

		XMMATRIX correctedLocal = worldMatrixAfter * parentInv;
		XMStoreFloat4x4(&m_xmf4x4Parent, correctedLocal);

		UpdateTransform(m_pParent ? &m_pParent->m_xmf4x4World : nullptr);
	}
}


XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetToParentPosition()
{
	return(XMFLOAT3(m_xmf4x4Parent._41, m_xmf4x4Parent._42, m_xmf4x4Parent._43));
}

XMFLOAT3 CGameObject::Get_World_Position()
{
	XMFLOAT3 obj_pos = GetPosition();
	XMFLOAT3 parent_pos = { 0.0f, 0.0f, 0.0f };

	if (m_pParent != nullptr)
		parent_pos = m_pParent->Get_World_Position();

	return XMFLOAT3(obj_pos.x + parent_pos.x, obj_pos.y + parent_pos.y, obj_pos.z + parent_pos.z);
}

std::shared_ptr<CGameObject> CGameObject::Get_Root_Object()
{
	std::shared_ptr<CGameObject> root = shared_from_this();
	while (root->m_pParent != nullptr)
	{
		root = root->m_pParent;
	}
	return root;
}

XMFLOAT3 CGameObject::Get_Root_WorldPosition()
{
	return Get_Root_Object()->GetPosition();
}

XMFLOAT3 CGameObject::Get_Root_Obj_Displacement()
{
	XMFLOAT3 worldPosition = GetPosition();
	XMFLOAT3 rootPosition = Get_Root_WorldPosition();

	return XMFLOAT3(
		worldPosition.x - rootPosition.x,
		worldPosition.y - rootPosition.y,
		worldPosition.z - rootPosition.z
	);
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4Parent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Parent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4Parent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Parent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4Parent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Parent);

	UpdateTransform(NULL);
}

void CGameObject::RotateInWorldAroundUp(float fAngle)
{
	RotateInWorld(&GetUp(), fAngle);
}

void CGameObject::RotateInWorld(XMFLOAT3* pxmf3WorldAxis, float fAngle)
{
	XMVECTOR vAxis = XMVector3Normalize(XMLoadFloat3(pxmf3WorldAxis));
	XMMATRIX mtxRotate = XMMatrixRotationAxis(vAxis, XMConvertToRadians(fAngle));

	XMMATRIX mWorld = XMLoadFloat4x4(&m_xmf4x4World);
	mWorld = XMMatrixMultiply(mWorld, mtxRotate); // 월드 공간에서 회전 적용

	// 부모가 있으면 역변환
	XMMATRIX mParentInv = XMMatrixIdentity();
	if (m_pParent)
	{
		mParentInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_pParent->m_xmf4x4World));
	}

	XMStoreFloat4x4(&m_xmf4x4Parent, XMMatrixMultiply(mWorld, mParentInv));
	UpdateTransform(nullptr);
}

void CGameObject::SetLookDirection(float x, float y, float z)
{
	SetLookDirection(XMFLOAT3(x, y, z));
}

void CGameObject::SetLookDirection(const XMFLOAT3& xmf3LookInput)
{
	XMFLOAT3 xmf3Look = xmf3LookInput;
	if (Vector3::Length(xmf3Look) < 1e-6f)
		xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3 look = Vector3::Normalize(xmf3Look);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	if (fabs(Vector3::DotProduct(look, up)) > 0.999f)
		up = XMFLOAT3(1.0f, 0.0f, 0.0f);

	XMFLOAT3 right = Vector3::Normalize(Vector3::CrossProduct(up, look));
	up = Vector3::Normalize(Vector3::CrossProduct(look, right));

	XMFLOAT3 scale = GetScale(m_xmf4x4Parent);
	XMFLOAT3 position = GetPosition();

	XMVECTOR vRight = XMVectorScale(XMLoadFloat3(&right), scale.x);
	XMVECTOR vUp = XMVectorScale(XMLoadFloat3(&up), scale.y);
	XMVECTOR vLook = XMVectorScale(XMLoadFloat3(&look), scale.z);

	XMFLOAT4X4 xmf4x4New;
	xmf4x4New._11 = XMVectorGetX(vRight); xmf4x4New._12 = XMVectorGetY(vRight); xmf4x4New._13 = XMVectorGetZ(vRight); xmf4x4New._14 = 0.0f;
	xmf4x4New._21 = XMVectorGetX(vUp);    xmf4x4New._22 = XMVectorGetY(vUp);    xmf4x4New._23 = XMVectorGetZ(vUp);    xmf4x4New._24 = 0.0f;
	xmf4x4New._31 = XMVectorGetX(vLook);  xmf4x4New._32 = XMVectorGetY(vLook);  xmf4x4New._33 = XMVectorGetZ(vLook);  xmf4x4New._34 = 0.0f;
	xmf4x4New._41 = position.x;           xmf4x4New._42 = position.y;           xmf4x4New._43 = position.z;           xmf4x4New._44 = 1.0f;

	m_xmf4x4Parent = xmf4x4New;
	UpdateTransform(NULL);
}


void CGameObject::Set_LookDirection_LookAt(float x, float y, float z)
{
	Set_LookDirection_LookAt(XMFLOAT3(x, y, z));
}

void CGameObject::Set_LookDirection_LookAt(const XMFLOAT3& lookDir)
{
	XMVECTOR vPosition = XMLoadFloat3(&GetPosition());
	XMVECTOR vLook = XMVector3Normalize(XMLoadFloat3(&lookDir));
	XMVECTOR vUp = XMVectorSet(0, 1, 0, 0);

	// LH 기준, 현재 위치에서 lookDir 방향으로 바라보는 view matrix
	XMMATRIX mLook = XMMatrixLookToLH(vPosition, vLook, vUp);

	// view matrix의 역행렬 = world matrix
	XMMATRIX mWorld = XMMatrixInverse(nullptr, mLook);

	// 기존 스케일 유지
	XMFLOAT3 scale = GetScale(m_xmf4x4Parent);
	XMMATRIX mScale = XMMatrixScaling(scale.x, scale.y, scale.z);

	XMMATRIX mFinal = mScale * mWorld;

	XMStoreFloat4x4(&m_xmf4x4Parent, mFinal);
	UpdateTransform(NULL);
}

void CGameObject::AlignWithNormal(XMFLOAT3& newNormal)
{
	// 새로운 Up 벡터
	XMFLOAT3 newUp = Vector3::Normalize(newNormal);

	// 기존 Look 유지
	XMFLOAT3 look = GetLook();

	// look과 up이 완전히 평행하면 → cross product가 0되므로 체크
	if (abs(Vector3::DotProduct(look, newUp)) > 0.99f)
	{
		// fallback: Right 유지, Look 재계산
		XMFLOAT3 right = GetRight();
		look = Vector3::CrossProduct(newUp, right, true);
	}

	// Right 재계산
	XMFLOAT3 right = Vector3::CrossProduct(newUp, look, true);
	look = Vector3::CrossProduct(right, newUp, true);

	// 스케일 유지
	XMFLOAT3 scale = GetScale(m_xmf4x4Parent);

	XMMATRIX mWorld;
	mWorld.r[0] = XMVectorScale(XMLoadFloat3(&right), scale.x);
	mWorld.r[1] = XMVectorScale(XMLoadFloat3(&newUp), scale.y);
	mWorld.r[2] = XMVectorScale(XMLoadFloat3(&look), scale.z);
	mWorld.r[3] = XMVectorSet(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43, 1.0f);

	// 부모가 있다면 로컬 변환으로 역변환
	XMMATRIX mLocal = mWorld;
	if (m_pParent)
	{
		XMMATRIX parentInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_pParent->m_xmf4x4World));
		mLocal = mWorld * parentInv;
	}

	XMStoreFloat4x4(&m_xmf4x4Parent, mLocal);
	UpdateTransform(nullptr);
}

std::shared_ptr<CTexture> CGameObject::FindReplicatedTexture(const _TCHAR* pstrTextureName)
{
	for (const auto& material_ptr : Material_list)
	{
		for (size_t j = 0; j < material_ptr->m_nTextures; j++)
		{
			if (material_ptr->m_ppTextures[j])
			{
				if (!_tcsncmp(material_ptr->m_ppstrTextureNames[j].data(), pstrTextureName, _tcslen(pstrTextureName)))
					return material_ptr->m_ppTextures[j];
			}
		}
	}

	std::shared_ptr<CTexture> pTexture = nullptr;
	if (m_pSibling && (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName))) return pTexture;
	if (m_pChild && (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName))) return pTexture;

	return nullptr;
}


int ReadIntegerFromFile(FILE* pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
	return(nValue);
}

float ReadFloatFromFile(FILE* pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
	return(fValue);
}

BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

std::unordered_map<std::string, std::shared_ptr<CMesh>> CGameObject::MeshCache;

std::shared_ptr<CMesh> CGameObject::LoadMeshWithCache(const std::string& meshPath, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	auto it = MeshCache.find(meshPath);
	if (it != MeshCache.end())
		return it->second;

	auto mesh = std::make_shared<CStandardMesh>(device, cmdList);
	mesh->LoadMeshFrom_OtherFile(device, cmdList, meshPath.c_str());
	MeshCache[meshPath] = mesh;
	return mesh;
}

void CGameObject::ClearMeshCache()
{
	MeshCache.clear();
}

void CGameObject::LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::shared_ptr<CGameObject> pParent, FILE* pInFile, CShader* pShader)
{
	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	Material_list.clear();
	int materialCount = ReadIntegerFromFile(pInFile);
	Material_list.reserve(materialCount);

	std::shared_ptr<CMaterial> pMaterial = nullptr;
	std::vector<Light_Material_Info> tempLightInfos(materialCount);

	for (;;)
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ReadIntegerFromFile(pInFile);
			pMaterial = std::make_shared<CMaterial>(7);

			if (!pShader)
			{
				UINT nMeshType = GetMeshType();
				if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE)
				{
					if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT)
						pMaterial->SetSkinnedAnimationShader();
					else
						pMaterial->SetStandardShader();
				}
			}

			if (nMaterial >= Material_list.size())
				Material_list.resize(nMaterial + 1);

			Material_list[nMaterial] = pMaterial;
			tempLightInfos[nMaterial] = Light_Material_Info{};
		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_cAlbedo), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(tempLightInfos[nMaterial].gEmissive), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(tempLightInfos[nMaterial].gSpecular), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(tempLightInfos[nMaterial].gSpecular.w), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(tempLightInfos[nMaterial].gRoughness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(tempLightInfos[nMaterial].gMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Glossiness>:") || !strcmp(pstrToken, "<GlossyReflection>:"))
		{
			float dummy = 0.0f;
			nReads = (UINT)::fread(&dummy, sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ALBEDO_MAP, ROOT_PARAMETER_ALBEDO_TEXTURE_SRV_INDEX,
				pMaterial->m_ppstrTextureNames[0].data(), pMaterial->m_ppTextures, 0, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<SpecularMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_SPECULAR_MAP, ROOT_PARAMETER_SPECULAR_TEXTURE_SRV_INDEX,
				pMaterial->m_ppstrTextureNames[1].data(), pMaterial->m_ppTextures, 1, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_NORMAL_MAP, ROOT_PARAMETER_NORMAL_TEXTURE_SRV_INDEX,
				pMaterial->m_ppstrTextureNames[2].data(), pMaterial->m_ppTextures, 2, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<MetallicMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_METALLIC_MAP, ROOT_PARAMETER_METALLIC_TEXTURE_SRV_INDEX,
				pMaterial->m_ppstrTextureNames[3].data(), pMaterial->m_ppTextures, 3, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<EmissionMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_EMISSION_MAP, ROOT_PARAMETER_EMISSION_TEXTURE_SRV_INDEX,
				pMaterial->m_ppstrTextureNames[4].data(), pMaterial->m_ppTextures, 4, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			for (int i = 0; i < Material_list.size(); ++i)
			{
				auto& material = Material_list[i];
				if (material)
				{
					const Light_Material_Info& info = tempLightInfos[i];
					int similarID = Light_Material_Manager::Find_Similar_Material(info);

					if (similarID >= 0)
					{
						material->m_Material_ID = static_cast<UINT>(similarID);
					}
					else
					{
						material->m_Material_ID = Light_Material_Manager::Add_Material(info);
					}
				}
			}
			break;
		}
	}
}


std::shared_ptr<CGameObject> CGameObject::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, std::shared_ptr<CGameObject> pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	std::shared_ptr<CGameObject> pGameObject = std::make_shared<CGameObject>();

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4Parent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{

//			CStandardMesh* pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
			shared_ptr<CStandardMesh> pMesh = make_shared<CStandardMesh>(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			//CSkinnedMesh* pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			shared_ptr<CSkinnedMesh> pSkinnedMesh = make_shared<CSkinnedMesh>(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
			if (!strcmp(pstrToken, "<Mesh>:"))
				pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			pGameObject->SetMesh(pSkinnedMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					std::shared_ptr<CGameObject> pChild_raw_ptr = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader, pnSkinnedMeshes);

					std::shared_ptr<CGameObject> pChild(pChild_raw_ptr);
					if (pChild)
						pGameObject->Set_Child(pChild);
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

std::shared_ptr<CGameObject> CGameObject::Load_Scene_HierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, std::shared_ptr<CGameObject> pParent, FILE* pInFile, CShader* pShader)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	std::shared_ptr<CGameObject> pGameObject = std::make_shared<CGameObject>();

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4Parent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			::ReadStringFromFile(pInFile, pstrToken);
			if (!strcmp(pstrToken, "<Mesh_Name>:"))
			{
				::ReadStringFromFile(pInFile, pstrToken);
				std::string fileName = "Scene/Scene_File_7/Meshes/bin/" + std::string(pstrToken);

				std::shared_ptr<CMesh> sharedMesh = CGameObject::LoadMeshWithCache(fileName, pd3dDevice, pd3dCommandList);
				if (sharedMesh)
					pGameObject->SetMesh(sharedMesh);
			}
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					std::shared_ptr<CGameObject> pChild_raw_ptr = CGameObject::Load_Scene_HierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader);

					std::shared_ptr<CGameObject> pChild(pChild_raw_ptr); // 소유권 이전
					if (pChild)
						pGameObject->Set_Child(pChild);
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

CLoadedModelInfo* CGameObject::Load_Scene_File(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();

	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				std::shared_ptr<CGameObject> ModelRootObject_raw_ptr = CGameObject::Load_Scene_HierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader);

				std::shared_ptr<CGameObject> ModelRootObject_shared_ptr(ModelRootObject_raw_ptr);
				pLoadedModel->m_pModelRootObject = ModelRootObject_shared_ptr;

				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
		}
		else
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	//	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pLoadedModel);
}

void CGameObject::FlattenGameObjectHierarchy(std::shared_ptr<CGameObject> node, std::vector<shared_ptr<CGameObject>>& outList)
{
	if (!node) return;

	const std::string& name = node->Get_Name();


	if (!name.empty())
	{
		outList.push_back(node);
	}


	std::shared_ptr<CGameObject> child = node->Get_Child();
	while (child)
	{
		FlattenGameObjectHierarchy(child, outList);
		child = child->Get_Sibling();
	}
}

void CGameObject::PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent)
{
	if (pParent != NULL)
	{
		char pstrDebug[256] = { 0 };
		sprintf_s(pstrDebug, sizeof(pstrDebug), "\n(Frame: %s) <- (Parent: %s)", pGameObject->m_pstrFrameName, pParent->m_pstrFrameName);
		OutputDebugStringA(pstrDebug);
	}

	if (pGameObject->m_pSibling)
		CGameObject::PrintFrameInfo(pGameObject->m_pSibling.get(), pParent);

	if (pGameObject->m_pChild)
		CGameObject::PrintFrameInfo(pGameObject->m_pChild.get(), pGameObject);
}

void CGameObject::LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel, char* pstrFileName)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nAnimationSets = 0;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nAnimationSets = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets = new CAnimationSets(nAnimationSets);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			pLoadedModel->m_pAnimationSets->m_nBoneFrames = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches.resize(pLoadedModel->m_pAnimationSets->m_nBoneFrames, nullptr);
			//pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches = new CGameObject*[pLoadedModel->m_pAnimationSets->m_nBoneFrames];

			for (int j = 0; j < pLoadedModel->m_pAnimationSets->m_nBoneFrames; j++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				shared_ptr<CGameObject> frame_ptr = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);
				pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches[j] = frame_ptr.get();

#ifdef _WITH_DEBUG_SKINNING_BONE
				TCHAR pstrDebug[256] = { 0 };
				TCHAR pwstrAnimationBoneName[64] = { 0 };
				TCHAR pwstrBoneCacheName[64] = { 0 };
				size_t nConverted = 0;
				mbstowcs_s(&nConverted, pwstrAnimationBoneName, 64, pstrToken, _TRUNCATE);
				mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, pLoadedModel->m_ppBoneFrameCaches[j]->m_pstrFrameName, _TRUNCATE);
				_stprintf_s(pstrDebug, 256, _T("AnimationBoneFrame:: Cache(%s) AnimationBone(%s)\n"), pwstrBoneCacheName, pwstrAnimationBoneName);
				OutputDebugString(pstrDebug);
#endif
			}
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken); //Animation Set Name

			float fLength = ::ReadFloatFromFile(pInFile);
			int nFramesPerSecond = ::ReadIntegerFromFile(pInFile);
			int nKeyFrames = ::ReadIntegerFromFile(pInFile);

//			pLoadedModel->m_pAnimationSets->m_pAnimationSet_list[nAnimationSet] = new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames, pstrToken);
//
//			for (int i = 0; i < nKeyFrames; i++)
//			{
//				::ReadStringFromFile(pInFile, pstrToken);
//				if (!strcmp(pstrToken, "<Transforms>:"))
//				{
//					CAnimationSet* pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSet_list[nAnimationSet];
//
//					int nKey = ::ReadIntegerFromFile(pInFile); //i
//					float fKeyTime = ::ReadFloatFromFile(pInFile);
//
//#ifdef _WITH_ANIMATION_SRT
//					m_pfKeyFrameScaleTimes[i] = fKeyTime;
//					m_pfKeyFrameRotationTimes[i] = fKeyTime;
//					m_pfKeyFrameTranslationTimes[i] = fKeyTime;
//					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameScales[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
//					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4KeyFrameRotations[i], sizeof(XMFLOAT4), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
//					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameTranslations[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
//#else
//					pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
//					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
//#endif
//				}
//			}

		/*	auto animSet = std::make_shared<CAnimationSet>(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames, pstrToken);
			std::string filename_key(pstrFileName);
			auto sharedAnimSet = CAnimationSets::AddOrGetSharedAnimationSet(animSet, filename_key);
			pLoadedModel->m_pAnimationSets->m_pAnimationSet_list[nAnimationSet] = sharedAnimSet.get();

			bool bIsNew = (sharedAnimSet.get() == animSet.get());*/

			auto animSet = std::make_shared<CAnimationSet>(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames, pstrToken);
			std::string filename_key(pstrFileName);
			auto sharedAnimSet = CAnimationSets::AddOrGetSharedAnimationSet(animSet, filename_key);

			pLoadedModel->m_pAnimationSets->m_pAnimationSet_list[nAnimationSet] = sharedAnimSet;

			bool bIsNew = (sharedAnimSet == animSet);

			if (bIsNew)
			{
				for (int i = 0; i < nKeyFrames; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<Transforms>:"))
					{
						CAnimationSet* pAnimationSet = sharedAnimSet.get();

						int nKey = ::ReadIntegerFromFile(pInFile);
						float fKeyTime = ::ReadFloatFromFile(pInFile);

						pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
						nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i],
							sizeof(XMFLOAT4X4),
							pLoadedModel->m_pAnimationSets->m_nBoneFrames,
							pInFile);
					}
				}
			}
			else
			{
				for (int i = 0; i < nKeyFrames; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken); // "<Transforms>"
					int nKey = ::ReadIntegerFromFile(pInFile); // i
					float fKeyTime = ::ReadFloatFromFile(pInFile); // skip
					fseek(pInFile, sizeof(XMFLOAT4X4) * pLoadedModel->m_pAnimationSets->m_nBoneFrames, SEEK_CUR); // skip fread
				}
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}

CLoadedModelInfo* CGameObject::LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();

	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				std::shared_ptr<CGameObject> ModelRootObject_raw_ptr = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, &pLoadedModel->m_nSkinnedMeshes);

				std::shared_ptr<CGameObject> ModelRootObject_shared_ptr(ModelRootObject_raw_ptr);
				pLoadedModel->m_pModelRootObject = ModelRootObject_shared_ptr;

				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
			else if (!strcmp(pstrToken, "<Animation>:"))
			{
				CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel, pstrFileName);
				pLoadedModel->PrepareSkinning();
			}
			else if (!strcmp(pstrToken, "</Animation>:"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	//	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pLoadedModel);
}

std::string CGameObject::Get_Mesh_Name()
{
	if (m_pMesh != NULL)
		if (m_pMesh->Vertex_Existence())
			return m_pMesh->Get_Name();

	return string("None");


}

BoundingOrientedBox* CGameObject::Get_Collider()
{
	if (m_pMesh == NULL) return NULL;

	BoundingOrientedBox* pOriginalBoundingBox = m_pMesh->Get_BoundingBox();
	if (pOriginalBoundingBox == NULL) return NULL;

	m_WorldOBB = *pOriginalBoundingBox;

	if (m_WorldOBB.Extents.x == 0.0f) m_WorldOBB.Extents.x = 1.0f;
	if (m_WorldOBB.Extents.y == 0.0f) m_WorldOBB.Extents.y = 1.0f;
	if (m_WorldOBB.Extents.z == 0.0f) m_WorldOBB.Extents.z = 1.0f;

	XMVECTOR quaternionRotation = XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_xmf4x4World));
	XMStoreFloat4(&m_WorldOBB.Orientation, quaternionRotation);

	return &m_WorldOBB;
}

void CGameObject::Add_Collider(float cube_length)
{
	if (cube_length > 0.0f)
	{
		BoundingOrientedBox* collider_box = new BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(cube_length, cube_length, cube_length), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		Set_Collider(collider_box);
	}
	else
		Set_Collider(NULL);

}

void CGameObject::Set_Collider(BoundingOrientedBox* ptr)
{
	if (m_pMesh == NULL)
		m_pMesh = make_shared<OBBContainer>();
//		m_pMesh = new OBBContainer();

	m_pMesh->Set_BoundingBox(ptr);
}

ServerSyncData CGameObject::MakeSyncData()
{
	ServerSyncData data;
	data.position = GetPosition();
	data.lookVector = GetLook();
	if (GetSkinnedAnimationController()) {
		data.track_info_list = GetSkinnedAnimationController()->MakeSyncData();
	}
	return data;
}

void CGameObject::ApplySyncData(const ServerSyncData& syncData)
{
	SetLookDirection(syncData.lookVector);
	SetPosition(syncData.position);
}

void CGameObject::Launch(const XMVECTOR& target_dir)
{
	m_fRotationSpeed = 720.0f;
	if (m_bInAir) return;
	m_bInAir = true;
	XMVECTOR dirNorm = XMVector3Normalize(target_dir);
	m_vVelocity = XMVectorSet(
		XMVectorGetX(dirNorm) * m_fMoveSpeed,
		m_fInitialUpSpeed,
		XMVectorGetZ(dirNorm) * m_fMoveSpeed,
		0.0f
	);
}

std::shared_ptr<CGameObject> CGameObject::DropWeapon(const char* targetName) {
	std::shared_ptr<CGameObject> root = Get_Root_Object();
	root->UpdateTransform(nullptr);

	std::shared_ptr<CGameObject> target = FindFrame(const_cast<char*>(targetName));
	if (!target || !target->m_pParent)
		return nullptr;
	target->Set_Active(false);
	std::shared_ptr<CGameObject> parentRaw = target->GetParent();

	std::shared_ptr<CGameObject> prev;
	std::shared_ptr<CGameObject> curr = parentRaw->m_pChild;
	while (curr && curr.get() != target.get()) {
		prev = curr;
		curr = curr->m_pSibling;
	}
	if (!curr)
		return nullptr;

	XMFLOAT4X4 savedWorld = curr->m_xmf4x4World;

	curr->m_xmf4x4Parent = savedWorld;
	curr->m_xmf4x4World = savedWorld;

	auto rawSword = FindFrame(const_cast<char*>(targetName));
	auto swordClone = rawSword->GetWeapon(false);

	pWeapon = new WeaponObject();
	pWeapon->pWeapon.push_back(swordClone);
	swordClone->Launch(XMVectorNegate(XMLoadFloat3(&GetLook())));
	swordClone->target_dir = XMVectorNegate(XMLoadFloat3(&GetLook()));;
	swordClone->type = EObjectType::DropWeapon;
	swordClone->Set_Active(true);

	return swordClone;
}

void CGameObject::RestoreWeapon(const char* targetName) {
	auto sword = FindFrame(const_cast<char*>(targetName));
	if (sword != nullptr)
		sword->Set_Active(true);
	if (!pWeapon->pWeapon.empty()) {
		//pWeapon->pWeapon.clear();
		//for (auto& obj : pWeapon->pWeapon) {
		//	//obj->Set_Active(false);
		//	obj.reset();
		//}
	}
}

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, LPCTSTR pFileName,
	int start_x_pos, int start_z_pos, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, int Vertex_gap, int nMaxDepth, shared_ptr<CHeightMapImage> sharedHeightMapImage)	: CGameObject(1)
{
	static int tile_map_number = 0;
	bool isRoot = (sharedHeightMapImage == nullptr);
	m_pHeightMapImage = isRoot ? make_shared<CHeightMapImage>(pFileName, nWidth, nLength, xmf3Scale) : sharedHeightMapImage;

	if (isRoot) 
	{
		m_TerrainBaseTexture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
		m_TerrainBaseTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Sand_Base.dds", RESOURCE_TEXTURE2D, 0);
		m_TerrainDetailTexture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
		m_TerrainDetailTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Detail_Texture_8.dds", RESOURCE_TEXTURE2D, 0);

		m_TerrainShader = new Deferred_CTerrainShader();
		m_TerrainShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, RenderTarget_Config::RTV_FORMAT_num, RenderTarget_Config::RTV_FORMATS, RenderTarget_Config::DSV_FORMAT);
		m_TerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

		CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, m_TerrainBaseTexture.get(), 0, ROOT_PARAMETER_TERRAIN_BASE_TEXTURE_SRV_INDEX);
		CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, m_TerrainDetailTexture.get(), 0, ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX);

		m_TerrainMaterial = new CMaterial(2);
		m_TerrainMaterial->SetTexture(m_TerrainBaseTexture, 0);
		m_TerrainMaterial->SetTexture(m_TerrainDetailTexture, 1);
		m_TerrainMaterial->SetShader(m_TerrainShader);

		Light_Material_Info light_info;
		light_info.gSpecular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
		m_TerrainMaterial->m_Material_ID = Light_Material_Manager::Add_Material(light_info);
	}

	(tile_map_number == 0) ? Set_Name("Root_Tile_Map") : Set_Tile(tile_map_number++);

	Vertex_gap = (Vertex_gap % 2) ? Vertex_gap + 1 : Vertex_gap;
	m_nWidth = nWidth; 
	m_nLength = nLength; 
	m_xmf3Scale = xmf3Scale; 
	m_nDepth = nMaxDepth;
	Area_LT = { start_x_pos * xmf3Scale.x, start_z_pos * xmf3Scale.z };
	Area_RB = { (start_x_pos + m_nWidth-1) * xmf3Scale.x, (start_z_pos + m_nLength-1) * xmf3Scale.z };
	Tile_Start_Pos = { (float)start_x_pos , (float)start_z_pos };

	if (isRoot) 
	{
		m_pFullMesh = make_shared<CHeightMapGridMesh>(pd3dDevice, pd3dCommandList, 0, 0,
			m_pHeightMapImage->GetHeightMapWidth(), m_pHeightMapImage->GetHeightMapLength(),
			xmf3Scale, xmf4Color, Vertex_gap, m_pHeightMapImage.get());
		SetMesh(nullptr);
	}
	else if (m_nDepth == 0) 
	{
		auto partMesh = make_shared<CHeightMapGridMesh>(pd3dDevice, pd3dCommandList, start_x_pos, start_z_pos,
			nWidth + 1, nLength + 1, xmf3Scale, xmf4Color, Vertex_gap, m_pHeightMapImage.get());
		SetMesh(partMesh);
	}
	else SetMesh(nullptr);
}

void CHeightMapTerrain::DivideIntoChildren(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, LPCTSTR pFileName, XMFLOAT3 xmf3Scale, int Vertex_gap)
{
	if (m_nDepth <= 0) return;
	int Cell_num = 2;
	long blocks_x_size[2] = { m_nWidth / 2, m_nWidth - m_nWidth / 2 };
	long blocks_z_size[2] = { m_nLength / 2, m_nLength - m_nLength / 2 };

	for (int z = 0; z < Cell_num; ++z) 
	{
		for (int x = 0; x < Cell_num; ++x) 
		{
			XMFLOAT4 tile_color = Get_Random_Color(1.0f);
			int xStart = (int)Tile_Start_Pos.x + x * blocks_x_size[0];
			int zStart = (int)Tile_Start_Pos.y + z * blocks_z_size[0];

			auto child = std::make_shared<CHeightMapTerrain>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pFileName, xStart, zStart,
				blocks_x_size[x], blocks_z_size[z], xmf3Scale, tile_color, Vertex_gap, m_nDepth - 1, m_pHeightMapImage);

			Set_Child(child);
			child->DivideIntoChildren(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pFileName, xmf3Scale, Vertex_gap);
		}
	}
}


CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_TerrainShader)
	{
		m_TerrainShader->ReleaseShaderVariables();
		delete m_TerrainShader;
		m_TerrainShader = nullptr;
	}

	if (m_TerrainMaterial)
	{
		m_TerrainMaterial->ReleaseUploadBuffers();
		delete m_TerrainMaterial;
		m_TerrainMaterial = nullptr;
	}

	m_TerrainBaseTexture.reset();
	m_TerrainDetailTexture.reset();
	m_pFullMesh.reset();
	m_pHeightMapImage.reset();
}

void CHeightMapTerrain::Set_Tile(int n)
{
	tile_number = n;
	string tile_name = "tile map - " + std::to_string(n);
	Set_Name(tile_name);
}

float CHeightMapTerrain::Get_Height(float x, float z, bool bReverseQuad)
{
	CHeightMapTerrain* last_tile_ptr = nullptr;
	return Get_Height(x, z, bReverseQuad, last_tile_ptr);
}

float CHeightMapTerrain::Get_Height(float x, float z, bool bReverseQuad, CHeightMapTerrain*& last_tile_ptr)
{
	float world_height = Get_World_Position().y;
	float mesh_height = 0.0f;

	if (last_tile_ptr != NULL)
		mesh_height = Get_Mesh_Height(x, z, bReverseQuad, last_tile_ptr);
	else
		mesh_height = Get_Mesh_Height(x, z, bReverseQuad);

	return world_height + mesh_height;
}

float CHeightMapTerrain::Get_Mesh_Height(float x, float z, bool bReverseQuad)
{
	CHeightMapTerrain* last_tile_ptr = nullptr;
	return Get_Mesh_Height(x, z, bReverseQuad, last_tile_ptr);
}

float CHeightMapTerrain::Get_Mesh_Height(float x, float z, bool bReverseQuad, CHeightMapTerrain*& last_tile_ptr)
{
	if (last_tile_ptr != NULL)
	{
		if (x >= last_tile_ptr->Area_LT.x && x < last_tile_ptr->Area_RB.x &&
			z >= last_tile_ptr->Area_LT.y && z < last_tile_ptr->Area_RB.y)
		{
			return last_tile_ptr->Get_Mesh_Height(x, z, bReverseQuad);
		}
	}
	else
	{
		last_tile_ptr = nullptr;
	}

	if (Get_Child())
	{
		CGameObject* child_ptr = Get_Child().get();
		float h = ((CHeightMapTerrain*)child_ptr)->Get_Mesh_Height(x, z, bReverseQuad, last_tile_ptr);
		if (h != -1) return h;
	}

	float world_x = x - m_xmf4x4World._41;
	float world_z = z - m_xmf4x4World._43;

	if (x >= Area_LT.x && x < Area_RB.x && z >= Area_LT.y && z < Area_RB.y)
	{
		last_tile_ptr = this;
		return m_pMesh ? m_pMesh->Get_Height(world_x, world_z) : -1;
	}

	if (Get_Sibling())
	{
		CGameObject* sibling_ptr = Get_Sibling().get();
		float h = ((CHeightMapTerrain*)sibling_ptr)->Get_Mesh_Height(x, z, bReverseQuad, last_tile_ptr);
		if (h != -1) return h;
	}

	return -1.0f;

}

XMFLOAT3 CHeightMapTerrain::Get_Mesh_Normal(float x, float z)
{
	CHeightMapTerrain* last_tile_ptr = nullptr;
	return Get_Mesh_Normal(x, z, last_tile_ptr);
}

XMFLOAT3 CHeightMapTerrain::Get_Mesh_Normal(float x, float z, CHeightMapTerrain*& last_tile_ptr)
{
	if (last_tile_ptr != NULL)
	{
		if (x >= last_tile_ptr->Area_LT.x && x < last_tile_ptr->Area_RB.x &&
			z >= last_tile_ptr->Area_LT.y && z < last_tile_ptr->Area_RB.y)
		{
			return last_tile_ptr->Get_Mesh_Normal(x, z);
		}
	}


	x -= m_xmf4x4World._41;
	z -= m_xmf4x4World._43;



	if (x >= Area_LT.x && x < Area_RB.x && z >= Area_LT.y && z < Area_RB.y)
	{
		CGameObject* child_ptr = Get_Child().get();
		if (child_ptr)
			return ((CHeightMapTerrain*)child_ptr)->Get_Mesh_Normal(x, z);
		else
		{
			return m_pMesh->Get_Normal(x, z);
		}
	}
	else
	{
		CGameObject* sibling_ptr = Get_Sibling().get();
		if (sibling_ptr)
			return ((CHeightMapTerrain*)sibling_ptr)->Get_Mesh_Normal(x, z);
	}

	return XMFLOAT3(0.0f, 1.0f, 0.0f);
}

int CHeightMapTerrain::Get_Tile(float x, float z)
{
	CHeightMapTerrain* last_tile_ptr = nullptr;
	return Get_Tile(x, z, last_tile_ptr);
}

int CHeightMapTerrain::Get_Tile(float x, float z, CHeightMapTerrain*& last_tile_ptr)
{
	if (last_tile_ptr != NULL)
	{
		if (x >= last_tile_ptr->Area_LT.x && x < last_tile_ptr->Area_RB.x &&
			z >= last_tile_ptr->Area_LT.y && z < last_tile_ptr->Area_RB.y)
		{
			return last_tile_ptr->Get_Tile(x, z);
		}
	}

	x -= m_xmf4x4World._41;
	z -= m_xmf4x4World._43;

	if (x >= Area_LT.x && x < Area_RB.x && z >= Area_LT.y && z < Area_RB.y)
	{
		CGameObject* child_ptr = Get_Child().get();
		if (child_ptr)
			return child_ptr->Get_Tile(x, z);
		else
			return tile_number;

	}
	else
	{
		CGameObject* sibling_ptr = Get_Sibling().get();
		if (sibling_ptr)
			return sibling_ptr->Get_Tile(x, z);
	}


	return -1;
}

void CHeightMapTerrain::Get_Active_TileNum_List(std::vector<int>& tile_list)
{
	if (Get_Active())
		tile_list.push_back(tile_number);

	CGameObject* child_ptr = Get_Child().get();
	if (child_ptr)
		child_ptr->Get_Active_TileNum_List(tile_list);

	CGameObject* sibling_ptr = Get_Sibling().get();
	if (sibling_ptr)
		sibling_ptr->Get_Active_TileNum_List(tile_list);
}

BoundingOrientedBox* CHeightMapTerrain::Get_Collider()
{
	if (m_pMesh == NULL)
		return NULL;
	BoundingOrientedBox* pOriginalBoundingBox = m_pMesh->Get_BoundingBox();
	if (pOriginalBoundingBox == NULL)
		return NULL;



	BoundingOrientedBox pWorldBoundingBox(*pOriginalBoundingBox);

	if (pWorldBoundingBox.Extents.x == 0.0f)
		pWorldBoundingBox.Extents.x = 1.0f;
	if (pWorldBoundingBox.Extents.y == 0.0f)
		pWorldBoundingBox.Extents.y = 1.0f;
	if (pWorldBoundingBox.Extents.z == 0.0f)
		pWorldBoundingBox.Extents.z = 1.0f;


	XMVECTOR quaternionRotation = XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_xmf4x4World));
	XMStoreFloat4(&pWorldBoundingBox.Orientation, quaternionRotation);

	return &pWorldBoundingBox;
}

void CHeightMapTerrain::Reset_Obj_List_Height(std::vector<std::shared_ptr<CGameObject>> obj_list)
{
	for (std::shared_ptr<CGameObject> obj_ptr : obj_list)
	{
		XMFLOAT3 pos = obj_ptr->GetPosition();
		float height = Get_Mesh_Height(pos.x, pos.z);
		float diff = obj_ptr->Get_Root_Obj_Displacement().y;
		XMFLOAT3 new_pos = { pos.x,  height + diff, pos.z };
		obj_ptr->Modify_World_Position(new_pos);
	}
}

void CHeightMapTerrain::Reset_Obj_List_Up_Vector(std::vector<std::shared_ptr<CGameObject>> obj_list)
{
	for (std::shared_ptr<CGameObject> obj_ptr : obj_list)
	{
		XMFLOAT3 pos = obj_ptr->GetPosition();
		XMFLOAT3 new_normal = Get_Mesh_Normal(pos.x, pos.z);
		obj_ptr->Modify_World_Up_Vector(new_normal);
	}
}

void CHeightMapTerrain::Check_Culling(CCamera* pCamera)
{
	if (!IsVisible(pCamera))
		Set_Active(false);
	else
		Set_Active(true);


	std::shared_ptr<CGameObject> pChild = Get_Child();
	if (pChild)
		pChild->Check_Culling(pCamera);


	std::shared_ptr<CGameObject> pSibling = Get_Sibling();
	if (pSibling)
		pSibling->Check_Culling(pCamera);

}

void CHeightMapTerrain::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (!Get_Active()) return;

	if (m_pFullMesh && m_TerrainMaterial && m_TerrainMaterial->m_pShader)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
		m_TerrainMaterial->UpdateShaderVariable(pd3dCommandList);
		m_TerrainMaterial->m_pShader->Setting_Render(pd3dCommandList, 0);
		m_pFullMesh->Render(pd3dCommandList, 0);
	}

	if (Get_Child())    Get_Child()->Render(pd3dCommandList, pCamera);
	if (Get_Sibling())  Get_Sibling()->Render(pd3dCommandList, pCamera);
}


void CHeightMapTerrain::Render_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (Get_Active() && m_pFullMesh != NULL)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (m_TerrainMaterial && m_TerrainMaterial->m_pShader)
		{
			m_TerrainMaterial->UpdateShaderVariable(pd3dCommandList);

			m_TerrainMaterial->m_pShader->Setting_Render(pd3dCommandList, 1);

			m_pFullMesh->Render(pd3dCommandList, 0);

		}
	}

}



Deferred_Plane_Shader* Plane_Object::deferred_plane_shader = NULL;
Plane_Shader* Plane_Object::plane_shader = NULL; 

Plane_Object::Plane_Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, int nLength, XMFLOAT4 xmf4Color)
{
	if (deferred_plane_shader == nullptr)
	{
		deferred_plane_shader = new Deferred_Plane_Shader();
		deferred_plane_shader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, RenderTarget_Config::RTV_FORMAT_num, RenderTarget_Config::RTV_FORMATS, RenderTarget_Config::DSV_FORMAT);
		deferred_plane_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

		CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	int side_vertex_n = 10;
//	PlaneMesh* plane_mesh = new PlaneMesh(pd3dDevice, pd3dCommandList, nLength, side_vertex_n);
	shared_ptr<PlaneMesh> plane_mesh = make_shared<PlaneMesh>(pd3dDevice, pd3dCommandList, nLength, side_vertex_n);
	SetMesh(plane_mesh);

	Plane_Material = new CMaterial(2);

	Plane_Material->SetShader(deferred_plane_shader);
}

Plane_Object::~Plane_Object()
{
}

void Plane_Object::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (Get_Active() && m_pMesh != NULL)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (Plane_Material && Plane_Material->m_pShader)
		{
			Plane_Material->UpdateShaderVariable(pd3dCommandList);

			Plane_Material->m_pShader->Setting_Render(pd3dCommandList, 0);
			m_pMesh->Render(pd3dCommandList, 0);
		}
	}

	if (Get_Active())
	{
		std::shared_ptr<CGameObject> pChild = Get_Child();
		if (pChild) pChild->Render(pd3dCommandList, pCamera);
	}

	std::shared_ptr<CGameObject> pSibling = Get_Sibling();
	if (pSibling) pSibling->Render(pd3dCommandList, pCamera);

}

void Plane_Object::Render_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (Get_Active() && m_pMesh != NULL)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (Plane_Material && Plane_Material->m_pShader)
		{
			Plane_Material->m_pShader->Setting_Render(pd3dCommandList, 1);
			m_pMesh->Render(pd3dCommandList, 0);
		}
	}

	if (Get_Active())
	{
		std::shared_ptr<CGameObject> pChild = Get_Child();
		if (pChild) pChild->Render_Shadow(pd3dCommandList, pCamera);
	}

	std::shared_ptr<CGameObject> pSibling = Get_Sibling();
	if (pSibling) pSibling->Render_Shadow(pd3dCommandList, pCamera);

}


void Plane_Object::Set_BaseTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* filename)
{
	if (filename == NULL)
		return;

	Plane_BaseTexture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
	Plane_BaseTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, filename, RESOURCE_TEXTURE2D, 0);

	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, Plane_BaseTexture.get(), 0, ROOT_PARAMETER_PLANE_BASE_TEXTURE_INDEX);

	Plane_Material->SetTexture(Plane_BaseTexture, 0);
}

void Plane_Object::Set_DetailTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* filename)
{
	if (filename == NULL)
		return;

	Plane_DetailTexture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
	Plane_DetailTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, filename, RESOURCE_TEXTURE2D, 0);

	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, Plane_DetailTexture.get(), 0, ROOT_PARAMETER_PLANE_DETAIL_TEXTURE_INDEX);

	Plane_Material->SetTexture(Plane_DetailTexture, 1);

}

CS_Wave_Shader* Wave_Object::cs_wave_shader = NULL;

Wave_Object::Wave_Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature, int nLength, int side_vertex_n, bool use_deferred_shader)
	: Plane_Object()
{
	if (use_deferred_shader && deferred_plane_shader == nullptr)
	{
		deferred_plane_shader = new Deferred_Plane_Shader();
		deferred_plane_shader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, RenderTarget_Config::RTV_FORMAT_num, RenderTarget_Config::RTV_FORMATS, RenderTarget_Config::DSV_FORMAT);
		deferred_plane_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	if (!use_deferred_shader && plane_shader == nullptr)
	{
		plane_shader = new Plane_Shader();
		plane_shader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		plane_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	if (cs_wave_shader == nullptr)
	{
		cs_wave_shader = new CS_Wave_Shader();
		cs_wave_shader->CreateShader(pd3dDevice);
		cs_wave_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	}

	Plane_Material = new CMaterial(2);
	if(use_deferred_shader)
		Plane_Material->SetShader(deferred_plane_shader);
	else if(!use_deferred_shader)
		Plane_Material->SetShader(plane_shader);


	Light_Material_Info temp;
	temp.gSpecular = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	temp.gEmissive = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	UINT ID = Light_Material_Manager::Add_Material(temp);
	Plane_Material->m_Material_ID = ID;

	Side_Length = nLength;
	constexpr int kMaxTextureSize = 15000;
	desiredTexelSize = 20.0f;

	shared_ptr<PlaneMesh> plane_mesh = make_shared<PlaneMesh>(pd3dDevice, pd3dCommandList, nLength, side_vertex_n);
	SetMesh(plane_mesh);

	int tex_Length = static_cast<int>(ceil(nLength / desiredTexelSize));

	if (tex_Length > kMaxTextureSize)
	{
		tex_Length = kMaxTextureSize;
		desiredTexelSize = static_cast<float>(nLength) / tex_Length;
	}

	Tex_Length = tex_Length;




	wave_data_texture = new CTexture(
		4,                 // 0: HeightMap_A, 1: HeightMap_B, 2: NormalMap, 
		RESOURCE_TEXTURE2D,
		0,                 // No samplers
		4,                 // Graphics RootParameters: HeightMap (pinged), NormalMap
		4,                 // Compute UAV RootParameter
		4,                 // Compute SRV RootParameter
		4,                 // Graphics SRV handles
		4,                 // Compute UAV handles
		4                  // Compute SRV handles
	);

	// HeightMap_Read: index 0 (read-only SRV)
	wave_data_texture->CreateTexture(pd3dDevice, pd3dCommandList, 0, RESOURCE_TEXTURE2D, tex_Length, tex_Length, 1, 1, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr);

	// HeightMap_Write: index 1 (UAV + SRV)
	wave_data_texture->CreateTexture(pd3dDevice, pd3dCommandList, 1, RESOURCE_TEXTURE2D, tex_Length, tex_Length, 1, 1, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr);

	// NormalMap: index 2 (UAV + SRV)
	wave_data_texture->CreateTexture(pd3dDevice, pd3dCommandList, 2, RESOURCE_TEXTURE2D, tex_Length, tex_Length, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr);

	// Pos_Normal: index 3 (UAV)
	wave_data_texture->CreateStructuredBuffer(pd3dDevice, pd3dCommandList, 3, nullptr, 4, sizeof(float), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Pos_Normal_ReadBack_buffer = Create_Control_Buffer(pd3dDevice, BUFFER_READBACK, sizeof(UINT) * 4);


	// HeightMap_A: index 0
	CDescriptor_Heap::CreateComputeShaderResourceView(pd3dDevice, wave_data_texture, 0, 1); // RootParam[1] - SRV(t0)
	CDescriptor_Heap::CreateComputeUnorderedAccessView(pd3dDevice, wave_data_texture, 0, 2); // RootParam[2] - UAV(u0)
	CDescriptor_Heap::CreateGraphicsShaderResourceView(pd3dDevice, wave_data_texture, 0, 5); // Graphics RootParam[0] (t#)

	// HeightMap_B: index 1
	CDescriptor_Heap::CreateComputeShaderResourceView(pd3dDevice, wave_data_texture, 1, 1); // SRV(t0)
	CDescriptor_Heap::CreateComputeUnorderedAccessView(pd3dDevice, wave_data_texture, 1, 2); // UAV(u0)
	CDescriptor_Heap::CreateGraphicsShaderResourceView(pd3dDevice, wave_data_texture, 1, 5); // Graphics RootParam[0]

	// NormalMap: index 2
	CDescriptor_Heap::CreateComputeUnorderedAccessView(pd3dDevice, wave_data_texture, 2, 3); // UAV(u1)
	CDescriptor_Heap::CreateGraphicsShaderResourceView(pd3dDevice, wave_data_texture, 2, 6); // Graphics RootParam[1]

	// Pos_Normal: index 2
	CDescriptor_Heap::CreateComputeUnorderedAccessView(pd3dDevice, wave_data_texture, 3, 4); // UAV(u2)

}

Wave_Object::~Wave_Object()
{
}

void Wave_Object::Copy_Buffer_Data(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->CopyBufferRegion(Pos_Normal_ReadBack_buffer, 0, wave_data_texture->GetResource(3), 0, sizeof(UINT) * 4);
}

XMFLOAT3 Wave_Object::Readback_Buffer_Data()
{
	if (!Pos_Normal_ReadBack_buffer)
		return XMFLOAT3{ 0.0f,0.0f,0.0f };

	float* pReadData = nullptr;
	D3D12_RANGE readRange = { 0, sizeof(float) * 4 }; // 4개의 float (16바이트)

	if (SUCCEEDED(Pos_Normal_ReadBack_buffer->Map(0, &readRange, reinterpret_cast<void**>(&pReadData))) && pReadData)
	{
		BoatPos_WaveNormal = XMFLOAT3(pReadData[0], pReadData[1], pReadData[2]);
		BoatPos_WaveHeight = pReadData[3];

		D3D12_RANGE writtenRange = { 0, 0 };
		Pos_Normal_ReadBack_buffer->Unmap(0, nullptr);
		return BoatPos_WaveNormal;

	}
	else
	{
		DebugOutput(" Failed to map debug readback buffer.\n");
	}

	return XMFLOAT3{ 0.0f,0.0f,0.0f };

}

void Wave_Object::Synchronize_Wave_to_Boat(Boat_Object* boat_ptr)
{
	World_Boat_Pos = boat_ptr->GetPosition();
	World_Boat_Dir = Vector3::ScalarProduct(boat_ptr->GetLook(), -1.0f, false);

	XMFLOAT3 boat_velocity = boat_ptr->Get_Velocity();
	float boat_rotation_speed = boat_ptr->Get_RotationSpeed();

	float speed = XMVectorGetX(XMVector3Length(XMLoadFloat3(&boat_velocity)));

	float absRotation = fabsf(boat_rotation_speed);
	float rotationFactor = 1.0f - (absRotation / 90.0f);
	rotationFactor = std::clamp(rotationFactor, 0.5f, 1.0f);

	World_Boat_Velocity = speed * rotationFactor;


	if (!IsZeroVector(BoatPos_WaveNormal))
	{
		BoatPos_WaveNormal = Vector3::Normalize(BoatPos_WaveNormal);
		boat_ptr->Set_Wave_Normal(BoatPos_WaveNormal);
		boat_ptr->Set_Wave_Height(BoatPos_WaveHeight);
	}
}

void Wave_Object::Animate(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
	if (!cs_wave_shader) return;

	// Set compute root signature
	pd3dCommandList->SetComputeRootSignature(cs_wave_shader->Wave_ComputeRootSignature_ptr);

	// Step 1: Update global simulation time
	CS_Wave_Shader::total_time += fTimeElapsed;
	if (CS_Wave_Shader::total_time >= XM_2PI)
		CS_Wave_Shader::total_time -= XM_2PI;

	cs_wave_shader->update_wave_info->g_TotalTime = CS_Wave_Shader::total_time;

	// Step 2: Prepare dispatch group sizes
	const int readIndex = bPingPongToggle ? 1 : 0;
	const int writeIndex = bPingPongToggle ? 0 : 1;
	const UINT threadSize = 8;
	const UINT n = static_cast<UINT>(ceil(Tex_Length / float(threadSize)));

	// Step 3: Dispatch Global Wave Pass (runs unconditionally)
	cs_wave_shader->OnPrepareDispatch(pd3dCommandList, 0);
	wave_data_texture->BindComputeSrvToRootParameter(pd3dCommandList, 1, readIndex);     // SRV: previous heightmap
	wave_data_texture->BindComputeUavToRootParameter(pd3dCommandList, 2, writeIndex);    // UAV: write new heightmap
	cs_wave_shader->UpdateShaderVariables(pd3dCommandList);
	cs_wave_shader->Dispatch(pd3dCommandList, n, n, 1);

	// Step 4: Insert UAV barrier after global wave pass
	D3D12_RESOURCE_BARRIER uavBarrier0 = {};
	uavBarrier0.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier0.UAV.pResource = wave_data_texture->GetResource(writeIndex);
	pd3dCommandList->ResourceBarrier(1, &uavBarrier0);

	// Step 5: Check boat direction validity
	if (World_Boat_Dir.x == 0 && World_Boat_Dir.z == 0)
	{
		bPingPongToggle = !bPingPongToggle;
		return;
	}

	// Step 6: Update boat-related parameters
	XMFLOAT3 Plane_Position = GetPosition();
	float planeHalfSize = Side_Length * 0.5f;

	XMFLOAT2 boatTexel = {
		(World_Boat_Pos.x - (Plane_Position.x - planeHalfSize)) / desiredTexelSize,
		(World_Boat_Pos.z - (Plane_Position.z - planeHalfSize)) / desiredTexelSize
	};

	XMFLOAT2 dirXZ = { World_Boat_Dir.x, World_Boat_Dir.z };
	XMVECTOR v = XMVector2Normalize(XMLoadFloat2(&dirXZ));
	XMFLOAT2 normDirXZ;
	XMStoreFloat2(&normDirXZ, v);

	cs_wave_shader->update_wave_info->g_BoatPos = boatTexel;
	cs_wave_shader->update_wave_info->g_BoatDir = normDirXZ;
	cs_wave_shader->update_wave_info->g_WakeMaxDist = World_Boat_Velocity;
	cs_wave_shader->UpdateShaderVariables(pd3dCommandList);

	// Step 7: Dispatch Boat Wake Pass
	cs_wave_shader->OnPrepareDispatch(pd3dCommandList, 1);
	wave_data_texture->BindComputeSrvToRootParameter(pd3dCommandList, 1, writeIndex);
	wave_data_texture->BindComputeUavToRootParameter(pd3dCommandList, 2, readIndex);
	cs_wave_shader->Dispatch(pd3dCommandList, n, n, 1);

	// Step 8: UAV barrier after wake pass
	D3D12_RESOURCE_BARRIER uavBarrier1 = {};
	uavBarrier1.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier1.UAV.pResource = wave_data_texture->GetResource(readIndex);
	pd3dCommandList->ResourceBarrier(1, &uavBarrier1);

	// Step 9: Dispatch Normal Map Generation Pass
	cs_wave_shader->OnPrepareDispatch(pd3dCommandList, 2);
	wave_data_texture->BindComputeSrvToRootParameter(pd3dCommandList, 1, readIndex);
	wave_data_texture->BindComputeUavToRootParameter(pd3dCommandList, 2, writeIndex);
	wave_data_texture->BindComputeUavToRootParameter(pd3dCommandList, 3, 2); // NormalMap
	wave_data_texture->BindComputeUavToRootParameter(pd3dCommandList, 4, 3); // Position + Normal buffer
	cs_wave_shader->Dispatch(pd3dCommandList, n, n, 1);

	// Step 10: Toggle ping-pong state
	bPingPongToggle = !bPingPongToggle;
}

void Wave_Object::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	int renderHeightMapIndex = bPingPongToggle ? 1 : 0;

	wave_data_texture->BindGraphicsSrvToRootParameter(pd3dCommandList, 5, renderHeightMapIndex);

	if (Get_Active() && m_pMesh != NULL)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
		wave_data_texture->UpdateGraphicsSrvShaderVariables(pd3dCommandList);


		if (Plane_Material && Plane_Material->m_pShader)
		{
			Plane_Material->m_pShader->Setting_Render(pd3dCommandList, 0);
			Plane_Material->m_pShader->UpdateShaderVariables(pd3dCommandList);
			Plane_Material->UpdateShaderVariable(pd3dCommandList);
			m_pMesh->Render(pd3dCommandList, 0);
		}
	}

	if (Get_Active())
	{
		std::shared_ptr<CGameObject> pChild = Get_Child();
		if (pChild) pChild->Render(pd3dCommandList, pCamera);
	}

	std::shared_ptr<CGameObject> pSibling = Get_Sibling();
	if (pSibling) pSibling->Render(pd3dCommandList, pCamera);

}

void Wave_Object::Render_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	int renderHeightMapIndex = bPingPongToggle ? 1 : 0;

	wave_data_texture->BindGraphicsSrvToRootParameter(pd3dCommandList, 5, renderHeightMapIndex);

	if (Get_Active() && m_pMesh != NULL)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
		wave_data_texture->UpdateGraphicsSrvShaderVariables(pd3dCommandList);


		if (Plane_Material && Plane_Material->m_pShader)
		{
			Plane_Material->m_pShader->Setting_Render(pd3dCommandList, 1);
			Plane_Material->m_pShader->UpdateShaderVariables(pd3dCommandList);
			m_pMesh->Render(pd3dCommandList, 0);
		}
	}

	if (Get_Active())
	{
		std::shared_ptr<CGameObject> pChild = Get_Child();
		if (pChild) pChild->Render_Shadow(pd3dCommandList, pCamera);
	}

	std::shared_ptr<CGameObject> pSibling = Get_Sibling();
	if (pSibling) pSibling->Render_Shadow(pd3dCommandList, pCamera);

}

Boat_Object::Boat_Object() : CGameObject(1)
{
	m_fMaxVelocityXZ = 200.0f;
	m_xmf3Velocity.x = 300.0f;
	m_xmf3Velocity.y = 0.0f;
	m_xmf3Velocity.z = 0.0f;
	m_fFriction = 50.0f;
}

Boat_Object::~Boat_Object()
{

}

void Boat_Object::Move(float fSpeed, bool bUpdateVelocity)
{
	XMFLOAT3 look = Vector3::Normalize(GetLook());
	XMFLOAT3 shift = Vector3::ScalarProduct(look, fSpeed, false);

	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, shift);
	}
	else
	{
		XMFLOAT3 position = GetPosition();
		position = Vector3::Add(position, shift);
		SetPosition(position);
	}
}

void Boat_Object::MoveForward(float speed)
{
	XMFLOAT3 look = Vector3::Normalize(GetLook());
	XMFLOAT3 shift = Vector3::ScalarProduct(look, speed, false);
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, shift);
}

void Boat_Object::Yaw(float angle)
{
	XMFLOAT3 up = Vector3::Normalize(GetUp());
	XMMATRIX mRotate = XMMatrixRotationAxis(XMLoadFloat3(&up), XMConvertToRadians(angle));


	m_xmf4x4Parent = Matrix4x4::Multiply(mRotate, m_xmf4x4Parent);
	UpdateTransform(nullptr);
}

void Boat_Object::Add_Rotate(float angleDelta)
{
	m_fRotationSpeed += angleDelta;
	m_fRotationSpeed = std::clamp<float>(m_fRotationSpeed, -45.0f, 45.0f);
}

void Boat_Object::UpdateRotationFromWave(float fTimeElapsed)
{
	const XMFLOAT3 worldUp = XMFLOAT3(0.0f, 1.0f, 0.0f);

	float normalFollowSpeed = 0.3f;
	float restoreUpSpeed = 1.0f;

	float normalFollowWeight = 1.0f - expf(-normalFollowSpeed * fTimeElapsed);
	float restoreToUpWeight = 1.0f - expf(-restoreUpSpeed * fTimeElapsed);

	if (!IsZeroVector(wave_normal_vector))
	{
		XMFLOAT3 waveUp = Vector3::Normalize(wave_normal_vector);
		boat_up_vector = Lerp(boat_up_vector, waveUp, normalFollowWeight);
	}

	boat_up_vector = Lerp(boat_up_vector, worldUp, restoreToUpWeight);
	boat_up_vector = Vector3::Normalize(boat_up_vector);

	AlignWithNormal(boat_up_vector);
}

void Boat_Object::UpdateMovementOnWave(float fTimeElapsed)
{
	// --- 속도 제한 ---
	float velocityFull = Vector3::Length(m_xmf3Velocity);
	if (velocityFull > m_fMaxVelocityXZ)
	{
		float scale = m_fMaxVelocityXZ / velocityFull;
		m_xmf3Velocity.x *= scale;
		m_xmf3Velocity.z *= scale;
	}

	// --- Look 벡터 방향으로 이동 처리 ---
	XMFLOAT3 lookDir = Vector3::Normalize(GetLook());
	XMFLOAT3 velocityXZ = Vector3::ScalarProduct(lookDir, velocityFull, false);

	XMFLOAT3 pos = GetPosition();
	XMFLOAT3 deltaMove = Vector3::ScalarProduct(velocityXZ, fTimeElapsed, false);
	XMFLOAT3 newPos = Vector3::Add(pos, deltaMove);

	// --- 부드러운 높이 보정 ---

	smoothedHeight = std::lerp(smoothedHeight, wave_height, 0.1f);
	newPos.y = smoothedHeight * 30.0f;

	SetPosition(newPos);

	// --- 감속 처리 (속도 크기 감소만)
	float fDeceleration = m_fFriction * fTimeElapsed;
	if (fDeceleration > velocityFull) fDeceleration = velocityFull;

	velocityFull -= fDeceleration;

	// 감속된 속도를 Look 방향에 재적용
	XMFLOAT3 newVelocity = Vector3::ScalarProduct(lookDir, velocityFull, false);
	m_xmf3Velocity = XMFLOAT3(newVelocity.x, 0.0f, newVelocity.z);
}

void Boat_Object::Animate(float fTimeElapsed)
{
	UpdateRotationFromWave(fTimeElapsed);
	UpdateMovementOnWave(fTimeElapsed);
	CGameObject::Rotate(&boat_up_vector, m_fRotationSpeed * fTimeElapsed);

	m_fRotationSpeed = std::lerp(m_fRotationSpeed, 0.0f, 0.01f);

	XMFLOAT3 localFixedZAxis = { 0.0f, 0.0f, 1.0f };

	if (fabsf(m_fRotationSpeed) > 5.0f)
	{
		float effectiveSpeed = (fabsf(m_fRotationSpeed) - 5.0f) * 2.0f;
		float rotationDirection = m_fRotationSpeed / fabsf(m_fRotationSpeed);
		Boat_Frames_Marker["Captain_Wheel"]->Rotate(&localFixedZAxis, rotationDirection * (15.0f + effectiveSpeed) * fTimeElapsed);
	}

}

void Boat_Object::HandleBoundaryReflection(float boundary)
{
	XMFLOAT3 pos = GetPosition();
	XMFLOAT3 velocity = Get_Velocity();

	bool bounced = false;

	if (pos.x > boundary || pos.x < -boundary) {
		velocity.x *= -1.0f;
		bounced = true;
	}
	if (pos.z > boundary || pos.z < -boundary) {
		velocity.z *= -1.0f;
		bounced = true;
	}

	if (bounced) {
		Set_Velocity(velocity);
		XMFLOAT3 new_dir = Vector3::Normalize(velocity);
		Set_LookDirection_LookAt(new_dir);
	}
}

bool Boat_Object::GetMarkerWorldPosition(const std::string& name, XMFLOAT3& outWorldPos)
{
	auto it = Boat_Frames_Marker.find(name);
	if (it == Boat_Frames_Marker.end() || !it->second) return false;

	outWorldPos = it->second->GetPosition();
	return true;
}

bool Boat_Object::Is_Moving()
{
	if (5.0f < Vector3::Length(m_xmf3Velocity))
		return true;
	return false;
}

void Boat_Object::Change_Model(bool is_stay_mode)
{
	if (is_stay_mode)
	{
		Boat_Frames_Marker["Move_Model_1"]->Set_Active(false);
		Boat_Frames_Marker["Move_Model_2"]->Set_Active(false);
		Boat_Frames_Marker["Move_Model_3"]->Set_Active(false);
		Boat_Frames_Marker["Move_Model_4"]->Set_Active(false);
		Boat_Frames_Marker["Move_Model_5"]->Set_Active(false);

		Boat_Frames_Marker["Stay_Model_1"]->Set_Active(true);
		Boat_Frames_Marker["Stay_Model_2"]->Set_Active(true);
		Boat_Frames_Marker["Stay_Model_3"]->Set_Active(true);
		Boat_Frames_Marker["Stay_Model_4"]->Set_Active(true);
	}
	else
	{
		Boat_Frames_Marker["Move_Model_1"]->Set_Active(true);
		Boat_Frames_Marker["Move_Model_2"]->Set_Active(true);
		Boat_Frames_Marker["Move_Model_3"]->Set_Active(true);
		Boat_Frames_Marker["Move_Model_4"]->Set_Active(true);
		Boat_Frames_Marker["Move_Model_5"]->Set_Active(true);

		Boat_Frames_Marker["Stay_Model_1"]->Set_Active(false);
		Boat_Frames_Marker["Stay_Model_2"]->Set_Active(false);
		Boat_Frames_Marker["Stay_Model_3"]->Set_Active(false);
		Boat_Frames_Marker["Stay_Model_4"]->Set_Active(false);
	}

}


//=====================================================================================
CSkyBoxShader* CSkyBox::pSkybox_shader = NULL;
shared_ptr<CSkyBoxMesh> CSkyBox::pSkyBoxMesh = NULL;

void CSkyBox::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature)
{
	if (pSkybox_shader == NULL)
	{
		pSkybox_shader = new CSkyBoxShader();
		pSkybox_shader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature.get());
		pSkybox_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}
}


CSkyBox::CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	: CGameObject(1)
{
	if (pSkybox_shader == NULL)
	{
		DebugOutput("No shader for Builing Skybox\n");
		return;
	}
	else if (pSkyBoxMesh == NULL)
	{
		pSkyBoxMesh = make_shared<CSkyBoxMesh>(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 20.0f);
	}

	SetMesh(pSkyBoxMesh);

	skybox_material = make_shared<CMaterial>(1);
	skybox_material->SetShader(pSkybox_shader);

}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Set_BaseTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* filename)
{
	if (filename == NULL)
		return;

	if (skybox_texture)
		skybox_texture->Release();

	skybox_texture = make_shared<CTexture>(1, RESOURCE_TEXTURE_CUBE, 0, 1, 0, 0, 1, 0, 0);
	skybox_texture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, filename, RESOURCE_TEXTURE_CUBE, 0);

	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, skybox_texture.get(), 0, ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX);


	skybox_material->SetTexture(skybox_texture, 0);
}

void CSkyBox::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	if (Get_Active() && m_pMesh != NULL)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (skybox_material && skybox_material->m_pShader)
		{
			skybox_material->m_pShader->Setting_Render(pd3dCommandList, 0);
			skybox_material->m_pShader->UpdateShaderVariables(pd3dCommandList);
			skybox_material->UpdateShaderVariable(pd3dCommandList);
			m_pMesh->Render(pd3dCommandList, 0);
		}
	}

}

//=====================================================================================

Trail_Object::Trail_Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4 main_color)
{
	trail_mesh = new Trail_Mesh(pd3dDevice, pd3dCommandList, main_color, 64); // N = length,  N * 32
	m_fAccumulatedTime = 0.0f;
}

Trail_Object::~Trail_Object()
{

}

void Trail_Object::Animate(float fTimeElapsed)
{
	if (!m_pTargetObject)
		return;

	m_fAccumulatedTime += fTimeElapsed;
	m_fSegmentTimer += fTimeElapsed;

	if (m_fSegmentTimer < m_fSegmentInterval)
		return;
	
	m_fSegmentTimer = 0.0f;

	XMMATRIX worldMatrix = XMLoadFloat4x4(&m_pTargetObject->m_xmf4x4World);

	XMFLOAT3 top, bottom;

	if (m_bUseTargetScale)
	{
		// apply parent's scale value
		XMVECTOR worldTop = XMVector3TransformCoord(XMLoadFloat3(&m_vLocalTop), worldMatrix);
		XMVECTOR worldBottom = XMVector3TransformCoord(XMLoadFloat3(&m_vLocalBottom), worldMatrix);
		XMStoreFloat3(&top, worldTop);
		XMStoreFloat3(&bottom, worldBottom);
	}
	else
	{
		// not apply parent's scale value
		XMVECTOR right = XMVector3Normalize(worldMatrix.r[0]);
		XMVECTOR up = XMVector3Normalize(worldMatrix.r[1]);
		XMVECTOR look = XMVector3Normalize(worldMatrix.r[2]);
		XMVECTOR trans = worldMatrix.r[3];

		XMMATRIX rotationMatrix = XMMATRIX(right, up, look, XMVectorSet(0, 0, 0, 1));

		XMVECTOR localTopVec = XMVectorSet(m_vLocalTop.x, m_vLocalTop.y, m_vLocalTop.z, 0.0f);
		XMVECTOR localBottomVec = XMVectorSet(m_vLocalBottom.x, m_vLocalBottom.y, m_vLocalBottom.z, 0.0f);

		XMVECTOR worldTop = XMVectorAdd(trans, XMVector3TransformNormal(localTopVec, rotationMatrix));
		XMVECTOR worldBottom = XMVectorAdd(trans, XMVector3TransformNormal(localBottomVec, rotationMatrix));

		XMStoreFloat3(&top, worldTop);
		XMStoreFloat3(&bottom, worldBottom);
	}

	trail_mesh->AddSegment(top, bottom, m_fAccumulatedTime);
	trail_mesh->UpdateTrail(m_fAccumulatedTime);
	trail_mesh->UpdateIndexBuffer();
	trail_mesh->UpdateVertexBuffer();
}

void Trail_Object::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
	trail_mesh->Render(pd3dCommandList, 0);
}

CMonsterObject::~CMonsterObject()
{
}

void CMonsterObject::Animate(float fTimeElapsed)
{
	OnPrepareAnimate();

	if (m_pSkinnedAnimationController)
	{
		m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);
	}

	/*if (On_Ground)
	{
		CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
		XMFLOAT3 xmf3PlayerPosition = GetPosition();
		XMFLOAT3 world_normal = pTerrain->Get_Mesh_Normal(xmf3PlayerPosition.x, xmf3PlayerPosition.z, last_tile_ptr);
		AlignWithNormal(world_normal);
	}*/

	shared_ptr<CGameObject> sibling_ptr = Get_Sibling();
	if (sibling_ptr != nullptr)
		sibling_ptr->Animate(fTimeElapsed);

	shared_ptr<CGameObject> child_ptr = Get_Child();
	if (child_ptr != nullptr)
		child_ptr->Animate(fTimeElapsed);

	GetStateMachine()->update(fTimeElapsed);
}

void CMonsterObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) 
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

void CMonsterObject::Render_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render_Shadow(pd3dCommandList, pCamera);

}
void CMonsterObject::SetupWeaponCollider()
{
	std::shared_ptr<CGameObject> model = FindFrame(WeaponName);

	if (!model || !model->m_pMesh) return;

	if (WeaponName == "HeadA_LP") {
		model->customRotation = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(160.0f),
			XMConvertToRadians(90.0f),
			XMConvertToRadians(0.0f));
	}
	if (WeaponName == "spear_lp") {
		model->customRotation = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(45.0f),
			XMConvertToRadians(30.0f),
			XMConvertToRadians(0.0f));
	}

	model->type = EObjectType::MonsterWeapon;

	XMFLOAT4X4 worldMatrixFloat = model->m_xmf4x4World;
	XMVECTOR scale, rotationQuat, translation;
	XMFLOAT4 quaternion;
	XMMATRIX worldMatrix = XMLoadFloat4x4(&worldMatrixFloat);

	if (XMMatrixDecompose(&scale, &rotationQuat, &translation, worldMatrix))
		XMStoreFloat4(&quaternion, rotationQuat);
	else
		quaternion = XMFLOAT4(0, 0, 0, 1);

	BoundingOrientedBox* obb = new BoundingOrientedBox(
		model->m_pMesh->GetAABBCenter(),
		model->m_pMesh->GetAABBExtents(),
		quaternion
	);

	model->Set_Collider(obb);
	model->bUpdateOBBOff();
	Weapon_ptr = model;

}

void CMonsterObject::ApplySyncData(const ServerSyncData& syncData)
{
	CGameObject::ApplySyncData(syncData);

	auto controller = GetSkinnedAnimationController();
	if (!controller) return;
	controller->ResetWeight();
	auto track = controller->m_pAnimationTracks;
	vector<Animation_Sync> track_list = syncData.track_info_list;

	for (Animation_Sync animation_track_info : track_list)
	{
		track[animation_track_info.track_index].m_fPosition = animation_track_info.track_position;
		track[animation_track_info.track_index].m_fWeight = animation_track_info.weight;
	}

	controller->ApplyCurrentAnimationPose(this);
	//std::cout << "monster aplly, list size - " << track_list.size() << std::endl;
}

///////////////////////////////////////////////////////////////////

CFishManObject::CFishManObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature)
{
	RootMotionTrackSet = {
		TRACK_FISHMAN_WALK,
		TRACK_FISHMAN_WALK_BACK,
		TRACK_FISHMAN_ATTACK1,
		TRACK_FISHMAN_ATTACK2,
		TRACK_FISHMAN_GET_HIT,
		TRACK_FISHMAN_DEAD
	};

	std::unordered_set<int> OnceType = {
		TRACK_FISHMAN_ATTACK1,
		TRACK_FISHMAN_ATTACK2,
		TRACK_FISHMAN_GET_HIT,
		TRACK_FISHMAN_DEAD
	};

	m_StateMachine = std::make_unique<FishManStateMachine>(this);

	type = EObjectType::Monster;

	CLoadedModelInfo* pFishManModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/FishmanLP.bin", NULL);

	n_Animation = 9;
	RootIndex = 0;
	prevWeights.resize(n_Animation, 0.0f);
	targetWeights.resize(n_Animation, 0.0f);
	m_pRootModel = pFishManModel->m_pModelRootObject;
	m_pSkinnedAnimationController = std::make_shared<CAnimationController>(pd3dDevice, pd3dCommandList, n_Animation, pFishManModel);
	m_pSkinnedAnimationController->RootIndex = RootIndex;
	for (int i = 0; i < n_Animation; ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, true);
	}
	for (int i = 0; i < n_Animation; ++i) {
		if (OnceType.contains(i)) {
			m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
		}
	}
	SetScale(10.0f, 10.0f, 10.0f);
	WeaponName = "spear_lp";
	BoundingOrientedBox* body = new BoundingOrientedBox(
		XMFLOAT3(0.0f, 0.8f, 0.0f),  
		XMFLOAT3(0.4f, 0.8f, 0.4f),  
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) 
	);
	Set_Collider(body);

	Set_Name("FishMan");



}

///////////////////////////////////////////////////////////////////

CAnubisObject::CAnubisObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature)
{
	RootMotionTrackSet = {
		TRACK_ANUBIS_IDLE,
		TRACK_ANUBIS_IDLE_BREAK,
		TRACK_ANUBIS_IDLE_TO_ATTACK_IDLE,
		TRACK_ANUBIS_WALK,
		TRACK_ANUBIS_BACK_WALK,
		TRACK_ANUBIS_ATTACK1,
		TRACK_ANUBIS_ATTACK2,
		TRACK_ANUBIS_SKILL,
		TRACK_ANUBIS_GET_HIT,
		TRACK_ANUBIS_DEAD
	};

	std::unordered_set<int> OnceType = {
		TRACK_ANUBIS_ATTACK1,
		TRACK_ANUBIS_ATTACK2,
		TRACK_ANUBIS_SKILL,
		TRACK_ANUBIS_GET_HIT,
		TRACK_ANUBIS_DEAD
	};

	n_Animation = 10;
	RootIndex = 0;

	m_StateMachine = std::make_unique<AnubisStateMachine>(this);

	type = EObjectType::Monster;

	CLoadedModelInfo* pAnubisModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Anubis_LP.bin", NULL);

	prevWeights.resize(n_Animation, 0.0f);
	targetWeights.resize(n_Animation, 0.0f);

	m_pRootModel = pAnubisModel->m_pModelRootObject;
	m_pSkinnedAnimationController = std::make_shared<CAnimationController>(pd3dDevice, pd3dCommandList, n_Animation, pAnubisModel);
	m_pSkinnedAnimationController->RootIndex = RootIndex;
	for (int i = 0; i < n_Animation; ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, true);
	}

	for (int i = 0; i < n_Animation; ++i) {
		if (OnceType.contains(i)) {
			m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
		}
	}
	SetScale(15.0f, 15.0f, 15.0f);

	WeaponName = "Staff_LP";
	BoundingOrientedBox* body = new BoundingOrientedBox(
		XMFLOAT3(0.0f, 0.9f, 0.0f),
		XMFLOAT3(0.3f, 0.9f, 0.3f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
	);
	Set_Collider(body);

	Set_Name("Anubis");

}

///////////////////////////////////////////////////////////////////

CDragonObject::CDragonObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature)
{
	RootMotionTrackSet = {
		TRACK_DRAGON_ATTACK1,
		TRACK_DRAGON_RUN,
		TRACK_DRAGON_GOT_HIT1,
		TRACK_DRAGON_GOT_HIT2,
		TRACK_DRAGON_FLY_DIVE,
		TRACK_DRAGON_DEAD
	};

	std::unordered_set<int> OnceType = {
		TRACK_DRAGON_ATTACK1,
		TRACK_DRAGON_DEAD
	};

	n_Animation = 13;
	RootIndex = 16;

	m_StateMachine = std::make_unique<DragonStateMachine>(this);

	type = EObjectType::Monster;

	CLoadedModelInfo* pDragonModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Dragon_LP.bin", NULL);

	prevWeights.resize(n_Animation, 0.0f);
	targetWeights.resize(n_Animation, 0.0f);
	m_pRootModel = pDragonModel->m_pModelRootObject;
	m_pSkinnedAnimationController = std::make_shared<CAnimationController>(pd3dDevice, pd3dCommandList, n_Animation, pDragonModel);
	m_pSkinnedAnimationController->RootIndex = RootIndex;
	for (int i = 0; i < n_Animation; ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, true);
	}

	for (int i = 0; i < n_Animation; ++i) {
		if (OnceType.contains(i)) {
			m_pSkinnedAnimationController->m_pAnimationTracks[i].m_nType = ANIMATION_TYPE_ONCE;
		}
	}
	SetScale(15.0f, 15.0f, 15.0f);

	BoundingOrientedBox* body = new BoundingOrientedBox(
		XMFLOAT3(0.0f, 1.0f, -1.6f),
		XMFLOAT3(0.8f, 1.0f, 2.8f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
	);
	Set_Collider(body);

	Set_Name("Dragon");

	WeaponName = "HeadA_LP";
}