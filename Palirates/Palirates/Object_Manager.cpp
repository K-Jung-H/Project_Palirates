#include "stdafx.h"
#include "Object_Manager.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


BoundingBox_Shader::BoundingBox_Shader()
{

}

BoundingBox_Shader::~BoundingBox_Shader()
{
}
void BoundingBox_Shader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
}

D3D12_INPUT_LAYOUT_DESC BoundingBox_Shader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 8;  
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	// 정점 정보를 위한 입력 원소들
	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	// 인스턴싱 정보를 위한 입력 원소들
	pd3dInputElementDescs[2] = { "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[3] = { "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[4] = { "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[5] = { "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	// 인스턴스 색상
	pd3dInputElementDescs[6] = { "INSTANCECOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	// 인스턴싱에서 bool 값을 전달하려면 UINT (0, 1)으로 변환
	pd3dInputElementDescs[7] = { "INSTANCEBOOL", 0, DXGI_FORMAT_R32_UINT, 1, 80, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;
	return(d3dInputLayoutDesc);
}

D3D12_DEPTH_STENCIL_DESC BoundingBox_Shader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

D3D12_RASTERIZER_DESC BoundingBox_Shader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_SHADER_BYTECODE BoundingBox_Shader::CreateVertexShader(ID3DBlob** VertexShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VS_BoundingBox", "vs_5_1", VertexShaderBlob));
	else
	{
		D3D12_SHADER_BYTECODE d3dShaderByteCode = { 0, NULL };
		return 		d3dShaderByteCode;
	}
}

D3D12_SHADER_BYTECODE BoundingBox_Shader::CreatePixelShader(ID3DBlob** PixelShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0)
		return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PS_BoundingBox", "ps_5_1", PixelShaderBlob));
	else
	{
		D3D12_SHADER_BYTECODE d3dShaderByteCode = { 0, NULL };
		return 		d3dShaderByteCode;
	}
}


CubeMesh* OBB_Drawer::obb_Mesh = NULL;
BoundingBox_Shader* OBB_Drawer::obb_shader = NULL;

OBB_Drawer::OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	if (obb_Mesh == NULL)
		obb_Mesh = new CubeMesh(pd3dDevice, pd3dCommandList);
	

	if (obb_shader == NULL)
	{
		obb_shader = new BoundingBox_Shader();
		obb_shader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		obb_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}
}

OBB_Drawer::~OBB_Drawer()
{
	Release_OBB_Data_ShaderVariables();
	obb_Mesh->Release();
	delete obb_shader;

}


void OBB_Drawer::Create_OBB_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = rendering_max_num * ((sizeof(BoundingBox_Instance_Info) + 255) & ~255); //256의 배수
	Instance_info = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(BoundingBox_Instance_Info) * rendering_max_num, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	Instance_info->Map(0, NULL, (void**)&Mapped_Instance_info);

	m_d3dInstancingBufferView.BufferLocation = Instance_info->GetGPUVirtualAddress();
	m_d3dInstancingBufferView.StrideInBytes = sizeof(BoundingBox_Instance_Info);
	m_d3dInstancingBufferView.SizeInBytes = sizeof(BoundingBox_Instance_Info) * rendering_max_num;
}

void OBB_Drawer::Update_OBB_Data(ID3D12GraphicsCommandList* pd3dCommandList, std::vector<std::shared_ptr<CGameObject>>gameobj_container)
{
	std::vector<std::shared_ptr<CGameObject>> obb_obj_ptr_list;

	for (std::shared_ptr<CGameObject> obj_ptr : gameobj_container)
	{
		FindOBBObjects(obj_ptr, obb_obj_ptr_list);
	}

	int obb_num = obb_obj_ptr_list.size();

	if (obb_num > rendering_max_num)
	{
		// 새로운 버퍼 크기 재조정 
		// == 크기를 키운 새로운 버퍼 생성
		DebugOutput("Not Prepared");
	}
	else
	{
		XMFLOAT4X4 world_matrix;
		for (int i = 0; i < obb_num; ++i)
		{
			// 오류 방지
			if (Get_OBB_WorldMatrix(obb_obj_ptr_list[i].get(), &world_matrix))
				Mapped_Instance_info[i].active = true;
			else
				Mapped_Instance_info[i].active = false;

			Mapped_Instance_info[i].world_4x4transform = world_matrix;
			Mapped_Instance_info[i].box_color = { 0.0f, 1.0f, 0.0f, 1.0f };
		}

		for (int i = obb_num; i < rendering_max_num; ++i)
		{
			Mapped_Instance_info[i].active = false;
			Mapped_Instance_info[i].box_color = { 1.0f, 0.0f, 0.0f, 1.0f };
			Mapped_Instance_info[i].world_4x4transform = Matrix4x4::Identity();
		}
	}


}

void OBB_Drawer::FindOBBObjects(std::shared_ptr<CGameObject> obj, std::vector<std::shared_ptr<CGameObject>>& obb_obj_ptr_list)
{
	if (!obj || obj->Get_Collider() == NULL)
		return;

	obb_obj_ptr_list.push_back(obj); 


	FindOBBObjects(obj->Get_Child(), obb_obj_ptr_list);


	FindOBBObjects(obj->Get_Sibling(), obb_obj_ptr_list);
}


bool OBB_Drawer::Get_OBB_WorldMatrix(CGameObject* g_obj, XMFLOAT4X4* world_matrix)
{
	// 게임 오브젝트에서 BoundingOrientedBox를 가져오고, NULL 체크
	BoundingOrientedBox* pBoundingBox = g_obj->Get_Collider();
	if (pBoundingBox == NULL)
		return NULL;

	// BoundingOrientedBox의 Extents 값을 가져옵니다 (필요한 값만 사용)
	XMVECTOR extents = XMLoadFloat3(&pBoundingBox->Extents);

	// 게임 오브젝트의 위치와 회전값을 로드
	XMVECTOR center = XMLoadFloat3(&pBoundingBox->Center);
	XMMATRIX worldMatrix = XMLoadFloat4x4(&g_obj->m_xmf4x4World);

	// 월드 행렬에서 회전 행렬 추출
	XMVECTOR scale, rotation, translation;
	XMMatrixDecompose(&scale, &rotation, &translation, worldMatrix);

	// 회전 행렬을 기준으로 BoundingOrientedBox에 적용할 변환 행렬을 구성
	XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotation);
	XMMATRIX scaleMatrix = XMMatrixScaling(
		2.0f * XMVectorGetX(extents),
		2.0f * XMVectorGetY(extents),
		2.0f * XMVectorGetZ(extents));

	XMMATRIX translationMatrix = XMMatrixTranslationFromVector(center);

	// 최종 월드 행렬 계산 (스케일, 회전, 이동 순서로 적용)
	XMMATRIX finalWorldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	// 행렬을 XMFLOAT4X4 형식으로 저장 (HLSL에서 사용하는 행렬은 행 우선(row-major) 형태이므로 전치 필요)
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(finalWorldMatrix));

	return &xmf4x4World;
}

void OBB_Drawer::Release_OBB_Data_ShaderVariables()
{
	if (Instance_info) Instance_info->Unmap(0, NULL);
	if (Instance_info) Instance_info->Release();
}

void OBB_Drawer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (obb_Mesh)
		obb_Mesh->Render(pd3dCommandList, m_d3dInstancingBufferView, rendering_max_num);

};



//==================================================

Object_Manager::Object_Manager()
{
}

Object_Manager::~Object_Manager()
{

}

void Object_Manager::Add_Object(std::shared_ptr<CGameObject > obj_ptr)
{
	if (obj_ptr->m_pSkinnedAnimationController != NULL)
		skinned_object_list.push_back(obj_ptr);
	else
		non_skinned_object_list.push_back(obj_ptr);
}

void Object_Manager::Delete_Object(std::shared_ptr<CGameObject > obj_ptr)
{
	auto it = std::find(skinned_object_list.begin(), skinned_object_list.end(), obj_ptr);

	if (it != skinned_object_list.end()) 
		skinned_object_list.erase(it);
	

	it = std::find(non_skinned_object_list.begin(), non_skinned_object_list.end(), obj_ptr);

	if (it != non_skinned_object_list.end()) 
		non_skinned_object_list.erase(it);
	

}

void Object_Manager::Animate_Objects(Object_Type type, float fTimeElapsed)
{
	switch (type)
	{
	case Object_Type::skinned:
	{
		for ( std::shared_ptr<CGameObject>& obj_ptr : skinned_object_list)
			if (obj_ptr->Active)
				obj_ptr->Animate(fTimeElapsed);
	}
	break;

	case Object_Type::non_skinned:
	{
		for ( std::shared_ptr<CGameObject>& obj_ptr : non_skinned_object_list)
			if (obj_ptr->Active)
			{
				obj_ptr->Animate(fTimeElapsed);
				obj_ptr->UpdateTransform(NULL);
			}
	}
	break;

	case Object_Type::etc:
	default:
	{
		DebugOutput("Object_Manager::Animate_Objects() - Using_Wrong_Type");
		::PostQuitMessage(0);
	}
	break;

	}

}

void Object_Manager::Animate_Objects_All(float fTimeElapsed)
{
	Animate_Objects(Object_Type::skinned, fTimeElapsed);
	Animate_Objects(Object_Type::non_skinned, fTimeElapsed);

}

void Object_Manager::Render_Objects(Object_Type type, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	switch (type)
	{
	case Object_Type::skinned:
	{
		for (std::shared_ptr<CGameObject>& skinned_obj_ptr : skinned_object_list)
		{	
			if (skinned_obj_ptr->Active)
			{
				skinned_obj_ptr->UpdateTransform(NULL);
				skinned_obj_ptr->Render(pd3dCommandList, pCamera);
			}
		}
	}
	break;

	case Object_Type::non_skinned:
	{
		for (std::shared_ptr<CGameObject>& obj_ptr : non_skinned_object_list)
			if (obj_ptr->Active)
				obj_ptr->Render(pd3dCommandList, pCamera);
	}
	break;

	case Object_Type::etc:
	default:
	{
		DebugOutput("Object_Manager::Render_Objects() - Using_Wrong_Type");
		::PostQuitMessage(0);
	}
	break;
	}


}

void Object_Manager::Render_Objects_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	Render_Objects(Object_Type::skinned, pd3dCommandList, pCamera);
	Render_Objects(Object_Type::non_skinned, pd3dCommandList, pCamera);
}

std::vector<std::shared_ptr<CGameObject>>* Object_Manager::Get_Object_List(Object_Type type)
{
	switch (type)
	{
	case Object_Type::skinned:
		return &skinned_object_list;
		break;

	case Object_Type::non_skinned:
		return &non_skinned_object_list;
		break;
	
	case Object_Type::etc:	
	default:
		DebugOutput("Object_Manager::Get_Object_List() - Using_Wrong_Type");
		::PostQuitMessage(0);
		break;
	}
}

void Object_Manager::Clear_Object_List(Object_Type type)
{
	switch (type)
	{
	case Object_Type::skinned:
		skinned_object_list.clear();
		skinned_object_list.shrink_to_fit();
		break;

	case Object_Type::non_skinned:
		non_skinned_object_list.clear();
		non_skinned_object_list.shrink_to_fit();
		break;

	case Object_Type::etc:
	default:
		DebugOutput("Object_Manager::Clear_Object_List() - Using_Wrong_Type");
		::PostQuitMessage(0);
		break;
	}

}

void Object_Manager::Clear_Object_List_All()
{
	Clear_Object_List(Object_Type::skinned);
	Clear_Object_List(Object_Type::non_skinned);
}


//==================================================

void Object_Manager::Create_OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	bounding_box_drawer = make_shared<OBB_Drawer>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	bounding_box_drawer->Create_OBB_Data_ShaderVariables(pd3dDevice, pd3dCommandList);
}

void Object_Manager::Update_OBB_Drawer(ID3D12GraphicsCommandList* pd3dCommandList, std::vector<std::shared_ptr<CGameObject>>gameobj_container)
{
	bounding_box_drawer->Update_OBB_Data(pd3dCommandList, gameobj_container);
}
void Object_Manager::Render_OBB_Drawer(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	bounding_box_drawer->Render(pd3dCommandList, pCamera);
}