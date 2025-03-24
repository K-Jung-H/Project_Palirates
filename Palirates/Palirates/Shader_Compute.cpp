#include "stdafx.h"
#include "Shader_Compute.h"
#include "Scene.h"


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

void Post_ComputeShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState)
{
	ID3DBlob* pd3dComputeShaderBlob = NULL;

	D3D12_CACHED_PIPELINE_STATE d3dCachedPipelineState = { };
	D3D12_COMPUTE_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dRootSignature;
	d3dPipelineStateDesc.CS = CreateComputeShader(&pd3dComputeShaderBlob, nPipelineState);
	d3dPipelineStateDesc.NodeMask = 0;
	d3dPipelineStateDesc.CachedPSO = d3dCachedPipelineState;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateComputePipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[nPipelineState]);

	if (pd3dComputeShaderBlob) pd3dComputeShaderBlob->Release();

	m_cxThreadGroups = cxThreadGroups;
	m_cyThreadGroups = cyThreadGroups;
	m_czThreadGroups = czThreadGroups;
}

void Post_ComputeShader::CreateResourcesAndUavs(ID3D12Device* pd3dDevice, UINT index, DXGI_FORMAT format)
{
	// 결과물 저장을 위한 1개의 텍스처만 관리한다고 가정
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 0, 1, 0, 0, 1, 0);

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = format;

	m_pTexture->CreateTexture(pd3dDevice, nullptr, 0, RESOURCE_TEXTURE2D, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT,
		1, 1, format, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, &clearValue);

	// UAV 뷰 생성 - 결과물 저장용
	int Test_RootParamater_Index = 0;
	CDescriptor_Heap::CreateComputeUnorderedAccessViews(pd3dDevice, m_pTexture, 1, 0, Test_RootParamater_Index);

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

