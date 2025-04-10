//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Scene.h"



CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers, 
	int nGraphicsSrvRootParameters, int nComputeUavRootParameters, int nComputeSrvRootParameters, 
	int nGraphicsSrvGpuHandles, int nComputeUavGpuHandles, int nComputeSrvGpuHandles) : m_nTextureType(nTextureType)
{
	m_pnResourceTypes.resize(nTextures, 0);
	m_ppd3dTextures.resize(nTextures, nullptr);
	m_ppd3dTextureUploadBuffers.resize(nTextures, nullptr);
	m_pdxgiBufferFormats.resize(nTextures, DXGI_FORMAT_UNKNOWN);
	m_pnBufferElements.resize(nTextures, 0);
	m_pnBufferStrides.resize(nTextures, 0);

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
		pd3dCommandList->SetComputeRootDescriptorTable( m_pnComputeUavRootParameterIndices[paramIndex], m_pd3dComputeUavRootParameterGpuDescriptorHandles[textureIndex]);
	}
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

	int type = GetTextureType(index);
	switch (type)
	{
	case RESOURCE_TEXTURE2D:
	case RESOURCE_TEXTURE2D_ARRAY:
		srv.Format = desc.Format;
		srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv.Texture2D.MipLevels = -1;
		srv.Texture2D.MostDetailedMip = 0;
		srv.Texture2D.PlaneSlice = 0;
		srv.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY:
		srv.Format = desc.Format;
		srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srv.Texture2DArray.MipLevels = -1;
		srv.Texture2DArray.MostDetailedMip = 0;
		srv.Texture2DArray.PlaneSlice = 0;
		srv.Texture2DArray.ResourceMinLODClamp = 0.0f;
		srv.Texture2DArray.FirstArraySlice = 0;
		srv.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE:
		srv.Format = desc.Format;
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

CMaterial::CMaterial(int nTextures)
{
	m_nTextures = nTextures;

	m_ppTextures = new CTexture*[m_nTextures];
	m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
	for (int i = 0; i < m_nTextures; i++) m_ppTextures[i] = NULL;
	for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';
}

CMaterial::~CMaterial()
{
	if (m_nTextures > 0)
	{
		for (int i = 0; i < m_nTextures; i++) 
			if (m_ppTextures[i])
				m_ppTextures[i]->Release();

		delete[] m_ppTextures;
		m_ppTextures = nullptr;

		if (m_ppstrTextureNames)
		{
			delete[] m_ppstrTextureNames;
			m_ppstrTextureNames = nullptr;
		}
	}


	DebugOutput("\nDelete Material");

}

CMaterial::CMaterial(const CMaterial& other)
{
	m_cAlbedo = other.m_cAlbedo;
	m_cEmissive = other.m_cEmissive;

	m_fRoughness = other.m_fRoughness;
	m_fMetallic = other.m_fMetallic;
	m_fSpecular = other.m_fSpecular;

	m_xmf4SpecularColor = other.m_xmf4SpecularColor;
	m_fGlossiness = other.m_fGlossiness;
	m_fGlossyReflection = other.m_fGlossyReflection;


	m_pShader = other.m_pShader;
	m_nType = other.m_nType;
	m_nTextures = other.m_nTextures;

	if (other.m_ppstrTextureNames != nullptr) 
	{
		m_ppstrTextureNames = new _TCHAR[other.m_nTextures][64]; 
		for (int i = 0; i < other.m_nTextures; ++i) 
		{
			std::memcpy(m_ppstrTextureNames[i], other.m_ppstrTextureNames[i], sizeof(_TCHAR) * 64);
		}
	}
	else {
		m_ppstrTextureNames = nullptr;
	}

	if (other.m_ppTextures != nullptr)
	{
		m_ppTextures = new CTexture * [other.m_nTextures];  
		for (int i = 0; i < other.m_nTextures; ++i) 
		{
			if (other.m_ppTextures[i])
			{
				m_ppTextures[i] = new CTexture(*other.m_ppTextures[i]); 
			}
			else
			{
				m_ppTextures[i] = nullptr;
			}
		}
	}
	else 
	{
		m_ppTextures = nullptr;
	}
}

void CMaterial::SetShader(CShader* pShader)
{
	if (pShader)
	{
		if (pShader != m_pStandardShader && pShader != m_pSkinnedAnimationShader)
			if(m_pShader != NULL)
				m_pShader->Release();

		m_pShader = pShader;

		if (pShader != m_pStandardShader && pShader != m_pSkinnedAnimationShader)
			if (m_pShader != NULL)
				m_pShader->AddRef();
	}

}

void CMaterial::SetTexture(CTexture *pTexture, UINT nTexture) 
{ 
	if (m_ppTextures[nTexture]) m_ppTextures[nTexture]->Release();
	m_ppTextures[nTexture] = pTexture; 
	if (m_ppTextures[nTexture]) m_ppTextures[nTexture]->AddRef();  
}

void CMaterial::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->ReleaseUploadBuffers();
	}
}

CShader *CMaterial::m_pSkinnedAnimationShader = NULL;
CShader *CMaterial::m_pStandardShader = NULL;

void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	//if (m_pStandardShader)
	//{
		m_pStandardShader = new Deferred_CStandard_Shader();
		m_pStandardShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, RenderTarget_Config::RTV_FORMAT_num, RenderTarget_Config::RTV_FORMATS, RenderTarget_Config::DSV_FORMAT);
		m_pStandardShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//}

	//if (m_pSkinnedAnimationShader)
	//{
		m_pSkinnedAnimationShader = new Deferred_CSkinnedAnimationStandardShader();
		m_pSkinnedAnimationShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, RenderTarget_Config::RTV_FORMAT_num, RenderTarget_Config::RTV_FORMATS, RenderTarget_Config::DSV_FORMAT);
		m_pSkinnedAnimationShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//}

	//if (Object_Manager::instance_shader)
	//{
		Object_Manager::instance_shader = std::make_shared<Deferred_CStandard_Instance_Shader>();
		Object_Manager::instance_shader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, RenderTarget_Config::RTV_FORMAT_num, RenderTarget_Config::RTV_FORMATS, RenderTarget_Config::DSV_FORMAT);
		Object_Manager::instance_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//}
}

void CMaterial::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList)
{


	Material_Info material_info;
	material_info.gAlbedoColor = m_cAlbedo;

	material_info.gRoughness = m_fRoughness;
	material_info.gMetallic = m_fMetallic;
	material_info.gEmissive_intensity = m_cEmissive.w;
	material_info.gSpecular_intensity = m_fSpecular;

//	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 12, &material_info, 16);
//	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 1, &m_nType, 28);

	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 8, &material_info, 16); // 16~23
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 1, &m_nType, 27);       // 27

	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i])
			m_ppTextures[i]->UpdateGraphicsSrvShaderVariables(pd3dCommandList);
	}
}

void CMaterial::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader)
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
//			*ppTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
			*ppTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);

			(*ppTexture)->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pwstrTextureName, RESOURCE_TEXTURE2D, 0);
			if (*ppTexture) (*ppTexture)->AddRef();

//			CScene::CreateShaderResourceViews(pd3dDevice, *ppTexture, 0, nRootParameter);
			CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, *ppTexture, 0, nRootParameter);

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
				CGameObject* pRootGameObject = pParent;
				*ppTexture = pRootGameObject->FindReplicatedTexture(pwstrTextureName);
				if (*ppTexture) (*ppTexture)->AddRef();
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void CRootMotionCallbackHandler::HandleCallback(void* pCallbackData, float fTrackPosition)
{
	float* pfData = (float*)pCallbackData;
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Data: %.2f, Position: %.2f\n"), *pfData, fTrackPosition);
	OutputDebugString(pstrDebug);
}


CAnimationSet::CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrames, int nAnimatedBones, char *pstrName)
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
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4*[nKeyFrames];
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
		if ((m_pfKeyFrameTranslationTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameTranslationTimes[i+1]))
		{
			float t = (fPosition - m_pfKeyFrameTranslationTimes[i]) / (m_pfKeyFrameTranslationTimes[i+1] - m_pfKeyFrameTranslationTimes[i]);
			T = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i+1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameScales - 1); i++)
	{
		if ((m_pfKeyFrameScaleTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameScaleTimes[i+1]))
		{
			float t = (fPosition - m_pfKeyFrameScaleTimes[i]) / (m_pfKeyFrameScaleTimes[i+1] - m_pfKeyFrameScaleTimes[i]);
			S = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameScales[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameScales[i+1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameRotations - 1); i++)
	{
		if ((m_pfKeyFrameRotationTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameRotationTimes[i+1]))
		{
			float t = (m_fPosition - m_pfKeyFrameRotationTimes[i]) / (m_pfKeyFrameRotationTimes[i+1] - m_pfKeyFrameRotationTimes[i]);
			R = XMQuaternionSlerp(XMQuaternionConjugate(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i][nBone])), XMQuaternionConjugate(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i+1][nBone])), t);
			break;
		}
	}

	XMStoreFloat4x4(&xmf4x4Transform, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
#else   
	for (int i = 0; i < (m_nKeyFrames - 1); i++) 
	{
		if ((m_pfKeyFrameTimes[i] <= fPosition) && (fPosition < m_pfKeyFrameTimes[i+1]))
		{
			float t = (fPosition - m_pfKeyFrameTimes[i]) / (m_pfKeyFrameTimes[i+1] - m_pfKeyFrameTimes[i]);
			xmf4x4Transform = Matrix4x4::Interpolate(m_ppxmf4x4KeyFrameTransforms[i][nBone], m_ppxmf4x4KeyFrameTransforms[i+1][nBone], t);
			break;
		}
	}
	if (fPosition >= m_pfKeyFrameTimes[m_nKeyFrames-1]) xmf4x4Transform = m_ppxmf4x4KeyFrameTransforms[m_nKeyFrames-1][nBone];

#endif
	return(xmf4x4Transform);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationSets::CAnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	m_pAnimationSet_list = new CAnimationSet*[nAnimationSets];
}

CAnimationSets::~CAnimationSets()
{
	for (int i = 0; i < m_nAnimationSets; i++) 
		if (m_pAnimationSet_list[i]) 
			delete m_pAnimationSet_list[i];

	if (m_pAnimationSet_list)
		delete[] m_pAnimationSet_list;

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

void CAnimationTrack::SetAnimationCallbackHandler(CAnimationCallbackHandler * pCallbackHandler)
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
CAnimationController::CAnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nAnimationTracks, CLoadedModelInfo *pModel)
{
	m_nAnimationTracks = nAnimationTracks;
    m_pAnimationTracks = new CAnimationTrack[nAnimationTracks];

	m_pAnimationSets = pModel->m_pAnimationSets;
	m_pAnimationSets->AddRef();

	m_pModelRootObject = pModel->m_pModelRootObject;

	m_nSkinnedMeshes = pModel->m_nSkinnedMeshes;
	m_ppSkinnedMeshes = new CSkinnedMesh*[m_nSkinnedMeshes];

	for (int i = 0; i < m_nSkinnedMeshes; i++) 
		m_ppSkinnedMeshes[i] = pModel->m_ppSkinnedMeshes[i];

	m_ppd3dcbSkinningBoneTransforms = new ID3D12Resource*[m_nSkinnedMeshes]();
	m_ppcbxmf4x4MappedSkinningBoneTransforms = new XMFLOAT4X4*[m_nSkinnedMeshes]();

	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); 
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Map(0, NULL, (void **)&m_ppcbxmf4x4MappedSkinningBoneTransforms[i]);
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

	if (m_ppSkinnedMeshes) 
		delete[] m_ppSkinnedMeshes;
}

void CAnimationController::SetCallbackKeys(int nAnimationTrack, int nCallbackKeys)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetCallbackKeys(nCallbackKeys);
}

void CAnimationController::SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fKeyTime, void* pData)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetCallbackKey(nKeyIndex, fKeyTime, pData);
}

void CAnimationController::SetAnimationCallbackHandler(int nAnimationTrack, CAnimationCallbackHandler *pCallbackHandler)
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

void CAnimationController::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
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
		// Bone 정보 초기화
		// m_ppBoneFrameCaches가 각각 pRootGameObject의 자식 객체(=bone 객체)
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
		{
			m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = Matrix4x4::Zero();
		}

		// 활성화된 트랙의 전체 가중치 크기 -> 가중치 정규화에 사용
		float totalWeight = 0.0f;
		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable)
			{
				totalWeight += m_pAnimationTracks[k].m_fWeight;
			}
		}


		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable && totalWeight > 0.0f)
			{
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet];
				float fPosition = m_pAnimationTracks[k].UpdatePosition(m_pAnimationTracks[k].m_fPosition, fTimeElapsed, pAnimationSet->m_fLength);

				// 각 bone 마다 변환행렬 업데이트
				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent;
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);


					float normalizedWeight = m_pAnimationTracks[k].m_fWeight / totalWeight; // 트랙의 가중치 정규화
					XMFLOAT4X4 blendedTransform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, normalizedWeight)); // 정규화 비율을 적용한 트랙의 변환행렬 더하기

					const std::string& boneName = m_pAnimationSets->GetBoneName(j);
					if (pRootGameObject->Object_type == OBJECT_TPYE_MAIN_PLAYER || pRootGameObject->Object_type == OBJECT_TPYE_PLAYER) {
						if (boneName == "Hips")
						{
							if (k == TRACK_DIVEROLL_FORWARD && !m_pAnimationTracks[k].m_bFinished) {
								HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);
							}
							//if (dynamic_cast<CTerrainPlayer*>(pRootGameObject)->GetStateMachine()->Get_State() == State::Idle) {
							//	HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);
							//}
							blendedTransform._41 = 0.0f;
							//blendedTransform._42 = 0.8762761f;
							blendedTransform._43 = 0.0f;

						}
					}
					else if (pRootGameObject->Object_type == OBJECT_TPYE_MONSTER) {
						if ((boneName == "Gargoyle_LP" || boneName == "Anubis_lp"/* || boneName == "Hips"*/) && k == 3) {
							HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);
							/*std::wostringstream oss;
							oss << L"XMFLOAT3: (" << HipsPosition.x << L", " << HipsPosition.y << L", " << HipsPosition.z << L")\n";
							OutputDebugStringW(oss.str().c_str());*/

							blendedTransform._41 = 0.0f;
							//blendedTransform._42 = 0.0f;
							blendedTransform._43 = 0.0f;
						}

					}

					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = blendedTransform;
				}

				m_pAnimationTracks[k].HandleCallback();
			}
		}

		// 4. Transform 업데이트
		// m_ppBoneFrameCaches가 각각 pRootGameObject의 자식 객체(=bone 객체)이므로, 
		// 자식 객체에 부모 객체의 변환 정보를 상속하면서 뼈 객체마다 업데이트 된 m_xmf4x4Parent가 반영되어, 각 뼈의 정보 반영
		// == 각 뼈들의 변환 행렬 정보 업데이트
		pRootGameObject->UpdateTransform(NULL);


		// 5. 추가 애니메이션 처리
		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}

void CAnimationController::ApplyCurrentAnimationPose(CGameObject* pRootGameObject)
{
	if (!m_pAnimationTracks || !m_pAnimationSets) return;

	// 본 초기화
	for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
	{
		m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = Matrix4x4::Zero();
	}

	float totalWeight = 0.0f;
	for (int k = 0; k < m_nAnimationTracks; k++)
	{
		if (m_pAnimationTracks[k].m_bEnable)
		{
			totalWeight += m_pAnimationTracks[k].m_fWeight;
		}
	}

	for (int k = 0; k < m_nAnimationTracks; k++)
	{
		if (m_pAnimationTracks[k].m_bEnable && totalWeight > 0.0f)
		{
			CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet];
			float fPosition = m_pAnimationTracks[k].m_fPosition; 

			for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
			{
				XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent;
				XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);

				float normalizedWeight = m_pAnimationTracks[k].m_fWeight / totalWeight;
				XMFLOAT4X4 blendedTransform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, normalizedWeight));

				const std::string& boneName = m_pAnimationSets->GetBoneName(j);
				if (pRootGameObject->Object_type == OBJECT_TPYE_MAIN_PLAYER || pRootGameObject->Object_type == OBJECT_TPYE_PLAYER) {
					if (boneName == "Hips")
					{
						if (k == TRACK_DIVEROLL_FORWARD && !m_pAnimationTracks[k].m_bFinished) {
							HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);
						}
						//if (dynamic_cast<CTerrainPlayer*>(pRootGameObject)->GetStateMachine()->Get_State() == State::Idle) {
						//	HipsPosition = XMFLOAT3(blendedTransform._41, blendedTransform._42, blendedTransform._43);
						//}
						blendedTransform._41 = 0.0f;
						//blendedTransform._42 = 0.8762761f;
						blendedTransform._43 = 0.0f;

					}
				}

				m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4Parent = blendedTransform;
			}
			m_pAnimationTracks[k].HandleCallback();
		}
	}

	// 트랜스폼 적용
	pRootGameObject->UpdateTransform(nullptr);
}

void CAnimationController::ServerAdvanceTime(const ServerAnimationSyncData& syncData)
{

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
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSet_list[m_pAnimationTracks[k].m_nAnimationSet];
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
	if (m_ppSkinnedMeshes) 
		delete[] m_ppSkinnedMeshes;
}

void CLoadedModelInfo::PrepareSkinning()
{
	int nSkinnedMesh = 0;
	m_ppSkinnedMeshes = new CSkinnedMesh*[m_nSkinnedMeshes];
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nSkinnedMesh);

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
}

CGameObject::CGameObject(int nMaterials, const std::string_view& name) : CGameObject(name)
{
	Material_list.resize(nMaterials);
	for (std::shared_ptr<CMaterial> material_ptr: Material_list)
	{
		material_ptr.reset();
	}

}

CGameObject::~CGameObject()
{
	if (m_pMesh) 
		m_pMesh->Release();

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
		m_pMesh = new CMesh(*other.m_pMesh);  
	

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
		if (m_pMesh != nullptr) 
			delete m_pMesh;  
		m_pMesh = new CMesh(*other.m_pMesh);  
	}

	if (other.m_pSkinnedAnimationController != nullptr)
	{
		if (m_pSkinnedAnimationController != nullptr) 
			m_pSkinnedAnimationController.reset();  
		m_pSkinnedAnimationController = std::make_shared <CAnimationController>(*other.m_pSkinnedAnimationController);
	}

	return *this;
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
	if (pChild) 
		pChild->m_pParent = this;
	

	if (m_pChild) 
	{
		if (pChild) 
			pChild->m_pSibling = m_pChild->m_pSibling; 
		
		m_pChild->m_pSibling = pChild;
	}
	else 
		m_pChild = pChild;

}

void CGameObject::Set_Active(bool active, bool IsRoot)
{
	Active = active;

	if (m_pChild != NULL)
		m_pChild->Set_Active(active, false); 

	if (!IsRoot && m_pSibling != NULL)
		m_pSibling->Set_Active(active, false);
}

void CGameObject::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetShader(CShader *pShader)
{

	std::shared_ptr<CMaterial> material_ptr = std::make_shared<CMaterial>(0);
	material_ptr->SetShader(pShader);
	Material_list.push_back(material_ptr);

}

void CGameObject::SetShader(int nMaterial, CShader *pShader)
{


	if (Material_list.size() > nMaterial)
		Material_list[nMaterial]->SetShader(pShader);

}

void CGameObject::SetMaterial(int nMaterial, CMaterial *pMaterial)
{
	std::shared_ptr<CMaterial> material_ptr(pMaterial);
	Material_list[nMaterial] = material_ptr;
}

void CGameObject::FindAndSetSkinnedMesh(CSkinnedMesh **ppSkinnedMeshes, int *pnSkinnedMesh)
{
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) 
		ppSkinnedMeshes[(*pnSkinnedMesh)++] = (CSkinnedMesh *)m_pMesh;

	if (m_pSibling) 
		m_pSibling->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);

	if (m_pChild) 
		m_pChild->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
}

CGameObject* CGameObject::FindFrame(char* pstrFrameName)
{
	if (m_pstrFrameName && strcmp(m_pstrFrameName, pstrFrameName) == 0)
		return this;

	CGameObject* pFrameObject = nullptr;

	if (m_pSibling) 
	{
		pFrameObject = m_pSibling->FindFrame(pstrFrameName);
		if (pFrameObject)
			return pFrameObject;
	}

	if (m_pChild)
	{
		pFrameObject = m_pChild->FindFrame(pstrFrameName);
		if (pFrameObject)
			return pFrameObject;
	}

	return nullptr;
}

void CGameObject::UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent)
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

	if (m_pSkinnedAnimationController) 
		m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);


	if (m_pSibling) 
		m_pSibling->Animate(fTimeElapsed);

	if (m_pChild) 
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
	//CGameObject* root_obj_ptr = Get_Root_Object();
	//XMFLOAT3 world_pos = root_obj_ptr->GetPosition();

	//root_obj_ptr->previous_position.x = world_pos.x;
	//root_obj_ptr->previous_position.y = world_pos.y;
	//root_obj_ptr->previous_position.z = world_pos.z;
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

	if (m_pMesh)
	{
		if (!IsVisible(pCamera))
			return;

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
						int pipelineStateNum = pShader->Get_Num_PipelineState();
						for (int j = 0; j < pipelineStateNum; ++j)
						{
							pShader->Setting_Render(pd3dCommandList, j);


							// 재료(Material) 셰이더 변수 업데이트
							material_ptr->UpdateShaderVariable(pd3dCommandList);


							m_pMesh->Render(pd3dCommandList, i);
						}
					}
					else
					{

						// 셰이더가 없는 경우에도 재료 업데이트 후 메쉬 렌더링
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

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 16, &xmf4x4World, 0);   // 0~15

	//===============================================================================
	XMFLOAT3 now_position = GetPosition();
	XMFLOAT3 velocity = 	{ 
		now_position.x - previous_position.x,
		now_position.y - previous_position.y,
		now_position.z - previous_position.z
	};
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX, 3, &velocity, 24);      // 24~26

}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial)
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
		if(material_ptr!= NULL)
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
	XMFLOAT3 originalPosition;

	if (keepPosition) 
	{
		originalPosition = { m_xmf4x4Parent._41, m_xmf4x4Parent._42, m_xmf4x4Parent._43 };
		m_xmf4x4Parent._41 = m_xmf4x4Parent._42 = m_xmf4x4Parent._43 = 0.0f;
	}

	XMMATRIX scaleMatrix = XMMatrixScaling(x, y, z);
	XMMATRIX worldMatrix = XMLoadFloat4x4(&m_xmf4x4Parent);

	worldMatrix = worldMatrix * scaleMatrix;

	XMStoreFloat4x4(&m_xmf4x4Parent, worldMatrix);

	if (keepPosition) 
	{
		m_xmf4x4Parent._41 = originalPosition.x;
		m_xmf4x4Parent._42 = originalPosition.y;
		m_xmf4x4Parent._43 = originalPosition.z;
	}

	UpdateTransform(NULL);
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

CGameObject* CGameObject::Get_Root_Object()
{
	CGameObject* root = this;
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

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4Parent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Parent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4Parent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Parent);

	UpdateTransform(NULL);
}

void CGameObject::SetLookDirection(const XMFLOAT3& look)
{
	XMVECTOR vLook = XMVector3Normalize(XMLoadFloat3(&look));
	XMVECTOR vUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));
	vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

	// 기존 스케일, 위치 정보 추출
	XMFLOAT3 scale = GetScale(m_xmf4x4Parent); // 따로 함수가 없으면 m_xmf4x4Parent에서 직접 계산
	XMFLOAT3 position = GetPosition();

	// 스케일 반영된 회전 벡터
	XMVECTOR vScaledRight = XMVectorScale(vRight, scale.x);
	XMVECTOR vScaledUp = XMVectorScale(vUp, scale.y);
	XMVECTOR vScaledLook = XMVectorScale(vLook, scale.z);

	// 회전만 적용한 새 행렬 만들기
	XMFLOAT4X4 xmf4x4New;
	xmf4x4New._11 = XMVectorGetX(vScaledRight); xmf4x4New._12 = XMVectorGetY(vScaledRight); xmf4x4New._13 = XMVectorGetZ(vScaledRight); xmf4x4New._14 = 0.0f;
	xmf4x4New._21 = XMVectorGetX(vScaledUp);    xmf4x4New._22 = XMVectorGetY(vScaledUp);    xmf4x4New._23 = XMVectorGetZ(vScaledUp);    xmf4x4New._24 = 0.0f;
	xmf4x4New._31 = XMVectorGetX(vScaledLook);  xmf4x4New._32 = XMVectorGetY(vScaledLook);  xmf4x4New._33 = XMVectorGetZ(vScaledLook);  xmf4x4New._34 = 0.0f;
	xmf4x4New._41 = position.x;                 xmf4x4New._42 = position.y;                 xmf4x4New._43 = position.z;                 xmf4x4New._44 = 1.0f;

	m_xmf4x4Parent = xmf4x4New;
	UpdateTransform(NULL);
}

CTexture *CGameObject::FindReplicatedTexture(_TCHAR *pstrTextureName)
{
	for (std::shared_ptr<CMaterial> material_ptr : Material_list)
	{
		for (int j = 0; j < material_ptr->m_nTextures; j++)
		{
			if (material_ptr->m_ppTextures[j])
			{
				if (!_tcsncmp(material_ptr->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName)))
					return(material_ptr->m_ppTextures[j]);
			}
		}
	}

	CTexture *pTexture = NULL;
	if (m_pSibling) if (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName)) return(pTexture);
	if (m_pChild) if (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName)) return(pTexture);

	return(NULL);
}

int ReadIntegerFromFile(FILE *pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile); 
	return(nValue);
}

float ReadFloatFromFile(FILE *pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile); 
	return(fValue);
}

BYTE ReadStringFromFile(FILE *pInFile, char *pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile); 
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

void CGameObject::LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader)
{
	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	// delete old Material & Resize material
	Material_list.clear();

	int materialCount = ReadIntegerFromFile(pInFile);
	Material_list.reserve(materialCount);  // Reserve memory

	std::shared_ptr<CMaterial> pMaterial = nullptr;

	for (;;)
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ReadIntegerFromFile(pInFile);

			// add & define new Material 
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

			// Resize to nMaterial
			if (nMaterial >= Material_list.size())
				Material_list.resize(nMaterial + 1);

			Material_list[nMaterial] = pMaterial;
		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_cAlbedo), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_cEmissive), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSpecular), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fRoughness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Glossiness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossiness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 
				MATERIAL_ALBEDO_MAP, ROOT_PARAMETER_ALBEDO_TEXTURE_SRV_INDEX, 
				pMaterial->m_ppstrTextureNames[0], &(pMaterial->m_ppTextures[0]),
				pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<SpecularMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 
				MATERIAL_SPECULAR_MAP, ROOT_PARAMETER_SPECULAR_TEXTURE_SRV_INDEX,
				pMaterial->m_ppstrTextureNames[1], &(pMaterial->m_ppTextures[1]),
				pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 
				MATERIAL_NORMAL_MAP, ROOT_PARAMETER_NORMAL_TEXTURE_SRV_INDEX,
				pMaterial->m_ppstrTextureNames[2], &(pMaterial->m_ppTextures[2]),
				pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<MetallicMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 
				MATERIAL_METALLIC_MAP, ROOT_PARAMETER_METALLIC_TEXTURE_SRV_INDEX,
				pMaterial->m_ppstrTextureNames[3], &(pMaterial->m_ppTextures[3]),
				pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<EmissionMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 
				MATERIAL_EMISSION_MAP, ROOT_PARAMETER_EMISSION_TEXTURE_SRV_INDEX,
				pMaterial->m_ppstrTextureNames[4], &(pMaterial->m_ppTextures[4]),
				pParent, pInFile, pShader);
		}
		//else if (!strcmp(pstrToken, "<DetailAlbedoMap>:"))
		//{
		//	pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_ALBEDO_MAP, ROOT_PARAMETER_DETAIL_ALBEDO_TEXTURE_SRV_INDEX,
		//		pMaterial->m_ppstrTextureNames[5], &(pMaterial->m_ppTextures[5]),
		//		pParent, pInFile, pShader);
		//}
		//else if (!strcmp(pstrToken, "<DetailNormalMap>:"))
		//{
		//	pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_NORMAL_MAP, ROOT_PARAMETER_DETAIL_NORMAL_TEXTURE_SRV_INDEX,
		//		pMaterial->m_ppstrTextureNames[6], &(pMaterial->m_ppTextures[6]),
		//		pParent, pInFile, pShader);
		//}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			break;
		}
	}

}


CGameObject *CGameObject::LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CGameObject *pParent, FILE *pInFile, CShader *pShader, int *pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	CGameObject *pGameObject = new CGameObject();

	for ( ; ; )
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

			CStandardMesh *pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			CSkinnedMesh *pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
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
					CGameObject *pChild_raw_ptr = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader, pnSkinnedMeshes);
					
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

CGameObject* CGameObject::Load_Scene_HierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	CGameObject* pGameObject = new CGameObject();

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
			CStandardMesh* pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken);
			if (!strcmp(pstrToken, "<Mesh_Name>:"))
			{
				::ReadStringFromFile(pInFile, pstrToken);
				std::string fileName = "Scene/Scene_File/Meshes/bin/" + std::string(pstrToken); 

				char pstrFileName[128] = { '\0' };
				strncpy(pstrFileName, fileName.c_str(), sizeof(pstrFileName) - 1);
				pstrFileName[sizeof(pstrFileName) - 1] = '\0';

				pMesh->LoadMeshFrom_OtherFile(pd3dDevice, pd3dCommandList, pstrFileName);
			}
			pGameObject->SetMesh(pMesh);

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
					CGameObject* pChild_raw_ptr = CGameObject::Load_Scene_HierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader);

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

CLoadedModelInfo* CGameObject::Load_Scene_File(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader)
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
				CGameObject* ModelRootObject_raw_ptr = CGameObject::Load_Scene_HierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader);

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


void CGameObject::PrintFrameInfo(CGameObject* pGameObject, CGameObject *pParent)
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

void CGameObject::LoadAnimationFromFile(FILE *pInFile, CLoadedModelInfo *pLoadedModel)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nAnimationSets = 0;

	for ( ; ; )
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
				CGameObject* frame_ptr = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);
				pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches[j] = frame_ptr;

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

			pLoadedModel->m_pAnimationSets->m_pAnimationSet_list[nAnimationSet] = new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames, pstrToken);

			for (int i = 0; i < nKeyFrames; i++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				if (!strcmp(pstrToken, "<Transforms>:"))
				{
					CAnimationSet *pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSet_list[nAnimationSet];

					int nKey = ::ReadIntegerFromFile(pInFile); //i
					float fKeyTime = ::ReadFloatFromFile(pInFile);

#ifdef _WITH_ANIMATION_SRT
					m_pfKeyFrameScaleTimes[i] = fKeyTime;
					m_pfKeyFrameRotationTimes[i] = fKeyTime;
					m_pfKeyFrameTranslationTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameScales[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4KeyFrameRotations[i], sizeof(XMFLOAT4), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameTranslations[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
#else
					pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}

CLoadedModelInfo *CGameObject::LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char *pstrFileName, CShader *pShader)
{
	FILE *pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb"); 
	::rewind(pInFile);

	CLoadedModelInfo *pLoadedModel = new CLoadedModelInfo();

	char pstrToken[64] = { '\0' };

	for ( ; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				CGameObject* ModelRootObject_raw_ptr = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, &pLoadedModel->m_nSkinnedMeshes);
			
				std::shared_ptr<CGameObject> ModelRootObject_shared_ptr(ModelRootObject_raw_ptr);
				pLoadedModel->m_pModelRootObject = ModelRootObject_shared_ptr;

				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
			else if (!strcmp(pstrToken, "<Animation>:"))
			{
				CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
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
		if(m_pMesh->Vertex_Existence())
			return m_pMesh->Get_Name();

	return string("None");


}

BoundingOrientedBox* CGameObject::Get_Collider()
{
	if (m_pMesh == NULL)
		return NULL;
	BoundingOrientedBox* pOriginalBoundingBox = m_pMesh->Get_BoundingBox();
	if (pOriginalBoundingBox == NULL)
		return NULL;



	BoundingOrientedBox pWorldBoundingBox(*pOriginalBoundingBox);
//	pWorldBoundingBox.Center = GetPosition();

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
		m_pMesh = new OBBContainer();
	m_pMesh->Set_BoundingBox(ptr); 
}

ServerAnimationSyncData CGameObject::MakeSyncData()
{
	ServerAnimationSyncData data;
	data.position = GetPosition();
	data.lookVector = GetLook();
	data.currentState = State::Idle;

	return data;
}

void CGameObject::ApplySyncData(const ServerAnimationSyncData& syncData)
{
	
	SetLookDirection(syncData.lookVector);
	SetPosition(syncData.position);
}


CTexture* CHeightMapTerrain::pTerrainBaseTexture = nullptr;
CTexture* CHeightMapTerrain::pTerrainDetailTexture = nullptr;
Deferred_CTerrainShader* CHeightMapTerrain::pTerrainShader = nullptr;
CMaterial* CHeightMapTerrain::pTerrainMaterial = nullptr;
CHeightMapImage* CHeightMapTerrain::m_pHeightMapImage = nullptr;

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, 
	int start_x_pos, int start_z_pos, int nWidth, int nLength,  XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, int Vertex_gap, int nMaxDepth) : CGameObject(1)
{
	static int tile_map_number = 0;


	if (pTerrainBaseTexture == nullptr)
	{

		CreateShaderVariables(pd3dDevice, pd3dCommandList);

		pTerrainBaseTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
		pTerrainBaseTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Base_Texture.dds", RESOURCE_TEXTURE2D, 0);

		pTerrainDetailTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
		pTerrainDetailTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Detail_Texture_7.dds", RESOURCE_TEXTURE2D, 0);


		pTerrainShader = new Deferred_CTerrainShader();
		pTerrainShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, RenderTarget_Config::RTV_FORMAT_num, RenderTarget_Config::RTV_FORMATS, RenderTarget_Config::DSV_FORMAT);
		pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);




		CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, pTerrainBaseTexture, 0, ROOT_PARAMETER_TERRAIN_BASE_TEXTURE_SRV_INDEX);
		CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, pTerrainDetailTexture, 0, ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX);



		pTerrainMaterial = new CMaterial(2);
		pTerrainMaterial->SetTexture(pTerrainBaseTexture, 0);
		pTerrainMaterial->SetTexture(pTerrainDetailTexture, 1);
		pTerrainMaterial->SetShader(pTerrainShader);

		m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);


		tile_map_number = 0;
		Set_Name("Root_Tile_Map");
	}
	else
	{
		Set_Tile(tile_map_number++);
	}

	// ============================================================

	Vertex_gap = (Vertex_gap % 2) ? Vertex_gap + 1 : Vertex_gap;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;
	m_nDepth = nMaxDepth;

	Area_LT.x = start_x_pos * xmf3Scale.x;
	Area_LT.y = start_z_pos * xmf3Scale.z;

	Area_RB.x = (start_x_pos + m_nWidth) * xmf3Scale.x;
	Area_RB.y = (start_z_pos + m_nLength) * xmf3Scale.z;

	Tile_Start_Pos = { (float)start_x_pos , (float)start_z_pos };

	if (nMaxDepth == 0)
	{
		CHeightMapGridMesh* part_mesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, start_x_pos, start_z_pos, nWidth +1, nLength +1, xmf3Scale, xmf4Color, Vertex_gap, m_pHeightMapImage);
		SetMesh(part_mesh);
	}
	else
	{
		int Cell_num = 2;
		long blocks_x_size[2];
		long blocks_z_size[2];

		blocks_x_size[0] = m_nWidth / 2;
		blocks_x_size[1] = m_nWidth - blocks_x_size[0]; 


		blocks_z_size[0] = m_nLength / 2;
		blocks_z_size[1] = m_nLength - blocks_z_size[0]; 


		if (nMaxDepth > 0)
		{
			for (int z = 0; z < Cell_num; ++z)
			{
				for (int x = 0; x < Cell_num; ++x)
				{

					XMFLOAT4 tile_color = Get_Random_Color(1.0f);

					int xStart = start_x_pos + x * blocks_x_size[0];
					int zStart = start_z_pos + z * blocks_z_size[0];

					CHeightMapTerrain* part_map_raw_ptr = 
						new CHeightMapTerrain(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pFileName, xStart, zStart, blocks_x_size[x], blocks_z_size[z], xmf3Scale, tile_color, Vertex_gap, nMaxDepth - 1);

					std::shared_ptr<CGameObject> part_map(part_map_raw_ptr);
					Set_Child(part_map);

				}
			}
		}
	}

	//if (!strncmp(m_pstrFrameName, "Root_Tile_Map", strlen("Root_Tile_Map")))
	//	PrintFrameInfo(this, NULL);
	
}


CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage != NULL) 
		delete m_pHeightMapImage;

	m_pHeightMapImage = NULL;
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

		x -= m_xmf4x4World._41;
		z -= m_xmf4x4World._43;


	if (x >= Area_LT.x && x < Area_RB.x && z >= Area_LT.y && z < Area_RB.y)
	{
		if (Get_Child())
		{
			CGameObject* child_ptr = Get_Child().get();
			return ((CHeightMapTerrain*)child_ptr)->Get_Mesh_Height(x, z, bReverseQuad, last_tile_ptr);
		}
		else
		{
			last_tile_ptr = this;
			return m_pMesh->Get_Height(x, z);
		}
	}
	else
	{
		if (Get_Sibling())
		{
			CGameObject* sibling_ptr = Get_Sibling().get();
			return ((CHeightMapTerrain*)sibling_ptr)->Get_Mesh_Height(x, z, bReverseQuad, last_tile_ptr);
		}
	}

	return -1;
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

	if (x >= Area_LT.x && x < Area_RB.x&& z >= Area_LT.y&& z < Area_RB.y)
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
	if (Get_Active() && m_pMesh != NULL)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (pTerrainMaterial && pTerrainMaterial->m_pShader)
		{
			pTerrainMaterial->UpdateShaderVariable(pd3dCommandList);

			pTerrainMaterial->m_pShader->Setting_Render(pd3dCommandList, 0); // 첫 번째 PSO
			m_pMesh->Render(pd3dCommandList, 0);

			pTerrainMaterial->m_pShader->Setting_Render(pd3dCommandList, 1); // 두 번째 PSO
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject(1)
{
	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

//	CTexture* pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	CTexture* pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1, 0, 0, 1, 0, 0);

	pSkyBoxTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"SkyBox/SkyBox_0.dds", RESOURCE_TEXTURE_CUBE, 0);

	CSkyBoxShader *pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//CScene::CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, 0, ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, pSkyBoxTexture, 0, ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX);
	
	CMaterial *pSkyBoxMaterial = new CMaterial(1);
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);

	SetMaterial(0, pSkyBoxMaterial);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	CGameObject::Render(pd3dCommandList, pCamera);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAngrybotAnimationController::CAngrybotAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel) : CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pModel)
{
}

CAngrybotAnimationController::~CAngrybotAnimationController()
{
}

void CAngrybotAnimationController::OnRootMotion(CGameObject* pRootGameObject)
{

}

CAngrybotObject::CAngrybotObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks)
{
	CLoadedModelInfo *pAngrybotModel = pModel;
	if (!pAngrybotModel) 
		pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Player.bin", NULL);

	Set_Child(pAngrybotModel->m_pModelRootObject);
	m_pSkinnedAnimationController = std::make_shared <CAngrybotAnimationController>(pd3dDevice, pd3dCommandList, nAnimationTracks, pAngrybotModel);
}

CAngrybotObject::~CAngrybotObject()
{
}

CHumanObject::CHumanObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pHumanModel = pModel;

	if (!pHumanModel) 
		pHumanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Human.bin", NULL);

	Set_Child(pHumanModel->m_pModelRootObject);
	m_pSkinnedAnimationController = std::make_shared<CAnimationController>(pd3dDevice, pd3dCommandList, nAnimationTracks, pHumanModel);
}

CHumanObject::~CHumanObject()
{
}


CMonsterObject::CMonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
	: m_StateMachine(std::make_unique<MonsterStateMachine>(this))
{
	Object_type = OBJECT_TPYE_MONSTER;
	CLoadedModelInfo* pHumanModel = pModel;
	//pHumanModel->m_pAnimationSets = pHumanModel->m_pAnimationSets->Clone();

	if (!pHumanModel) {
		pHumanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Gargoyle_LP.bin", NULL);
		//pHumanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Anubis_lp.bin", NULL);
	}
	n_Animation = nAnimationTracks;
	prevWeights.resize(n_Animation, 0.0f);
	targetWeights.resize(n_Animation, 0.0f);

	Set_Child(pHumanModel->m_pModelRootObject);
	m_pSkinnedAnimationController = std::make_shared<CAnimationController>(pd3dDevice, pd3dCommandList, nAnimationTracks, pHumanModel);
	for (int i = 0; i < n_Animation; ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, true);
	}
}

CMonsterObject::~CMonsterObject()
{
}

void CMonsterObject::Animate(float fTimeElapsed)
{

	//SetPosition(25.0f, 1064.0f, 25.0f);
	//OnPrepareRender();

	if (m_pSkinnedAnimationController)
	{
		/*if (Anime_test_FallingLoop)
			m_pSkinnedAnimationController->AdvanceTime2(fTimeElapsed, this);
		else
			m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);*/
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

	/*CAnimationController* animController = GetSkinnedAnimationController();
	for (int i = 0; i < 5; i++)
	{
		if (test_num == 1 && i == 0) {
			animController->SetTrackWeight(i, 1.0f);
		}
		else if (test_num == 2 && i == 1) {
			animController->SetTrackWeight(i, 1.0f);
		}
		else if (test_num == 3 && i == 4) {
			animController->SetTrackWeight(i, 1.0f);
		}
		else
			animController->SetTrackWeight(i, 0.0f);
	}*/
	//CGameObject::Animate(fTimeElapsed);
	GetStateMachine()->update(fTimeElapsed);
	//m_StateMachine->update(fTimeElapsed);
}

void CMonsterObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {
	CGameObject::Render(pd3dCommandList, pCamera);
	//GetStateMachine()->update(0.01f);
}

//CMultiPlayerObject::CMultiPlayerObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
//	: m_StateMachine(std::make_unique<MultiPlayerStateMachine>(this))
//{
//	Object_type = OBJECT_TPYE_PLAYER;
//	CLoadedModelInfo* pHumanModel = pModel;
//	//pHumanModel->m_pAnimationSets = pHumanModel->m_pAnimationSets->Clone();
//
//	if (!pHumanModel) {
//		pHumanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Gargoyle_LP.bin", NULL);
//		//pHumanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Anubis_lp.bin", NULL);
//	}
//	n_Animation = nAnimationTracks;
//	prevWeights.resize(n_Animation, 0.0f);
//	targetWeights.resize(n_Animation, 0.0f);
//
//	Set_Child(pHumanModel->m_pModelRootObject);
//	m_pSkinnedAnimationController = std::make_shared<CAnimationController>(pd3dDevice, pd3dCommandList, nAnimationTracks, pHumanModel);
//	for (int i = 0; i < n_Animation; ++i) {
//		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
//		m_pSkinnedAnimationController->SetTrackEnable(i, true);
//	}
//
//	m_pSkinnedAnimationController->m_pAnimationTracks[TRACK_DIVEROLL_FORWARD].m_nType = ANIMATION_TYPE_ONCE;
//	m_pSkinnedAnimationController->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_nType = ANIMATION_TYPE_ONCE;
//	m_pSkinnedAnimationController->m_pAnimationTracks[TRACK_GET_UP].m_nType = ANIMATION_TYPE_ONCE;
//
//	SetScale(10.0f, 10.0f, 10.0f);
//}
//
//CMultiPlayerObject::~CMultiPlayerObject()
//{
//}
//
//void CMultiPlayerObject::Animate(float fTimeElapsed)
//{
//
//	//SetPosition(25.0f, 1064.0f, 25.0f);
//	OnPrepareRender();
//
//	if (m_pSkinnedAnimationController)
//	{
//		/*if (Anime_test_FallingLoop)
//			m_pSkinnedAnimationController->AdvanceTime2(fTimeElapsed, this);
//		else
//			m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);*/
//		m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);
//	}
//
//	/*if (On_Ground)
//	{
//		CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
//		XMFLOAT3 xmf3PlayerPosition = GetPosition();
//		XMFLOAT3 world_normal = pTerrain->Get_Mesh_Normal(xmf3PlayerPosition.x, xmf3PlayerPosition.z, last_tile_ptr);
//		AlignWithNormal(world_normal);
//	}*/
//
//	shared_ptr<CGameObject> sibling_ptr = Get_Sibling();
//	if (sibling_ptr != nullptr)
//		sibling_ptr->Animate(fTimeElapsed);
//
//	shared_ptr<CGameObject> child_ptr = Get_Child();
//	if (child_ptr != nullptr)
//		child_ptr->Animate(fTimeElapsed);
//
//	/*CAnimationController* animController = GetSkinnedAnimationController();
//	for (int i = 0; i < 5; i++)
//	{
//		if (test_num == 1 && i == 0) {
//			animController->SetTrackWeight(i, 1.0f);
//		}
//		else if (test_num == 2 && i == 1) {
//			animController->SetTrackWeight(i, 1.0f);
//		}
//		else if (test_num == 3 && i == 4) {
//			animController->SetTrackWeight(i, 1.0f);
//		}
//		else
//			animController->SetTrackWeight(i, 0.0f);
//	}*/
//	//CGameObject::Animate(fTimeElapsed);
//	GetStateMachine()->update(fTimeElapsed);
//	//m_StateMachine->update(fTimeElapsed);
//}
//
//void CMultiPlayerObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {
//	CGameObject::Render(pd3dCommandList, pCamera);
//	//GetStateMachine()->update(0.01f);
//}