#include "stdafx.h"
#include "Object_Manager.h"


CMesh* OOBB_Drawer::oobb_Mesh = NULL;
CShader* OOBB_Drawer::oobb_shader = NULL;

void OOBB_Drawer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(OOBB_INFO) + 255) & ~255); //256�� ���
	m_pd3dcbOOBBInfo = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbOOBBInfo->Map(0, NULL, (void**)&m_pcbMappedOOBBInfo);
}

bool OOBB_Drawer::UpdateOOBB_Data(ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* g_obj, XMFLOAT4 line_color)
{
	// ���� ������Ʈ���� BoundingOrientedBox�� ��������, NULL üũ
	BoundingOrientedBox* pBoundingBox = g_obj->Get_Collider();
	if (pBoundingBox == NULL)
		return false;

	// BoundingOrientedBox�� Extents ���� �����ɴϴ� (�ʿ��� ���� ���)
	XMVECTOR extents = XMLoadFloat3(&pBoundingBox->Extents);

	// ���� ������Ʈ�� ��ġ�� ȸ������ �ε�
	XMVECTOR center = XMLoadFloat3(&pBoundingBox->Center);
	XMMATRIX worldMatrix = XMLoadFloat4x4(&g_obj->m_xmf4x4World);

	// ���� ��Ŀ��� ȸ�� ��� ����
	XMVECTOR scale, rotation, translation;
	XMMatrixDecompose(&scale, &rotation, &translation, worldMatrix);

	// ȸ�� ����� �������� BoundingOrientedBox�� ������ ��ȯ ����� ����
	XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotation);
	XMMATRIX scaleMatrix = XMMatrixScaling(
		2.0f * XMVectorGetX(extents),
		2.0f * XMVectorGetY(extents),
		2.0f * XMVectorGetZ(extents));

	XMMATRIX translationMatrix = XMMatrixTranslationFromVector(center);

	// ���� ���� ��� ��� (������, ȸ��, �̵� ������ ����)
	XMMATRIX finalWorldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	// ����� XMFLOAT4X4 �������� ���� (HLSL���� ����ϴ� ����� �� �켱(row-major) �����̹Ƿ� ��ġ �ʿ�)
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(finalWorldMatrix));

	// ��� ���ۿ� ������ �����͸� ����
	m_pcbMappedOOBBInfo->line_color = line_color;  // OBB�� ����
	m_pcbMappedOOBBInfo->world_matrix = xmf4x4World;  // ���� ���� ���

	// GPU ��� ���ۿ� ����
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbOOBBInfo->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(PARAMETER_OOBB_CUBE_CBV, d3dGpuVirtualAddress);

	return true;
}
bool OOBB_Drawer::UpdateOOBB_Data(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* matrix, BoundingOrientedBox* pBoundingBox, XMFLOAT4 line_color)
{
	// ���� ������Ʈ���� BoundingOrientedBox�� ��������, NULL üũ
	if (pBoundingBox == NULL)
		return false;

	// BoundingOrientedBox�� Extents ���� �����ɴϴ� (�ʿ��� ���� ���)
	XMVECTOR extents = XMLoadFloat3(&pBoundingBox->Extents);

	// ���� ������Ʈ�� ��ġ�� ȸ������ �ε�
	XMVECTOR center = XMLoadFloat3(&pBoundingBox->Center);
	XMMATRIX worldMatrix = XMLoadFloat4x4(matrix);

	// ���� ��Ŀ��� ȸ�� ��� ����
	XMVECTOR scale, rotation, translation;
	XMMatrixDecompose(&scale, &rotation, &translation, worldMatrix);

	// ȸ�� ����� �������� BoundingOrientedBox�� ������ ��ȯ ����� ����
	XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotation);
	XMMATRIX scaleMatrix = XMMatrixScaling(
		2.0f * XMVectorGetX(extents),
		2.0f * XMVectorGetY(extents),
		2.0f * XMVectorGetZ(extents));

	XMMATRIX translationMatrix = XMMatrixTranslationFromVector(center);

	// ���� ���� ��� ��� (������, ȸ��, �̵� ������ ����)
	XMMATRIX finalWorldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	// ����� XMFLOAT4X4 �������� ���� (HLSL���� ����ϴ� ����� �� �켱(row-major) �����̹Ƿ� ��ġ �ʿ�)
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(finalWorldMatrix));

	// ��� ���ۿ� ������ �����͸� ����
	m_pcbMappedOOBBInfo->line_color = line_color;  // OBB�� ����
	m_pcbMappedOOBBInfo->world_matrix = xmf4x4World;  // ���� ���� ���

	// GPU ��� ���ۿ� ����
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbOOBBInfo->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(PARAMETER_OOBB_CUBE_CBV, d3dGpuVirtualAddress);

	return true;
}

void OOBB_Drawer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (oobb_Mesh)
		oobb_Mesh->Render(pd3dCommandList, 0);

};



//==================================================


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
