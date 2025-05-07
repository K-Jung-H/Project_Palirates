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
	m_ngraphicsPipelineStates = 1;
	m_ppd3dgraphicsPipelineStates = new ID3D12PipelineState * [m_ngraphicsPipelineStates];

	CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
}

D3D12_INPUT_LAYOUT_DESC BoundingBox_Shader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 7;  
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


CubeMesh* OBB_Drawer::obb_Mesh = nullptr;
BoundingBox_Shader* OBB_Drawer::obb_shader = nullptr;

OBB_Drawer::OBB_Drawer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig)
{
	if (!obb_Mesh)
		obb_Mesh = new CubeMesh(device, cmdList);

	if (!obb_shader)
	{
		obb_shader = new BoundingBox_Shader();
		obb_shader->CreateShader(device, cmdList, rootSig);
		obb_shader->CreateShaderVariables(device, cmdList);
	}
}

OBB_Drawer::~OBB_Drawer()
{
	Release_OBB_Data_ShaderVariables();
	if (obb_Mesh) obb_Mesh->Release();
	delete obb_shader;
}

void OBB_Drawer::Create_OBB_Data_ShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	UINT bufferSize = sizeof(BoundingBox_Instance_Info) * obb_instance_buffer_max_num;
	bufferSize = (bufferSize + 255) & ~255;


	Instance_info = CreateBufferResource(device, cmdList, nullptr, bufferSize,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	Instance_info->Map(0, nullptr, reinterpret_cast<void**>(&Mapped_Instance_info));

	m_d3dInstancingBufferView.BufferLocation = Instance_info->GetGPUVirtualAddress();
	m_d3dInstancingBufferView.StrideInBytes = sizeof(BoundingBox_Instance_Info);
	m_d3dInstancingBufferView.SizeInBytes = bufferSize;

}

void OBB_Drawer::Release_OBB_Data_ShaderVariables()
{

	if (Instance_info)
	{
		Instance_info->Unmap(0, nullptr);
		Instance_info->Release();
		Instance_info = nullptr;
	}
}


void OBB_Drawer::Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Object_Type type, Object_Manager* obj_mgr)
{
	switch (type)
	{
	case Object_Type::fixed:
		if (auto map_ptr = obj_mgr->Get_Object_List_Map(type))
			Update_From_Map(device, cmdList, *map_ptr);
		break;

	case Object_Type::skinned:
	case Object_Type::non_skinned:
		if (auto vec_ptr = obj_mgr->Get_Object_List(type))
			Update_From_Vector(device, cmdList, *vec_ptr);
		break;

	default:
		break;
	}
}


struct ObjectOBB
{
	std::shared_ptr<CGameObject> obj;
	BoundingOrientedBox obb;
	XMFLOAT4X4 worldMatrix;
};

void OBB_Drawer::Update_From_Vector(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<CGameObject>>& obj_list)
{
	std::vector<std::shared_ptr<CGameObject>> obb_list;
	std::unordered_set<CGameObject*> visited;


	for (auto& obj : obj_list)
		FindOBBObjects(obj, obb_list, visited);

	int count = static_cast<int>(obb_list.size());
	if (count > obb_instance_buffer_max_num)
	{
		Release_OBB_Data_ShaderVariables();
		obb_instance_buffer_max_num = std::min(count * 2, MAX_INSTANCING_NUM);
		Create_OBB_Data_ShaderVariables(device, cmdList);
	}

	std::vector<ObjectOBB> col_obb_list;
	int visible_count = 0;
	for (auto& obj : obb_list)
	{
		XMFLOAT4X4 world_matrix;

		if (!obj->GetbUpdateOBB())
			continue;

		std::optional<BoundingOrientedBox> obb = Get_OBB_WorldMatrix(obj.get(), &world_matrix);

		if (!obb.has_value())
			continue;

		Mapped_Instance_info[visible_count].world_4x4transform = world_matrix;
		XMStoreFloat4(&Mapped_Instance_info[visible_count].box_color, obj->Get_Active() ? Colors::LimeGreen : Colors::Crimson);
		++visible_count;

		col_obb_list.push_back(ObjectOBB{ obj, obb.value(), world_matrix });
	}

	rendering_num = visible_count;

	for (size_t i = 0; i < col_obb_list.size(); ++i)
	{
		for (size_t j = i + 1; j < col_obb_list.size(); ++j)
		{
			auto typeA = col_obb_list[i].obj->Object_type;
			auto typeB = col_obb_list[j].obj->Object_type;

			bool validPair =
				(typeA == OBJECT_TPYE_MAIN_PLAYER && typeB == OBJECT_TPYE_MONSTER_WEAPON) ||
				(typeA == OBJECT_TPYE_MONSTER_WEAPON && typeB == OBJECT_TPYE_MAIN_PLAYER) ||

				(typeA == OBJECT_TPYE_MONSTER && typeB == OBJECT_TPYE_PLAYER_WEAPON) ||
				(typeA == OBJECT_TPYE_PLAYER_WEAPON && typeB == OBJECT_TPYE_MONSTER);

			if (!validPair) continue; 

			const BoundingOrientedBox& a = col_obb_list[i].obb;
			const BoundingOrientedBox& b = col_obb_list[j].obb;

			if (a.Intersects(b))
			{
				/*const char* nameA = col_obb_list[i].obj->Get_Name();
				const char* nameB = col_obb_list[j].obj->Get_Name();

				char buffer[256];
				sprintf_s(buffer, "OBB 충돌 감지: [%s] <--> [%s]\n", nameA, nameB);
				OutputDebugStringA(buffer);*/

				if (typeA == OBJECT_TPYE_MAIN_PLAYER && typeB == OBJECT_TPYE_MONSTER_WEAPON) {
					std::shared_ptr<CTerrainPlayer> p = std::dynamic_pointer_cast<CTerrainPlayer>(col_obb_list[i].obj);
					if (p)
					{
						if (p->GetStateMachine()->Get_State() != State::Get_Hit_F2)
							p->GetStateMachine()->changeState(State::Get_Hit_F2, Key_Value::None);
					}
					continue;
				}
				if (typeA == OBJECT_TPYE_MONSTER_WEAPON && typeB == OBJECT_TPYE_MAIN_PLAYER) {
					std::shared_ptr<CTerrainPlayer> p = std::dynamic_pointer_cast<CTerrainPlayer>(col_obb_list[j].obj);
					if (p)
					{
						if (p->GetStateMachine()->Get_State() != State::Get_Hit_F2)
							p->GetStateMachine()->changeState(State::Get_Hit_F2, Key_Value::None);
					}
					continue;
				}

				if (typeA == OBJECT_TPYE_MONSTER && typeB == OBJECT_TPYE_PLAYER_WEAPON) {
					std::shared_ptr<CMonsterObject> monster = std::dynamic_pointer_cast<CMonsterObject>(col_obb_list[i].obj);
					if (monster)
					{
						if (monster->GetStateMachine()->Get_State() != State::Get_Hit)
							monster->GetStateMachine()->changeState(State::Get_Hit, Key_Value::None);
					}
					continue;
				}
				if (typeA == OBJECT_TPYE_PLAYER_WEAPON && typeB == OBJECT_TPYE_MONSTER) {
					std::shared_ptr<CMonsterObject> monster = std::dynamic_pointer_cast<CMonsterObject>(col_obb_list[j].obj);
					if (monster)
					{
						if (monster->GetStateMachine()->Get_State() != State::Get_Hit)
							monster->GetStateMachine()->changeState(State::Get_Hit, Key_Value::None);
					}
					continue;
				}
			}
		}
	}
}

void OBB_Drawer::Update_From_Map(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::unordered_map<std::string, Fixed_Object_Info>& obj_map)
{
	int total = 0;
	for (const auto& [_, info] : obj_map)
		total += static_cast<int>(info.fixed_obj_list.size());

	if (total > obb_instance_buffer_max_num)
	{
		Release_OBB_Data_ShaderVariables();
		obb_instance_buffer_max_num = std::min(total * 2, MAX_INSTANCING_NUM);
		Create_OBB_Data_ShaderVariables(device, cmdList);
	}

	int visible_count = 0;
	for (const auto& [_, info] : obj_map)
	{
		if (!info.obj_mesh || !info.obj_mesh->Get_BoundingBox()) continue;
		const BoundingOrientedBox& meshOBB = *info.obj_mesh->Get_BoundingBox();

		for (const auto& obj : info.fixed_obj_list)
		{

			XMFLOAT4X4 world;
			if (!Compute_Fixed_OBB_WorldMatrix(meshOBB, obj->m_xmf4x4World, world))
				continue;

			Mapped_Instance_info[visible_count].world_4x4transform = world;
			XMStoreFloat4(&Mapped_Instance_info[visible_count].box_color,
				obj->Get_Active() ? Colors::LimeGreen : Colors::Crimson);
			++visible_count;
		}
	}

	rendering_num = visible_count;
}

//bool OBB_Drawer::Get_OBB_WorldMatrix(CGameObject* g_obj, XMFLOAT4X4* world_matrix)
//{
//	if (!g_obj || !g_obj->Get_Collider())
//		return false;
//
//	CGameObject* target = g_obj;
//	if (target)
//	{
//		CSkinnedMesh* skinnedMesh = dynamic_cast<CSkinnedMesh*>(target->m_pMesh);
//		if (skinnedMesh)
//		{
//			BoundingOrientedBox obb = skinnedMesh->Get_WorldOBB();
//			XMMATRIX scaleMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&obb.Extents) * 2.0f);
//			XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
//			XMMATRIX translationMatrix = XMMatrixTranslationFromVector(XMLoadFloat3(&obb.Center));
//
//			XMMATRIX finalMatrix = scaleMatrix * g_obj->customRotation * rotationMatrix * translationMatrix;
//			XMStoreFloat4x4(world_matrix, XMMatrixTranspose(finalMatrix));
//			XMStoreFloat4x4(&g_obj->WeaponMatrix, finalMatrix);
//
//			return true;
//		}
//		else
//		{
//			BoundingOrientedBox localOBB = *g_obj->Get_Collider();
//			BoundingOrientedBox worldOBB = {};
//			XMMATRIX world = XMLoadFloat4x4(&g_obj->m_xmf4x4World);
//			localOBB.Transform(worldOBB, world);
//
//			XMVECTOR scale, rotQuat, trans;
//			if (!XMMatrixDecompose(&scale, &rotQuat, &trans, world))
//				rotQuat = XMQuaternionIdentity();
//
//			XMStoreFloat4(&worldOBB.Orientation, rotQuat);
//
//			if (worldOBB.Extents.x <= 0.0f || worldOBB.Extents.y <= 0.0f || worldOBB.Extents.z <= 0.0f)
//				return false;
//
//
//			XMMATRIX obbMatrix =
//				XMMatrixScaling(worldOBB.Extents.x * 2.0f, worldOBB.Extents.y * 2.0f, worldOBB.Extents.z * 2.0f) *
//				XMMatrixRotationQuaternion(XMLoadFloat4(&worldOBB.Orientation)) *
//				XMMatrixTranslationFromVector(XMLoadFloat3(&worldOBB.Center));
//
//			XMStoreFloat4x4(world_matrix, XMMatrixTranspose(obbMatrix));
//			XMStoreFloat4x4(&g_obj->WeaponMatrix, obbMatrix);
//			return true;
//		}
//	}
//	return false;
//}

std::optional<BoundingOrientedBox> OBB_Drawer::Get_OBB_WorldMatrix(CGameObject* g_obj, XMFLOAT4X4* world_matrix)
{
	if (!g_obj || !g_obj->Get_Collider())
		return std::nullopt;

	if (!g_obj->GetbUpdateOBB())
		return std::nullopt;

	CGameObject* target = g_obj;

	if (target)
	{
		CSkinnedMesh* skinnedMesh = dynamic_cast<CSkinnedMesh*>(target->m_pMesh);
		if (skinnedMesh)
		{
			BoundingOrientedBox obb = skinnedMesh->Get_WorldOBB();

			XMMATRIX scaleMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&obb.Extents) * 2.0f);
			XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
			XMMATRIX translationMatrix = XMMatrixTranslationFromVector(XMLoadFloat3(&obb.Center));

			XMMATRIX finalMatrix = scaleMatrix * g_obj->customRotation * rotationMatrix * translationMatrix;
			XMStoreFloat4x4(world_matrix, XMMatrixTranspose(finalMatrix));
			XMStoreFloat4x4(&g_obj->WeaponMatrix, finalMatrix);

			if (XMVector3NearEqual(XMLoadFloat3(&obb.Center), XMVectorZero(), XMVectorReplicate(0.001f)) &&
				XMVector4NearEqual(XMLoadFloat4(&obb.Orientation), XMVectorSet(1, 0, 0, 0), XMVectorReplicate(0.001f)))
				return std::nullopt;
			return obb;
		}
		else
		{
			BoundingOrientedBox localOBB = *g_obj->Get_Collider();
			BoundingOrientedBox worldOBB = {};
			XMMATRIX world = XMLoadFloat4x4(&g_obj->m_xmf4x4World);
			localOBB.Transform(worldOBB, world);

			XMVECTOR scale, rotQuat, trans;
			if (!XMMatrixDecompose(&scale, &rotQuat, &trans, world))
				rotQuat = XMQuaternionIdentity();

			XMStoreFloat4(&worldOBB.Orientation, rotQuat);

			if (worldOBB.Extents.x <= 0.0f || worldOBB.Extents.y <= 0.0f || worldOBB.Extents.z <= 0.0f)
				return std::nullopt;

			XMMATRIX obbMatrix =
				XMMatrixScaling(worldOBB.Extents.x * 2.0f, worldOBB.Extents.y * 2.0f, worldOBB.Extents.z * 2.0f) *
				XMMatrixRotationQuaternion(XMLoadFloat4(&worldOBB.Orientation)) *
				XMMatrixTranslationFromVector(XMLoadFloat3(&worldOBB.Center));

			XMStoreFloat4x4(world_matrix, XMMatrixTranspose(obbMatrix));
			XMStoreFloat4x4(&g_obj->WeaponMatrix, obbMatrix);

			if (XMVector3NearEqual(XMLoadFloat3(&worldOBB.Center), XMVectorZero(), XMVectorReplicate(0.001f)) &&
				XMVector4NearEqual(XMLoadFloat4(&worldOBB.Orientation), XMVectorSet(1, 0, 0, 0), XMVectorReplicate(0.001f)))
				return std::nullopt;
			return worldOBB;
		}
	}

	return std::nullopt;
}

bool OBB_Drawer::Compute_Fixed_OBB_WorldMatrix(const BoundingOrientedBox& localOBB, const XMFLOAT4X4& objectWorld, XMFLOAT4X4& out_world)
{
	XMMATRIX objWorld = XMLoadFloat4x4(&objectWorld);

	XMVECTOR scale = XMVectorSet(
		localOBB.Extents.x * 2.0f,
		localOBB.Extents.y * 2.0f,
		localOBB.Extents.z * 2.0f,
		0.0f
	);

	XMMATRIX scaleMatrix = XMMatrixScalingFromVector(scale);
	XMMATRIX rotMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&localOBB.Orientation));
	XMMATRIX offsetMatrix = XMMatrixTranslationFromVector(XMLoadFloat3(&localOBB.Center));

	XMMATRIX localOBBMatrix = scaleMatrix * rotMatrix * offsetMatrix;

	XMMATRIX finalMatrix = localOBBMatrix * objWorld;

	XMStoreFloat4x4(&out_world, XMMatrixTranspose(finalMatrix));
	return true;

}

void OBB_Drawer::FindOBBObjects(std::shared_ptr<CGameObject> obj, std::vector<std::shared_ptr<CGameObject>>& obb_list, std::unordered_set<CGameObject*>& visited)
{
	if (!obj || visited.count(obj.get()) > 0) return;


	visited.insert(obj.get());
	if (obj->Get_Collider()) obb_list.push_back(obj);

	FindOBBObjects(obj->Get_Child(), obb_list, visited);
	FindOBBObjects(obj->Get_Sibling(), obb_list, visited);
}


void OBB_Drawer::Render(ID3D12GraphicsCommandList* cmdList, CCamera* camera)
{
	obb_shader->Setting_Render(cmdList, 0);
	if (obb_Mesh)
		obb_Mesh->Render(cmdList, m_d3dInstancingBufferView, rendering_num);
}

//==================================================

void Fixed_Object_Info::Create_Instance_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT bufferSize = sizeof(Instance_Info) * instance_buffer_max_num;
	bufferSize = (bufferSize + 255) & ~255;

	Instance_info = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, bufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	Instance_info->Map(0, NULL, (void**)&Mapped_Instance_info);

	m_d3dInstancingBufferView.BufferLocation = Instance_info->GetGPUVirtualAddress();
	m_d3dInstancingBufferView.StrideInBytes = sizeof(Instance_Info);
	m_d3dInstancingBufferView.SizeInBytes = bufferSize;  // 256 정렬된 크기 사용

}



void Fixed_Object_Info::Update_Instance_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	int instance_obj_num = fixed_obj_list.size();
	int visible_count = 0;

	XMFLOAT4X4 world_matrix;

	if (instance_obj_num > instance_buffer_max_num)
	{
//		DebugOutput("\n\nResizing buffer to fit more" + obj_mesh->Get_Name() + "instances\n\n\n");

		Release_Instance_Data_ShaderVariables();

		instance_buffer_max_num = std::min<int>(instance_obj_num * 2, MAX_INSTANCING_NUM);

		Create_Instance_Data_ShaderVariables(pd3dDevice, pd3dCommandList);
	}

	// 가시성 검사 후, 보이는 인스턴스만 업데이트
	for (auto& obj_ptr : fixed_obj_list)
	{
		if (!obj_ptr->Get_Active())
			continue;

		XMFLOAT4X4 world_matrix = obj_ptr->m_xmf4x4World;
		XMStoreFloat4x4(&world_matrix, XMMatrixTranspose(XMLoadFloat4x4(&world_matrix)));

		Mapped_Instance_info[visible_count++] = { world_matrix };
	}

	rendering_num = visible_count; 
}

void Fixed_Object_Info::Release_Instance_Data_ShaderVariables()
{
	if (Instance_info) Instance_info->Unmap(0, NULL);
	if (Instance_info) Instance_info->Release();
}


std::shared_ptr<CShader> Object_Manager::instance_shader = NULL;
std::shared_ptr<CShader> Object_Manager::trail_shader = NULL;

bool Object_Manager::do_instance_update = false;


Object_Manager::Object_Manager()
{
}

Object_Manager::~Object_Manager()
{

}

void Object_Manager::Add_Object(std::shared_ptr<CGameObject> obj_ptr, Object_Type type)
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
	case Object_Type::player:
	{
		if (obj_ptr->m_pSkinnedAnimationController != NULL)
			player_list.push_back(std::dynamic_pointer_cast<CTerrainPlayer>(obj_ptr));
	}
	break;
	
	case Object_Type::trail:
	{
		if (obj_ptr != NULL)
			trail_obj_list.push_back(obj_ptr);
	}
	break;

	case Object_Type::plane:
	{
		if (obj_ptr != NULL)
			plane_obj_list.push_back(obj_ptr);
	}
	break;

	case Object_Type::etc:
		break;
	default:
		break;
	}
}

void Object_Manager::Add_Object_To_Unordered_Map(std::shared_ptr<CGameObject> obj_ptr, std::unordered_map<std::string, Fixed_Object_Info>& container)
{
	string name = obj_ptr->Get_Mesh_Name();

	//if (name == "SM_Env_Beach_02" ||
	//	name == "SM_Env_Beach_03" ||
	//	name == "SM_Env_Beach_04" ||
	//	name == "SM_Env_Beach_06" ||
	//	name == "SM_Env_Flat_Sand_02")
	//	name = "None";

	if (name != "None") 
	{
		container[name].fixed_obj_list.push_back(obj_ptr);

		if (unique_mesh_names.insert(name).second)
		{
			container[name].obj_mesh = std::shared_ptr<CMesh>(obj_ptr->m_pMesh);

			obj_ptr->m_pMesh = nullptr;
		}

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
			if (obj_ptr->Get_Active()) {
				if (obj_ptr->Object_type != OBJECT_TPYE_MAIN_PLAYER)
				obj_ptr->Animate(fTimeElapsed);
			}
	}
	break;

	case Object_Type::non_skinned:
	{
		for ( std::shared_ptr<CGameObject>& obj_ptr : non_skinned_object_list)
			if (obj_ptr->Get_Active())
			{
				obj_ptr->Animate(fTimeElapsed);
				/*if (obj_ptr->Object_type == 10) {
					obj_ptr->m_xmf4x4Parent = obj_ptr->m_xmf4x4World;
					obj_ptr->MoveForward(fTimeElapsed);
				}*/
				obj_ptr->UpdateTransform(NULL);
			}
	}
	break;
	case Object_Type::player:
	{
		for (std::shared_ptr<CGameObject>& obj_ptr : player_list)
			if (obj_ptr->Get_Active()) {
				obj_ptr->Animate(fTimeElapsed);
				/*std::wostringstream oss;
				oss << obj_ptr->GetSkinnedAnimationController()->m_pAnimationTracks[0].m_fSpeed << '\n';
				OutputDebugStringW(oss.str().c_str());*/
			}
	}
	break;

	case Object_Type::trail:
	{
		for (std::shared_ptr<CGameObject>& obj_ptr : trail_obj_list)
		{
			if (obj_ptr->Get_Active())
				obj_ptr->Animate(fTimeElapsed);
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
	Animate_Objects(Object_Type::player, fTimeElapsed);
	Animate_Objects(Object_Type::trail, fTimeElapsed);

}

void Object_Manager::Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (do_instance_update == false)
		return;
	else
		do_instance_update = false; // 다음 Update 호출 전까지는 인스턴스 정보 유지하기

	for (auto& pair : fixed_obj_info_map)
	{
		Fixed_Object_Info& info = pair.second;
		if (info.Instance_info == NULL)
		{
			info.Create_Instance_Data_ShaderVariables(pd3dDevice, pd3dCommandList);
			info.Update_Instance_Data(pd3dDevice, pd3dCommandList);
		}
		else
			info.Update_Instance_Data(pd3dDevice, pd3dCommandList);
	}

}

void Object_Manager::Check_Culling(CCamera* pCamera, Object_Type obj_type)
{
	switch (obj_type)
	{
	case Object_Type::skinned:
	{
		bool Is_Visible = false;
		for (std::shared_ptr<CGameObject>& obj_ptr : skinned_object_list)
		{
			Is_Visible = obj_ptr->IsVisible(pCamera);
			obj_ptr->Set_Active(Is_Visible);
			
		}
	} break;

	case Object_Type::non_skinned:
	{
		bool Is_Visible = false;
		for (std::shared_ptr<CGameObject>& obj_ptr : non_skinned_object_list)
		{
			Is_Visible = obj_ptr->IsVisible(pCamera);
			if (obj_ptr->Object_type != 10)
				obj_ptr->Set_Active(Is_Visible);

		}
	} break;

	case Object_Type::fixed:
	{
		if(terrain_ptr)
			Synchronize_Active_Objects_and_Tile();
	} break;

	case Object_Type::player:
	{
		bool Is_Visible = false;
		for (std::shared_ptr<CGameObject>& obj_ptr : player_list)
		{
			Is_Visible = obj_ptr->IsVisible(pCamera);
			obj_ptr->Set_Active(Is_Visible);

		}
	} break;

	case Object_Type::etc:
		break;
	}



}

void Object_Manager::Check_Culling_All(CCamera* pCamera)
{
	/// 타일맵 컬링하기
	//if (terrain_ptr != NULL)	
	//	terrain_ptr->Check_Culling(pCamera);

	//Check_Culling(pCamera, Object_Type::skinned);
	//Check_Culling(pCamera, Object_Type::non_skinned);
	//Check_Culling(pCamera, Object_Type::fixed);
}

void Object_Manager::Classify_Objects_By_Tile()
{	
	// 객체들의 위치에 따라 타일로 분류하는 함수
	
	//=============================== 
	for (auto& [tile_num, obj_list] : obj_list_in_tile)
		obj_list.clear();
	obj_list_in_tile.clear();
	//===============================

	CHeightMapTerrain* last_checked_tile = NULL;
	int Tile_Num = 0;

	for (auto& [meshName, instance_info] : fixed_obj_info_map)
	{
		for (std::shared_ptr<CGameObject> obj_ptr : instance_info.fixed_obj_list)
		{
			XMFLOAT3 obj_pos = obj_ptr->GetPosition();
			Tile_Num = terrain_ptr->Get_Tile(obj_pos.x, obj_pos.z, last_checked_tile);
			obj_list_in_tile[Tile_Num].push_back(obj_ptr);
		}
	}

	Reserve_Update();
}

void Object_Manager::Synchronize_Active_Objects_and_Tile()
{
	// 활성화된 타일 번호 리스트 생성
	std::vector<int> active_tile_num_list;
	terrain_ptr->Get_Active_TileNum_List(active_tile_num_list);
	std::unordered_set<int> active_tile_set(active_tile_num_list.begin(), active_tile_num_list.end());

	// 객체를 갖고 있는 타일 중에서,
	// 활성화된 타일이 갖는 객체들은 활성화
	// 비활성화된 타일의 객체들은 비활성화
	for (auto& [tile_num, obj_list] : obj_list_in_tile)
	{
		bool tile_active = true;
		if (active_tile_set.find(tile_num) != active_tile_set.end()) // 활성화 타일 리스트에 포함된 타일인 경우
			tile_active = true;
		else
			tile_active = false;

		for (std::shared_ptr<CGameObject> obj_ptr : obj_list)
			obj_ptr->Set_Active(tile_active);		
	}

	Reserve_Update();
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
		if (terrain_ptr)
		{
			terrain_ptr->Render(pd3dCommandList, pCamera); // 렌더링과 + 활성화 타일 선별
//			Synchronize_Active_Objects_and_Tile();
		}

		if (instance_shader)
			instance_shader->Setting_Render(pd3dCommandList, 0);

		for (auto& [meshName, instance_info] : fixed_obj_info_map)
		{
			for(std::shared_ptr<CGameObject> obj_ptr : instance_info.fixed_obj_list)
			{
				for(std::shared_ptr<CMaterial> obj_material : obj_ptr->Material_list)
				{
					if (obj_material)
					{
						// 재료(Material) 셰이더 변수 업데이트
						// 현재 의미 없음, 결국 메테리얼 하나의 정보를 기반으로 인스턴싱
						// -> 한번만 동작해야 함 
						// -> 하나의 머테리얼을 모두에게 적용하게 됨
						// -> 각각 다른머테리얼을 하려면, 인스턴싱을 하면 안됨 or 인스턴싱 넘버 기반으로 셰이더에서 처리하기
						// 아니면 인스턴싱 정보에 재질 ID 전달 및 ID 기반 조명 렌더링
						obj_material->UpdateShaderVariable(pd3dCommandList);
							
						// 메쉬 렌더링
						if (instance_info.obj_mesh)
							instance_info.obj_mesh->Instancing_Render(pd3dCommandList, instance_info.m_d3dInstancingBufferView, instance_info.rendering_num);
					}
					break;
				}
				break;
			}
		}
	}
	break;

	case Object_Type::player:
	{
		for (std::shared_ptr<CGameObject>& obj_ptr : player_list)
		{
			if (obj_ptr->Get_Active())
			{
				obj_ptr->UpdateTransform(NULL);
				obj_ptr->Render(pd3dCommandList, pCamera);
			}
		}
	}
	break;

	case Object_Type::trail:
	{
		if (!trail_obj_list.size())
			break;

		if (!trail_shader)
			break;
		trail_shader->Setting_Render(pd3dCommandList, 0);
		for (std::shared_ptr<CGameObject>& obj_ptr : trail_obj_list)
		{
			if (obj_ptr->Get_Active())
				obj_ptr->Render(pd3dCommandList, pCamera);
		}

	}
	break;

	case Object_Type::plane:
	{
		if (!plane_obj_list.size())
			break;

		for (std::shared_ptr<CGameObject>& obj_ptr : plane_obj_list)
		{
			obj_ptr->Render(pd3dCommandList, pCamera);
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
void Object_Manager::Render_Terrain(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (terrain_ptr) 
		terrain_ptr->Render(pd3dCommandList, pCamera);
}
void Object_Manager::Render_Objects_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	Render_Objects(Object_Type::skinned, pd3dCommandList, pCamera);
	Render_Objects(Object_Type::non_skinned, pd3dCommandList, pCamera);
	Render_Objects(Object_Type::player, pd3dCommandList, pCamera);
	Render_Objects(Object_Type::fixed, pd3dCommandList, pCamera);
}

void Object_Manager::Render_Transparent_Objects_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	Render_Objects(Object_Type::trail, pd3dCommandList, pCamera);
}


void Object_Manager::Post_Update(Object_Type type)
{
	switch (type)
	{
	case Object_Type::skinned:
	{
		for (std::shared_ptr<CGameObject>& obj_ptr : skinned_object_list)
			if (obj_ptr->Get_Active())
				obj_ptr->Record_Last_Pos();
	}
	break;

	case Object_Type::non_skinned:
	{
		for (std::shared_ptr<CGameObject>& obj_ptr : non_skinned_object_list)
			if (obj_ptr->Get_Active())
				obj_ptr->Record_Last_Pos();
	}
	break;

	case Object_Type::fixed:
	case Object_Type::etc:
	default:
	{
		DebugOutput("Object_Manager::Last_Update() - Using_Wrong_Type");
		::PostQuitMessage(0);
	}
	break;

	}

}

void Object_Manager::Post_Update_All()
{
	Post_Update(Object_Type::skinned);
	Post_Update(Object_Type::non_skinned);
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

	case Object_Type::player:
		return &player_list;
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
		//return &fixed_obj_info_map;
		break;
	case Object_Type::non_skinned:
	case Object_Type::etc:
	default:
		DebugOutput("Object_Manager::Get_Object_List_Map() - Using_Wrong_Type");
		::PostQuitMessage(0);
		break;
	}
}

std::vector<std::shared_ptr<CGameObject>> Object_Manager::Gather_All_Fixed_Objects()
{
	std::vector<std::shared_ptr<CGameObject>> result;
	for (auto& [name, info] : fixed_obj_info_map)
	{
		result.insert(result.end(), info.fixed_obj_list.begin(), info.fixed_obj_list.end());
	}
	return result;
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

			if (info.obj_mesh)
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
	//Clear_Object_List(Object_Type::fixed);

}

std::vector<GPU_OBB> Object_Manager::Extract_Fixed_OBBs()
{
	std::vector<GPU_OBB> obbList;

	for (const auto& [meshName, fixedInfo] : fixed_obj_info_map)
	{
		if (!fixedInfo.obj_mesh || !fixedInfo.obj_mesh->Get_BoundingBox()) continue;

		const BoundingOrientedBox& localOBB = *fixedInfo.obj_mesh->Get_BoundingBox();

		for (const auto& obj : fixedInfo.fixed_obj_list)
		{
			if (!obj || !obj->Get_Active()) continue;

			XMMATRIX objMat = XMLoadFloat4x4(&obj->m_xmf4x4World);

			// 분해하여 객체 회전과 스케일 추출
			XMVECTOR scale, rotation, translation;
			XMMatrixDecompose(&scale, &rotation, &translation, objMat);

			// 1. Center 변환 (localOBB.Center → 월드)
			XMVECTOR localCenter = XMLoadFloat3(&localOBB.Center);
			XMVECTOR worldCenter = XMVector3Transform(localCenter, objMat);

			// 2. Orientation 변환 (로컬 OBB 회전 * 객체 회전)
			XMVECTOR obbRot = XMLoadFloat4(&localOBB.Orientation);
			XMVECTOR finalRot = XMQuaternionMultiply(obbRot, rotation);
			finalRot = XMQuaternionNormalize(finalRot);

			// 3. Extents 변환 (로컬 Extents * 객체 스케일)
			XMFLOAT3 scaleVec;
			XMStoreFloat3(&scaleVec, scale);
			XMFLOAT3 worldExtents = {
				localOBB.Extents.x * scaleVec.x,
				localOBB.Extents.y * scaleVec.y,
				localOBB.Extents.z * scaleVec.z
			};

			// GPU OBB 구성
			GPU_OBB obb{};
			XMStoreFloat3(&obb.Center, worldCenter);
			XMStoreFloat4(&obb.Rotation, finalRot);
			obb.Extents = worldExtents;
			obb.Type = static_cast<UINT>(obj->Object_type);
			obb.Active = obj->Get_Active() ? 1 : 0;

			obbList.push_back(obb);
		}
	}

	return obbList;
}

//==================================================
void Object_Manager::Create_OBB_Drawer(Object_Type type, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	if (obb_drawer_map.find(type) != obb_drawer_map.end())
		return;

	auto drawer = std::make_shared<OBB_Drawer>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	drawer->Create_OBB_Data_ShaderVariables(pd3dDevice, pd3dCommandList);
	obb_drawer_map[type] = drawer;
}

void Object_Manager::Create_OBB_Drawers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	std::vector<Object_Type> types = { Object_Type::fixed, Object_Type::skinned, Object_Type::non_skinned };
	for (Object_Type type : types)
		Create_OBB_Drawer(type, pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
}

void Object_Manager::Update_OBB_Drawer(Object_Type type, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	auto it = obb_drawer_map.find(type);
	if (it != obb_drawer_map.end())
	{
		it->second->Update_OBB_Data(pd3dDevice, pd3dCommandList, type, this);
	}
}

void Object_Manager::Update_OBB_Drawers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (auto& [type, drawer] : obb_drawer_map)
		drawer->Update_OBB_Data(pd3dDevice, pd3dCommandList, type, this);
}

void Object_Manager::Render_OBB_Drawers(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* camera)
{
	for (auto& [type, drawer] : obb_drawer_map)
		drawer->Render(pd3dCommandList, camera);
}