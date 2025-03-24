#pragma once
#include "Shader.h"

class CComputeShader : public CShader
{
public:
	CComputeShader();
	virtual ~CComputeShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState = 0);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1, int nPipelineState = 0);

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);
	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState = 0);

protected:
	UINT							m_cxThreadGroups = 0;
	UINT							m_cyThreadGroups = 0;
	UINT							m_czThreadGroups = 0;
};