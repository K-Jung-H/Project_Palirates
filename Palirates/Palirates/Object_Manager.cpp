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
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_RASTERIZER_DESC BoundingBox_Shader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME; //   D3D12_FILL_MODE_WIREFRAME
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // D3D12_CULL_MODE_BACK
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
	UINT bufferSize = sizeof(BoundingBox_Instance_Info) * rendering_max_num;
	bufferSize = (bufferSize + 255) & ~255;

	Instance_info = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, bufferSize,	D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	Instance_info->Map(0, NULL, (void**)&Mapped_Instance_info);

	m_d3dInstancingBufferView.BufferLocation = Instance_info->GetGPUVirtualAddress();
	m_d3dInstancingBufferView.StrideInBytes = sizeof(BoundingBox_Instance_Info);
	m_d3dInstancingBufferView.SizeInBytes = bufferSize;  // 256 정렬된 크기 사용
}

void OBB_Drawer::Update_OBB_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::vector<std::shared_ptr<CGameObject>>gameobj_container)
{
	std::vector<std::shared_ptr<CGameObject>> obb_obj_ptr_list;
	std::unordered_set<CGameObject*> visited;  // 중복 검사를 위한 컨테이너

	for (std::shared_ptr<CGameObject> obj_ptr : gameobj_container)
	{
		FindOBBObjects(obj_ptr, obb_obj_ptr_list, visited);
	}

	int obb_num = obb_obj_ptr_list.size();

	if (obb_num > rendering_max_num)
	{
		// 새로운 버퍼 크기 재조정 
		// == 크기를 키운 새로운 버퍼 생성
		DebugOutput("\n\nResizing buffer to fit more instances\n\n\n");


		Release_OBB_Data_ShaderVariables();

		// 새로운 최대 크기 업데이트
		rendering_max_num = obb_num * 2;
		rendering_max_num = min(rendering_max_num, MAX_INSTANCING_NUM);

		// 새로운 버퍼 생성
		Create_OBB_Data_ShaderVariables(pd3dDevice, pd3dCommandList);
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
			
			if(obb_obj_ptr_list[i]->Get_Active())
				XMStoreFloat4(&Mapped_Instance_info[i].box_color, Colors::LimeGreen);
			else
				XMStoreFloat4(&Mapped_Instance_info[i].box_color, Colors::Crimson);

		}

		for (int i = obb_num; i < rendering_max_num; ++i)
		{
			Mapped_Instance_info[i].active = false;
			Mapped_Instance_info[i].world_4x4transform = Matrix4x4::Identity();
			XMStoreFloat4(&Mapped_Instance_info[i].box_color, Colors::Crimson);

		}
	}


}

void OBB_Drawer::FindOBBObjects(std::shared_ptr<CGameObject> obj, std::vector<std::shared_ptr<CGameObject>>& obb_obj_ptr_list, std::unordered_set<CGameObject*>& visited)
{
	// 이미 방문한 객체는 생략
	if (!obj || visited.count(obj.get()) > 0)  
		return;

	// 현재 객체 방문 기록 처리
	visited.insert(obj.get());  

	if (obj->Get_Collider() != NULL)
		obb_obj_ptr_list.push_back(obj);

	FindOBBObjects(obj->Get_Child(), obb_obj_ptr_list, visited);
	FindOBBObjects(obj->Get_Sibling(), obb_obj_ptr_list, visited);
}

bool OBB_Drawer::Get_OBB_WorldMatrix(CGameObject* g_obj, XMFLOAT4X4* world_matrix)
{
	if (g_obj->Get_Collider() == NULL)
		return false;

	BoundingOrientedBox pBoundingBox(*g_obj->Get_Collider());
	XMVECTOR extents = XMLoadFloat3(&pBoundingBox.Extents);

	XMMATRIX worldMatrix = XMLoadFloat4x4(&g_obj->m_xmf4x4World);

	XMVECTOR scale, rotation, translation;
	XMMatrixDecompose(&scale, &rotation, &translation, worldMatrix);

	XMMATRIX scaleMatrix = XMMatrixScaling(
		2.0f * XMVectorGetX(extents),
		2.0f * XMVectorGetY(extents),
		2.0f * XMVectorGetZ(extents));

	XMVECTOR obbRotation = XMLoadFloat4(&pBoundingBox.Orientation);
	XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(obbRotation);
	XMMATRIX translationMatrix = XMMatrixTranslationFromVector(translation + XMLoadFloat3(&pBoundingBox.Center));

	XMMATRIX finalWorldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	XMStoreFloat4x4(world_matrix, XMMatrixTranspose(finalWorldMatrix));

	return true;
}


void OBB_Drawer::Release_OBB_Data_ShaderVariables()
{
	if (Instance_info) Instance_info->Unmap(0, NULL);
	if (Instance_info) Instance_info->Release();
}

void OBB_Drawer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	obb_shader->Setting_Render(pd3dCommandList, 0);

	if (obb_Mesh)
		obb_Mesh->Render(pd3dCommandList, m_d3dInstancingBufferView, rendering_max_num);

};



//==================================================

void Fixed_Object_Info::Create_Instance_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT bufferSize = sizeof(Instance_Info) * rendering_max_num;
	bufferSize = (bufferSize + 255) & ~255;

	Instance_info = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, bufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	Instance_info->Map(0, NULL, (void**)&Mapped_Instance_info);

	m_d3dInstancingBufferView.BufferLocation = Instance_info->GetGPUVirtualAddress();
	m_d3dInstancingBufferView.StrideInBytes = sizeof(Instance_Info);
	m_d3dInstancingBufferView.SizeInBytes = bufferSize;  // 256 정렬된 크기 사용

}

void Fixed_Object_Info::Update_Instance_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	int instance_obj_num = fixed_obj_list.size();
	int i = 0;
	XMFLOAT4X4 world_matrix;

	if (instance_obj_num > rendering_max_num)
	{
		DebugOutput("\n\nResizing buffer to fit more" + obj_mesh->Get_Name() + "instances\n\n\n");

		Release_Instance_Data_ShaderVariables();

		rendering_max_num = std::min<int>(instance_obj_num * 2, MAX_INSTANCING_NUM);

		Create_Instance_Data_ShaderVariables(pd3dDevice, pd3dCommandList);
	}


	for (auto& obj_ptr : fixed_obj_list)
	{
		world_matrix = obj_ptr->m_xmf4x4World;

		if (obj_ptr->Get_Active())
			Mapped_Instance_info[i].active = true;
		else
			Mapped_Instance_info[i].active = false;

		Mapped_Instance_info[i].world_4x4transform = world_matrix;
		++i;  
	}

	for (int i = instance_obj_num; i < rendering_max_num; ++i)
	{
		Mapped_Instance_info[i].active = false;
		Mapped_Instance_info[i].world_4x4transform = Matrix4x4::Identity();
	}
}

void Fixed_Object_Info::Release_Instance_Data_ShaderVariables()
{
	if (Instance_info) Instance_info->Unmap(0, NULL);
	if (Instance_info) Instance_info->Release();
}


bool Object_Manager::do_instance_update = false;


Object_Manager::Object_Manager()
{
}

Object_Manager::~Object_Manager()
{

}

void Object_Manager::Add_Object(std::shared_ptr<CGameObject > obj_ptr, Object_Type type)
{
	switch (type)
	{
	case Object_Type::skinned:
	{
		if (obj_ptr->m_pSkinnedAnimationController != NULL)
			skinned_object_list.push_back(obj_ptr);
	}	break;
	case Object_Type::non_skinned:
		non_skinned_object_list.push_back(obj_ptr);
		break;
	case Object_Type::fixed:
	{		
		Add_Object_To_Unordered_Map(obj_ptr, fixed_obj_info_map);
	}	break;
	case Object_Type::etc:
		break;
	default:
		break;
	}
}

void Object_Manager::Add_Object_To_Unordered_Map(std::shared_ptr<CGameObject> obj_ptr, std::unordered_map<std::string, Fixed_Object_Info>& container)
{
	string name = obj_ptr->Get_Mesh_Name();

	if (name != "None") 
	{
		obj_ptr->m_ppMaterials[0]->m_pShader->Set_Instance_Shader();
		container[name].fixed_obj_list.push_back(obj_ptr);

		// 같은 name이 저장되어 있지 않다면 추가 == 메시 중복 검사
		// name이 처음 추가되었을 경우 true 반환
		if (unique_mesh_names.insert(name).second)
			container[name].obj_mesh = std::shared_ptr<CMesh>(obj_ptr->m_pMesh);

		// 기존 raw pointer 해제
		obj_ptr->m_pMesh = nullptr; 
	}

	std::shared_ptr<CGameObject> child_ptr = obj_ptr->Get_Child();

	if (child_ptr != nullptr)
		Add_Object_To_Unordered_Map(child_ptr, container);
	

	std::shared_ptr<CGameObject> sibling_ptr = obj_ptr->Get_Sibling();
	if (sibling_ptr != nullptr)
		Add_Object_To_Unordered_Map(sibling_ptr, container);
	
}

void Object_Manager::Delete_Object(std::shared_ptr<CGameObject > obj_ptr)
{
	//===========[fixed]===========
	auto it = std::find(skinned_object_list.begin(), skinned_object_list.end(), obj_ptr);

	if (it != skinned_object_list.end())
		skinned_object_list.erase(it);


	//===========[fixed]===========
	it = std::find(non_skinned_object_list.begin(), non_skinned_object_list.end(), obj_ptr);

	if (it != non_skinned_object_list.end())
		non_skinned_object_list.erase(it);

	//===========[fixed]===========
	for (auto iter = fixed_obj_info_map.begin(); iter != fixed_obj_info_map.end(); ) 
	{
		auto& obj_vector = iter->second.fixed_obj_list;

		obj_vector.erase(std::remove(obj_vector.begin(), obj_vector.end(), obj_ptr), obj_vector.end());

		if (obj_vector.empty()) 
			iter = fixed_obj_info_map.erase(iter);
		else 
			++iter;
		
	}
}

void Object_Manager::Animate_Objects(Object_Type type, float fTimeElapsed)
{
	switch (type)
	{
	case Object_Type::skinned:
	{
		for ( std::shared_ptr<CGameObject>& obj_ptr : skinned_object_list)
			if (obj_ptr->Get_Active())
				obj_ptr->Animate(fTimeElapsed);
	}
	break;

	case Object_Type::non_skinned:
	{
		for ( std::shared_ptr<CGameObject>& obj_ptr : non_skinned_object_list)
			if (obj_ptr->Get_Active())
			{
				obj_ptr->Animate(fTimeElapsed);
				obj_ptr->UpdateTransform(NULL);
			}
	}
	break;

	case Object_Type::fixed:
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

void Object_Manager::Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (do_instance_update == false)
		return;
	else
		do_instance_update = true; // 다음 Update 호출 전까지는 인스턴스 정보 유지하기

	for (auto& pair : fixed_obj_info_map)
	{
		Fixed_Object_Info& info = pair.second;
		if (info.Instance_info == NULL)
			info.Create_Instance_Data_ShaderVariables(pd3dDevice, pd3dCommandList);
		else
			info.Update_Instance_Data(pd3dDevice, pd3dCommandList);
	}
}

void Object_Manager::Render_Objects(Object_Type type, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	switch (type)
	{
	case Object_Type::skinned:
	{
		for (std::shared_ptr<CGameObject>& skinned_obj_ptr : skinned_object_list)
		{	
			if (skinned_obj_ptr->Get_Active())
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
			if (obj_ptr->Get_Active())
				obj_ptr->Render(pd3dCommandList, pCamera);
	}
	break;

	case Object_Type::fixed:
	{
		// 같은 메시를 사용하는 객체끼리 모두 벡터에 저장하였음
		// 이를 이용하여, 사물들을 위치 정보를 인스턴싱하는 업데이트 함수를 작성하고
		// 사물들의 위치정보를 저장한 인스턴싱 버퍼를 연결하여 렌더링
		// 최종 목표 == 메시별로 렌더링 함수 1번만 호출하는 것

		for (auto& [meshName, instance_info] : fixed_obj_info_map)
		{
			int Material_N = instance_info.fixed_obj_list[0]->m_nMaterials;
			if (Material_N > 0)
			{
				// 재료(Material) 처리
				for (int i = 0; i < Material_N; ++i)
				{					
					CMaterial* pMaterial = instance_info.fixed_obj_list[0]->m_ppMaterials[i];
					if (pMaterial)
					{
						CShader* pShader = pMaterial->m_pShader;
						if (pShader)
						{
							// PSO 순회 및 렌더링
							int pipelineStateNum = pShader->Get_Num_PipelineState();
							for (int j = 0; j < pipelineStateNum; ++j)
							{
								// PSO 설정
								pShader->Setting_Render(pd3dCommandList, 1);

								// 재료(Material) 셰이더 변수 업데이트
								pMaterial->UpdateShaderVariable(pd3dCommandList);
							}
						}
						else
						{
							// 셰이더가 없는 경우에도 재료 업데이트 후 메쉬 렌더링
							pMaterial->UpdateShaderVariable(pd3dCommandList);
						}

						// 메쉬 렌더링
						if (instance_info.obj_mesh)
							instance_info.obj_mesh->Instancing_Render(pd3dCommandList, instance_info.m_d3dInstancingBufferView, instance_info.rendering_max_num);
					}
				}
			}
		}
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
//	Render_Objects(Object_Type::skinned, pd3dCommandList, pCamera);
//	Render_Objects(Object_Type::non_skinned, pd3dCommandList, pCamera);
	Render_Objects(Object_Type::fixed, pd3dCommandList, pCamera);

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

std::unordered_map<std::string, Fixed_Object_Info>* Object_Manager::Get_Object_List_Map(Object_Type type)
{
	switch (type)
	{
	case Object_Type::fixed:
		return &fixed_obj_info_map;
		break;

	case Object_Type::skinned:
	case Object_Type::non_skinned:
	case Object_Type::etc:
	default:
		DebugOutput("Object_Manager::Get_Object_List_Map() - Using_Wrong_Type");
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

	case Object_Type::fixed:
		for (auto& pair : fixed_obj_info_map) 
		{
			Fixed_Object_Info& info = pair.second;

			info.fixed_obj_list.clear();
			info.fixed_obj_list.shrink_to_fit(); 

			info.obj_mesh.reset(); // 강제로 nullptr로 설정

			// 수동 할당된 메모리 
			if (info.Instance_info)
			{
				info.Instance_info -> Unmap(0, NULL);
				info.Instance_info->Release();
				info.Instance_info = nullptr;
			}
		}

		// 컨테이너 자체를 완전히 비우고 메모리 해제
		fixed_obj_info_map.clear();
		unique_mesh_names.clear();

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
	Clear_Object_List(Object_Type::fixed);

}


//==================================================

void Object_Manager::Create_OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	bounding_box_drawer = make_shared<OBB_Drawer>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	bounding_box_drawer->Create_OBB_Data_ShaderVariables(pd3dDevice, pd3dCommandList);
}

void Object_Manager::Update_OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::vector<std::shared_ptr<CGameObject>>gameobj_container)
{
	bounding_box_drawer->Update_OBB_Data(pd3dDevice, pd3dCommandList, gameobj_container);
}
void Object_Manager::Render_OBB_Drawer(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	bounding_box_drawer->Render(pd3dCommandList, pCamera);
}