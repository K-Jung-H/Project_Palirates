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

void BoundingBox_Shader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature)
{
	m_ngraphicsPipelineStates = 1;
	m_ppd3dgraphicsPipelineStates = new ID3D12PipelineState * [m_ngraphicsPipelineStates];

	CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature.get(), 0);
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

//=============================================================================

CubeMesh* OBB_Renderer::obb_Mesh = nullptr;
BoundingBox_Shader* OBB_Renderer::obb_shader = nullptr;

OBB_Renderer::OBB_Renderer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature)
{
	if (!obb_Mesh)
		obb_Mesh = new CubeMesh(device, cmdList);

	if (!obb_shader)
	{
		obb_shader = new BoundingBox_Shader();
		obb_shader->CreateShader(device, cmdList, pd3dGraphicsRootSignature);
		obb_shader->CreateShaderVariables(device, cmdList);
	}

	Create_OBB_Data_ShaderVariables(device, cmdList);
}

OBB_Renderer::~OBB_Renderer()
{
	Release_OBB_Data_ShaderVariables();
	if (obb_Mesh) 	obb_Mesh->Release();
	delete obb_shader;
}

void OBB_Renderer::Create_OBB_Data_ShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
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

void OBB_Renderer::Release_OBB_Data_ShaderVariables()
{

	if (Instance_info)
	{
		Instance_info->Unmap(0, nullptr);
		Instance_info->Release();
		Instance_info = nullptr;
	}
}

void OBB_Renderer::Update_Fixed(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::unordered_map<std::string, Fixed_Object_Info>& fixed_obj_info_map)
{
	int total = 0;
	for (const auto& [_, info] : fixed_obj_info_map)
		total += static_cast<int>(info.fixed_obj_list.size());

	if (total > obb_instance_buffer_max_num)
	{
		Release_OBB_Data_ShaderVariables();
		obb_instance_buffer_max_num = std::min(total * 2, MAX_INSTANCING_NUM);
		Create_OBB_Data_ShaderVariables(device, cmdList);
	}

	int visible_count = 0;

	for (const auto& [_, info] : fixed_obj_info_map)
	{
		if (!info.obj_mesh || !info.obj_mesh->Get_BoundingBox())
			continue;

		const BoundingOrientedBox& localOBB = *info.obj_mesh->Get_BoundingBox();

		for (const auto& obj : info.fixed_obj_list)
		{
			if (!obj) continue;

			XMFLOAT4X4 world;
			if (!OBB_Manager::Compute_Fixed_OBB_WorldMatrix(localOBB, obj->m_xmf4x4World, world))
				continue;

			Mapped_Instance_info[visible_count].world_4x4transform = world;
			XMStoreFloat4(&Mapped_Instance_info[visible_count].box_color,
				obj->Get_Active() ? Colors::LimeGreen : Colors::Crimson);
			++visible_count;
		}
	}

	rendering_num = visible_count;
}

void OBB_Renderer::Update_Dynamic(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<std::shared_ptr<CGameObject>>& obj_list)
{
	std::vector<std::shared_ptr<CGameObject>> obb_targets;
	std::unordered_set<CGameObject*> visited;

	for (const auto& obj : obj_list)
		OBB_Manager::FindOBBObjects(obj, obb_targets, visited);

	int count = static_cast<int>(obb_targets.size());
	if (count > obb_instance_buffer_max_num)
	{
		Release_OBB_Data_ShaderVariables();
		obb_instance_buffer_max_num = std::min(count * 2, MAX_INSTANCING_NUM);
		Create_OBB_Data_ShaderVariables(device, cmdList);
	}

	int visible_count = 0;

	for (const auto& obj : obb_targets)
	{
		if (!obj || !obj->Get_Active())
			continue;

		XMFLOAT4X4 world_matrix;
		auto obb = OBB_Manager::Get_OBB_WorldMatrix(obj.get(), &world_matrix);
		if (!obb)
			continue;

		Mapped_Instance_info[visible_count].world_4x4transform = world_matrix;
		XMStoreFloat4(&Mapped_Instance_info[visible_count].box_color,
			obj->Get_Active() ? Colors::LimeGreen : Colors::Crimson);
		++visible_count;
	}

	rendering_num = visible_count;
}


void OBB_Renderer::Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<std::shared_ptr<CGameObject>>& obj_list)
{
	Update_Dynamic(device, cmdList, obj_list);
}

void OBB_Renderer::Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::unordered_map<std::string, Fixed_Object_Info>& fixed_obj_info_map)
{
	Update_Fixed(device, cmdList, fixed_obj_info_map);
}


void OBB_Renderer::Render(ID3D12GraphicsCommandList* cmdList, CCamera* camera)
{
	obb_shader->Setting_Render(cmdList, 0);
	if (obb_Mesh)
		obb_Mesh->Render(cmdList, m_d3dInstancingBufferView, rendering_num);
}

//=============================================================================

void OBBCollision_Manager::Clear()
{
	obb_objects.clear();
	uniform_cell_map.clear();
}

void OBBCollision_Manager::Add_OBB(const OBB_Info& info)
{
	UINT index = static_cast<UINT>(obb_objects.size());
	obb_objects.push_back(info);
	Register_OBB_To_Cells(info.obb, index);
}

void OBBCollision_Manager::Register_OBB_To_Cells(const BoundingOrientedBox& obb, UINT index)
{
	XMINT3 min_cell, max_cell;
	Compute_CellBounds_From_OBB(obb, min_cell, max_cell);

	for (int x = min_cell.x; x <= max_cell.x; ++x)
		for (int y = min_cell.y; y <= max_cell.y; ++y)
			for (int z = min_cell.z; z <= max_cell.z; ++z)
			{
				XMINT3 cell = { x, y, z };
				uniform_cell_map[cell].push_back(index);
			}
}

void OBBCollision_Manager::Update_OBB_Data(const std::unordered_map<std::string, Fixed_Object_Info>& fixed_obj_info_map)
{
	Clear();

	for (const auto& [meshName, info] : fixed_obj_info_map)
	{
		if (meshName.find("Env") != std::string::npos) continue;
		if (!info.obj_mesh || !info.obj_mesh->Get_BoundingBox()) continue;

		const BoundingOrientedBox& localOBB = *info.obj_mesh->Get_BoundingBox();

		for (const auto& obj : info.fixed_obj_list)
		{
			if (!obj || !obj->Get_Active()) continue;

			BoundingOrientedBox worldOBB;
			localOBB.Transform(worldOBB, XMLoadFloat4x4(&obj->m_xmf4x4World));

			OBB_Info obb_info;
			obb_info.object = obj;
			obb_info.mesh = info.obj_mesh;
			obb_info.obb = worldOBB;
			obb_info.type = static_cast<UINT>(obj->Object_type);

			Add_OBB(obb_info);
		}
	}
}

void OBBCollision_Manager::Build_UniformGrid(float cellSize)
{
	grid_cell_size = cellSize;
	uniform_cell_map.clear();

	for (UINT i = 0; i < obb_objects.size(); ++i)
		Register_OBB_To_Cells(obb_objects[i].obb, i);
}

std::vector<OBB_Info> OBBCollision_Manager::Check_OBB_Collisions(const BoundingOrientedBox& obb) const
{
	std::vector<OBB_Info> collided;
	XMINT3 min_cell, max_cell;

	Compute_CellBounds_From_OBB(obb, min_cell, max_cell);
	std::unordered_set<UINT> tested;

	for (int x = min_cell.x; x <= max_cell.x; ++x)
		for (int y = min_cell.y; y <= max_cell.y; ++y)
			for (int z = min_cell.z; z <= max_cell.z; ++z)
			{
				XMINT3 cell = { x, y, z };
				auto it = uniform_cell_map.find(cell);
				if (it == uniform_cell_map.end()) 
					continue;

				for (UINT idx : it->second)
				{
					if (tested.insert(idx).second) 
					{
						const auto& other = obb_objects[idx];
						if (obb.Intersects(other.obb))
						{
							collided.push_back(other); 
#ifdef DEBUG_MESSAGE
							DebugOutput(other.mesh->Get_Name() + " is Collision!\n");
#endif
						}
					}
				}
			}

	return collided;
}

std::vector<OBB_Info> OBBCollision_Manager::Get_Nearby_OBBs(const BoundingOrientedBox& obb) const
{
	std::vector<OBB_Info> result;
	XMINT3 min_cell, max_cell;
	Compute_CellBounds_From_OBB(obb, min_cell, max_cell);
	std::unordered_set<UINT> inserted;

	for (int x = min_cell.x; x <= max_cell.x; ++x)
		for (int y = min_cell.y; y <= max_cell.y; ++y)
			for (int z = min_cell.z; z <= max_cell.z; ++z)
			{
				XMINT3 cell = { x, y, z };
				auto it = uniform_cell_map.find(cell);
				if (it == uniform_cell_map.end()) continue;

				for (UINT idx : it->second)
				{
					if (inserted.insert(idx).second)
					{
						result.push_back(obb_objects[idx]);
					}
				}
			}
	return result;
}

XMINT3 OBBCollision_Manager::Get_CellIndexFromPosition(const XMFLOAT3& pos) const
{
	return {
		static_cast<int>(std::floor(pos.x / grid_cell_size)),
		static_cast<int>(std::floor(pos.y / grid_cell_size)),
		static_cast<int>(std::floor(pos.z / grid_cell_size))
	};
}

void OBBCollision_Manager::Compute_CellBounds_From_OBB(const BoundingOrientedBox& obb, XMINT3& out_min_cell, XMINT3& out_max_cell) const
{
	XMFLOAT3 corners[8];
	obb.GetCorners(corners);

	BoundingBox aabb;
	BoundingBox::CreateFromPoints(aabb, 8, corners, sizeof(XMFLOAT3));

	XMFLOAT3 min = {
		aabb.Center.x - aabb.Extents.x,
		aabb.Center.y - aabb.Extents.y,
		aabb.Center.z - aabb.Extents.z
	};
	XMFLOAT3 max = {
		aabb.Center.x + aabb.Extents.x,
		aabb.Center.y + aabb.Extents.y,
		aabb.Center.z + aabb.Extents.z
	};

	out_min_cell = Get_CellIndexFromPosition(min);
	out_max_cell = Get_CellIndexFromPosition(max);
}

std::vector<OBB_Info> OBBCollision_Manager::Get_Visible_OBBs_From_CameraFrustum(const BoundingFrustum& frustum) const
{
	std::vector<OBB_Info> result;
	std::unordered_set<UINT> visibleIndices;

	for (const auto& [cellIdx, obbIndices] : uniform_cell_map)
	{
		BoundingBox cellAABB;

		float cellSize = grid_cell_size;

		XMFLOAT3 minPos = {
			cellIdx.x * cellSize,
			cellIdx.y * cellSize,
			cellIdx.z * cellSize
		};

		XMFLOAT3 maxPos = {
			minPos.x + cellSize,
			minPos.y + cellSize,
			minPos.z + cellSize
		};

		cellAABB.Center = {
			(minPos.x + maxPos.x) * 0.5f,
			(minPos.y + maxPos.y) * 0.5f,
			(minPos.z + maxPos.z) * 0.5f
		};

		cellAABB.Extents = {
			(maxPos.x - minPos.x) * 0.5f,
			(maxPos.y - minPos.y) * 0.5f,
			(maxPos.z - minPos.z) * 0.5f
		};

		if (!frustum.Intersects(cellAABB))
			continue;

		for (UINT idx : obbIndices)
		{
			if (idx >= obb_objects.size()) continue;

			const auto& obbInfo = obb_objects[idx];
			if (!obbInfo.object || !obbInfo.mesh) continue;

			BoundingOrientedBox worldOBB = obbInfo.obb;

			if (frustum.Intersects(worldOBB))
				visibleIndices.insert(idx);
		}
	}

	result.reserve(visibleIndices.size());
	for (UINT idx : visibleIndices)
		result.push_back(obb_objects[idx]);

	return result;
}


//=============================================================================

OBB_Manager::OBB_Manager(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature)
{
	fixed_obb_renderer = make_unique<OBB_Renderer>(device, cmdList, pd3dGraphicsRootSignature);
	dynamic_obb_renderer = make_unique<OBB_Renderer>(device, cmdList, pd3dGraphicsRootSignature);
	collision_manager = make_unique<OBBCollision_Manager>();
}

OBB_Manager::~OBB_Manager()
{

}


void OBB_Manager::Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<shared_ptr<CGameObject>>& obj_list)
{
	dynamic_obb_renderer->Update_OBB_Data(device, cmdList, obj_list);
}

void OBB_Manager::Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::unordered_map<std::string, Fixed_Object_Info>& fixed_obj_info_map)
{
	fixed_obb_renderer->Update_OBB_Data(device, cmdList, fixed_obj_info_map);
	collision_manager->Update_OBB_Data(fixed_obj_info_map);
}

void OBB_Manager::Render_OBB(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* camera)
{
	fixed_obb_renderer->Render(pd3dCommandList, camera);
	dynamic_obb_renderer->Render(pd3dCommandList, camera);
}


bool OBB_Manager::Check_Collision(const BoundingOrientedBox& obb) const
{
	vector<OBB_Info> collided_obb_list = collision_manager->Check_OBB_Collisions(obb);
	if (collided_obb_list.empty())
		return false;
	else
		return true;
}

bool OBB_Manager::Resolve_Slide_On_Collision(BoundingOrientedBox& playerOBB, XMVECTOR& inOutMoveDir) const
{
	const int maxIterations = 4;
	float originalSpeed = XMVectorGetX(XMVector3Length(inOutMoveDir));
	if (originalSpeed <= 0.0001f) return false;

	XMVECTOR moveDir = XMVector3Normalize(inOutMoveDir);
	XMVECTOR originalCenter = XMLoadFloat3(&playerOBB.Center);
	bool collisionOccurred = false;

	for (int i = 0; i < maxIterations; ++i)
	{
		XMVECTOR testCenter = originalCenter + moveDir * originalSpeed;

		BoundingOrientedBox testOBB = playerOBB;
		XMStoreFloat3(&testOBB.Center, testCenter);

		std::vector<OBB_Info> collisions = collision_manager->Check_OBB_Collisions(testOBB);
		if (collisions.empty()) break;

		XMVECTOR bestNormal = XMVectorZero();
		float maxDot = -1.0f;

		for (size_t j = 0; j < collisions.size(); ++j)
		{
			const BoundingOrientedBox& otherOBB = collisions[j].obb;
			XMVECTOR toOther = XMLoadFloat3(&otherOBB.Center) - testCenter;
			XMVECTOR normal = XMVector3Normalize(toOther);
			float dot = fabsf(XMVectorGetX(XMVector3Dot(normal, moveDir)));

			if (dot > maxDot)
			{
				maxDot = dot;
				bestNormal = normal;
			}
		}

		if (XMVector3Equal(bestNormal, XMVectorZero()))
			break;

		float projection = XMVectorGetX(XMVector3Dot(moveDir, bestNormal));
		XMVECTOR slideDir = moveDir - bestNormal * projection;
		slideDir = XMVectorSet(XMVectorGetX(slideDir), 0.0f, XMVectorGetZ(slideDir), 0.0f);
		moveDir = XMVector3Normalize(slideDir);

		originalSpeed *= 0.9f;
		collisionOccurred = true;
	}

	XMVECTOR finalCenter = originalCenter + moveDir * originalSpeed;
	BoundingOrientedBox finalOBB = playerOBB;
	XMStoreFloat3(&finalOBB.Center, finalCenter);

	std::vector<OBB_Info> stillColliding = collision_manager->Check_OBB_Collisions(finalOBB);
	if (!stillColliding.empty())
	{
		XMVECTOR pushBack = XMVectorZero();

		for (size_t j = 0; j < stillColliding.size(); ++j)
		{
			const BoundingOrientedBox& otherOBB = stillColliding[j].obb;
			XMVECTOR toOutside = finalCenter - XMLoadFloat3(&otherOBB.Center);
			pushBack += XMVector3Normalize(toOutside);
		}

		if (!XMVector3Equal(pushBack, XMVectorZero()))
		{
			float pushStrength = 1.5f + 0.25f * static_cast<float>(stillColliding.size());
			pushBack = XMVector3Normalize(pushBack) * pushStrength;
			inOutMoveDir = pushBack;
			return true;
		}
	}

	inOutMoveDir = moveDir * originalSpeed;
	return collisionOccurred;
}



//struct ObjectOBB
//{
//	std::shared_ptr<CGameObject> obj;
//	BoundingOrientedBox obb;
//	XMFLOAT4X4 worldMatrix;
//};

//void OBB_Manager::Update_From_Vector(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<CGameObject>>& obj_list)
//{

//	for (size_t i = 0; i < col_obb_list.size(); ++i)
//	{
//		for (size_t j = i + 1; j < col_obb_list.size(); ++j)
//		{
//			auto typeA = col_obb_list[i].obj->Object_type;
//			auto typeB = col_obb_list[j].obj->Object_type;
//
//			bool validPair =
//				(typeA == OBJECT_TPYE_MAIN_PLAYER && typeB == OBJECT_TPYE_MONSTER_WEAPON) ||
//				(typeA == OBJECT_TPYE_MONSTER_WEAPON && typeB == OBJECT_TPYE_MAIN_PLAYER) ||
//
//				(typeA == OBJECT_TPYE_MONSTER && typeB == OBJECT_TPYE_PLAYER_WEAPON) ||
//				(typeA == OBJECT_TPYE_PLAYER_WEAPON && typeB == OBJECT_TPYE_MONSTER);
//
//			if (!validPair) continue; 
//
//			const BoundingOrientedBox& a = col_obb_list[i].obb;
//			const BoundingOrientedBox& b = col_obb_list[j].obb;
//
//			if (a.Intersects(b))
//			{
//				/*const char* nameA = col_obb_list[i].obj->Get_Name();
//				const char* nameB = col_obb_list[j].obj->Get_Name();
//
//				char buffer[256];
//				sprintf_s(buffer, "OBB 충돌 감지: [%s] <--> [%s]\n", nameA, nameB);
//				OutputDebugStringA(buffer);*/
//
//				if (typeA == OBJECT_TPYE_MAIN_PLAYER && typeB == OBJECT_TPYE_MONSTER_WEAPON) {
//					std::shared_ptr<CTerrainPlayer> p = std::dynamic_pointer_cast<CTerrainPlayer>(col_obb_list[i].obj);
//					if (p)
//					{
//						if (p->GetStateMachine()->Get_State() != State::Get_Hit_F2)
//							p->GetStateMachine()->changeState(State::Get_Hit_F2, Key_Value::None);
//					}
//					continue;
//				}
//				if (typeA == OBJECT_TPYE_MONSTER_WEAPON && typeB == OBJECT_TPYE_MAIN_PLAYER) {
//					std::shared_ptr<CTerrainPlayer> p = std::dynamic_pointer_cast<CTerrainPlayer>(col_obb_list[j].obj);
//					if (p)
//					{
//						if (p->GetStateMachine()->Get_State() != State::Get_Hit_F2)
//							p->GetStateMachine()->changeState(State::Get_Hit_F2, Key_Value::None);
//					}
//					continue;
//				}
//
//				if (typeA == OBJECT_TPYE_MONSTER && typeB == OBJECT_TPYE_PLAYER_WEAPON) {
//					std::shared_ptr<CMonsterObject> monster = std::dynamic_pointer_cast<CMonsterObject>(col_obb_list[i].obj);
//					if (monster)
//					{
//						if (monster->GetStateMachine()->Get_State() != State::Get_Hit)
//							monster->GetStateMachine()->changeState(State::Get_Hit, Key_Value::None);
//					}
//					continue;
//				}
//				if (typeA == OBJECT_TPYE_PLAYER_WEAPON && typeB == OBJECT_TPYE_MONSTER) {
//					std::shared_ptr<CMonsterObject> monster = std::dynamic_pointer_cast<CMonsterObject>(col_obb_list[j].obj);
//					if (monster)
//					{
//						if (monster->GetStateMachine()->Get_State() != State::Get_Hit)
//							monster->GetStateMachine()->changeState(State::Get_Hit, Key_Value::None);
//					}
//					continue;
//				}
//			}
//		}
//	}
//}



XMMATRIX OBB_Manager::Build_OBB_WorldMatrix(const BoundingOrientedBox& obb, bool transpose)
{
	XMMATRIX scaleMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&obb.Extents) * 2.0f);
	XMMATRIX rotMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
	XMMATRIX transMatrix = XMMatrixTranslationFromVector(XMLoadFloat3(&obb.Center));

	XMMATRIX world = scaleMatrix * rotMatrix * transMatrix;
	return transpose ? XMMatrixTranspose(world) : world;
}

XMMATRIX OBB_Manager::Build_Weapon_OBB_WorldMatrix(const BoundingOrientedBox& obb, CXMMATRIX customRot, bool transpose)
{
	XMMATRIX scaleMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&obb.Extents) * 2.0f);
	XMMATRIX rotMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
	XMMATRIX transMatrix = XMMatrixTranslationFromVector(XMLoadFloat3(&obb.Center));

	XMMATRIX world = scaleMatrix * customRot * rotMatrix * transMatrix;
	return transpose ? XMMatrixTranspose(world) : world;
}

bool OBB_Manager::Get_OBB_WorldMatrix(CGameObject* g_obj, XMFLOAT4X4* world_matrix)
{
	if (!g_obj || !g_obj->Get_Collider())
		return false;

	CSkinnedMesh* skinnedMesh = dynamic_cast<CSkinnedMesh*>(g_obj->m_pMesh);
	if (skinnedMesh)
	{
		BoundingOrientedBox obb = skinnedMesh->Get_WorldOBB();

		XMMATRIX finalMatrix = Build_Weapon_OBB_WorldMatrix(obb, g_obj->customRotation, false);

		XMStoreFloat4x4(world_matrix, XMMatrixTranspose(finalMatrix));
		XMStoreFloat4x4(&g_obj->WeaponMatrix, finalMatrix);
		return true;
	}
	else
	{
		BoundingOrientedBox localOBB = *g_obj->Get_Collider();
		BoundingOrientedBox worldOBB;
		XMMATRIX world = XMLoadFloat4x4(&g_obj->m_xmf4x4World);
		localOBB.Transform(worldOBB, world);

		XMVECTOR scale, rotQuat, trans;
		if (!XMMatrixDecompose(&scale, &rotQuat, &trans, world))
			rotQuat = XMQuaternionIdentity();

		XMStoreFloat4(&worldOBB.Orientation, rotQuat);

		if (worldOBB.Extents.x <= 0.0f || worldOBB.Extents.y <= 0.0f || worldOBB.Extents.z <= 0.0f)
			return false;

		XMMATRIX obbMatrix = Build_OBB_WorldMatrix(worldOBB, true); 
		XMStoreFloat4x4(world_matrix, obbMatrix);
		XMStoreFloat4x4(&g_obj->WeaponMatrix, XMMatrixTranspose(obbMatrix));
		return true;
	}
}

bool OBB_Manager::Compute_Fixed_OBB_WorldMatrix(const BoundingOrientedBox& localOBB, const XMFLOAT4X4& objectWorld, XMFLOAT4X4& out_world)
{
	XMMATRIX localMat = Build_OBB_WorldMatrix(localOBB, false);
	XMMATRIX objMat = XMLoadFloat4x4(&objectWorld);
	XMMATRIX finalMat = localMat * objMat;

	XMStoreFloat4x4(&out_world, XMMatrixTranspose(finalMat));
	return true;
}


void OBB_Manager::FindOBBObjects(std::shared_ptr<CGameObject> obj, std::vector<std::shared_ptr<CGameObject>>& obb_list, std::unordered_set<CGameObject*>& visited)
{
	if (!obj || visited.count(obj.get()) > 0) return;


	visited.insert(obj.get());
	if (obj->Get_Collider()) obb_list.push_back(obj);

	FindOBBObjects(obj->Get_Child(), obb_list, visited);
	FindOBBObjects(obj->Get_Sibling(), obb_list, visited);
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
//	Animate_Objects(Object_Type::skinned, fTimeElapsed);
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
}

void Object_Manager::Check_Culling_All(CCamera* pCamera)
{
}


void Object_Manager::Render_Objects_Shadow(Object_Type type, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
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
				skinned_obj_ptr->Render_Shadow(pd3dCommandList, pCamera);
			}
		}
	}
	break;

	case Object_Type::non_skinned:
	{
		for (std::shared_ptr<CGameObject>& obj_ptr : non_skinned_object_list)
			if (obj_ptr->Get_Active())
				obj_ptr->Render_Shadow(pd3dCommandList, pCamera);
	}
	break;

	case Object_Type::fixed:
	{
		if (instance_shader)
			instance_shader->Setting_Render(pd3dCommandList, 1);

		for (auto& [meshName, instance_info] : fixed_obj_info_map)
		{
			if (instance_info.rendering_num == 0 || !instance_info.obj_mesh)
				continue;
			
			instance_info.obj_mesh->Instancing_Render(pd3dCommandList, instance_info.m_d3dInstancingBufferView, instance_info.rendering_num);

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
				obj_ptr->Render_Shadow(pd3dCommandList, pCamera);
			}
		}
	}
	break;

	case Object_Type::trail:
	case Object_Type::etc:
	default:
	{
		DebugOutput("Object_Manager::Render_Objects() - Using_Wrong_Type");
		::PostQuitMessage(0);
	}
	break;
	}


}

void Object_Manager::Render_Terrain_Shadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (terrain_ptr)
		terrain_ptr->Render_Shadow(pd3dCommandList, pCamera);
}

void Object_Manager::Render_Objects_Shadow_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	Render_Terrain_Shadow(pd3dCommandList, pCamera);
	Render_Objects_Shadow(Object_Type::skinned, pd3dCommandList, pCamera);
	Render_Objects_Shadow(Object_Type::non_skinned, pd3dCommandList, pCamera);
	Render_Objects_Shadow(Object_Type::player, pd3dCommandList, pCamera);
	Render_Objects_Shadow(Object_Type::fixed, pd3dCommandList, pCamera);
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
		if (instance_shader)
			instance_shader->Setting_Render(pd3dCommandList, 0);

		for (auto& [meshName, instance_info] : fixed_obj_info_map)
		{
			if (instance_info.rendering_num == 0 || !instance_info.obj_mesh)
				continue;

			if (!instance_info.fixed_obj_list.empty())
			{
				const auto& first_obj = instance_info.fixed_obj_list.front();
				if (!first_obj->Material_list.empty() && first_obj->Material_list[0])
					first_obj->Material_list[0]->UpdateShaderVariable(pd3dCommandList);
			}

			instance_info.obj_mesh->Instancing_Render(pd3dCommandList, instance_info.m_d3dInstancingBufferView, instance_info.rendering_num);

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
	Render_Terrain(pd3dCommandList, pCamera);
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
	size_t totalSize = 0;
	for (const auto& [name, info] : fixed_obj_info_map)
		totalSize += info.fixed_obj_list.size();

	std::vector<std::shared_ptr<CGameObject>> result;
	result.reserve(totalSize);

	for (const auto& [name, info] : fixed_obj_info_map)
		result.insert(result.end(), info.fixed_obj_list.begin(), info.fixed_obj_list.end());

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

void Object_Manager::Create_OBB_Manager(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature)
{
	if (obb_manager != NULL)
		return;

	obb_manager = std::make_unique<OBB_Manager>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

}


void Object_Manager::Update_OBB_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, Object_Type type)
{
	if (type == Object_Type::fixed)
		obb_manager->Update_OBB_Data(pd3dDevice, pd3dCommandList, fixed_obj_info_map);
	else
	{
		std::vector<std::shared_ptr<CGameObject>> merged;
		merged.reserve(skinned_object_list.size() + non_skinned_object_list.size());

		merged.insert(merged.end(), skinned_object_list.begin(), skinned_object_list.end());
		merged.insert(merged.end(), non_skinned_object_list.begin(), non_skinned_object_list.end());

		obb_manager->Update_OBB_Data(pd3dDevice, pd3dCommandList, merged);
	}


}

void Object_Manager::Check_OBB_Collision()
{
	for (shared_ptr<CGameObject> obj_ptr : skinned_object_list)
	{
		if (obj_ptr->Object_type != OBJECT_TPYE_MAIN_PLAYER)
			continue;

		auto* player = dynamic_cast<CTerrainPlayer*>(obj_ptr.get());
		if (!player) continue;

		const BoundingOrientedBox* localOBB = player->Get_Collider();
		if (!localOBB) continue;

		BoundingOrientedBox worldOBB;
		localOBB->Transform(worldOBB, XMLoadFloat4x4(&player->m_xmf4x4World));

		XMFLOAT3 velocity = player->GetVelocity();
		XMVECTOR moveDir = XMLoadFloat3(&velocity);
		float speed = XMVectorGetX(XMVector3Length(moveDir));

		if (speed <= 0.0001f) continue;

		moveDir = XMVector3Normalize(moveDir);

		if (obb_manager->Resolve_Slide_On_Collision(worldOBB, moveDir))
		{
			XMFLOAT3 slideDir;
			XMStoreFloat3(&slideDir, moveDir);
			player->EnableSliding(slideDir);
		}
	}
}

void Object_Manager::Check_OBB_Culling(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* camera)
{
	if (!obb_manager || !camera) 
		return;

	BoundingFrustum camera_frustum = camera->Get_Frustum(); 

	std::vector<OBB_Info> visibleOBBs = obb_manager->Get_Collision_Manager()->Get_Visible_OBBs_From_CameraFrustum(camera_frustum);

	ApplyCulledOBBsToInstanceBuffers(visibleOBBs);
}

void Object_Manager::ApplyCulledOBBsToInstanceBuffers(const std::vector<OBB_Info>& culledOBBs)
{
	std::unordered_map<std::string, std::vector<std::shared_ptr<CGameObject>>> visibleObjectsByMesh;

	// 1. 메시 이름 기준으로 가시 오브젝트 그룹화
	for (const auto& obb : culledOBBs)
	{
		if (!obb.object || !obb.mesh)
			continue;

		visibleObjectsByMesh[obb.mesh->Get_Name()].push_back(obb.object);
	}

	int totalVisibleInstances = 0;

	// 2. fixed_obj_info_map 순회하면서 인스턴스 버퍼 채우기 or 렌더링 0 처리
	for (auto& [meshName, info] : fixed_obj_info_map)
	{
		auto it = visibleObjectsByMesh.find(meshName);

		if (it == visibleObjectsByMesh.end())
		{
			// 컬링된 메시인 경우 렌더링 제외
			info.rendering_num = 0;
			continue;
		}

		const auto& objList = it->second;
		int visibleCount = static_cast<int>(objList.size());

		for (int i = 0; i < visibleCount; ++i)
		{
			XMFLOAT4X4 worldMatrix = objList[i]->m_xmf4x4World;
			XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
			info.Mapped_Instance_info[i] = { worldMatrix };
		}

		info.rendering_num = visibleCount;
		totalVisibleInstances += visibleCount;
	}

	//DebugOutput("Total Visible Instances: " + std::to_string(totalVisibleInstances) + "\n");
}

void Object_Manager::Render_OBB(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* camera)
{
	obb_manager->Render_OBB(pd3dCommandList, camera);
}
