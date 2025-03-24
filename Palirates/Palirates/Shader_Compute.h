#pragma once
#include "Shader.h"

class Post_ComputeShader : public PostProcessBaseShader
{
public:
	Post_ComputeShader();
	virtual ~Post_ComputeShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState = 0);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups = 1, UINT cyThreadGroups = 1, UINT czThreadGroups = 1, int nPipelineState = 0);
	void CreateResourcesAndUavs(ID3D12Device* pd3dDevice, UINT index, DXGI_FORMAT format);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);
	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState = 0);

protected:
	UINT							m_cxThreadGroups = 0;
	UINT							m_cyThreadGroups = 0;
	UINT							m_czThreadGroups = 0;
};


class CEdgeDetectCSShader : public Post_ComputeShader
{
public:
	CEdgeDetectCSShader();
	virtual ~CEdgeDetectCSShader();

	// UAV 출력용 리소스 생성

	// 추가: Compute Dispatch 실행
	void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList) {}

	// 결과 렌더링용 Fullscreen Quad
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {};
};