#pragma once
#include "Object.h"


struct OOBB_INFO
{
	XMFLOAT4X4 world_matrix;
	XMFLOAT4 line_color;
};

class OOBB_Drawer
{
protected:
	ID3D12Resource* m_pd3dcbOOBBInfo = NULL; // OOBB_INFO �� ���� �迭�� ó���ؼ�, �ν��Ͻ��ϴ� �ɷ� �ϱ�
	OOBB_INFO* m_pcbMappedOOBBInfo = NULL;
public:
	static CMesh* oobb_Mesh;
	static CShader* oobb_shader;
	OOBB_Drawer() { }
	~OOBB_Drawer() { oobb_Mesh->Release(); }
	void SetMesh(CMesh* mesh) { oobb_Mesh = mesh; }

	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	bool UpdateOOBB_Data(ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* g_obj, XMFLOAT4 line_color);
	bool UpdateOOBB_Data(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* matrix, BoundingOrientedBox* pBoundingBox, XMFLOAT4 line_color);

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

};

enum class Object_Type
{
	skinned,
	non_skinned,
	etc
};

class Object_Manager
{
private:

	OOBB_Drawer* bounding_box_drawer = NULL;

	// �ڷ� ������ unordered_map�̳� unordered_set �� �����ϸ�, �ִϸ��̼�,������ ó�� ������ ������ �� ������ ������..

	std::vector<std::shared_ptr<CGameObject>> skinned_object_list;
	std::vector<std::shared_ptr<CGameObject>> non_skinned_object_list;

	// �ٸ� ������ �Ǽ��� shared_ptr<CGameObject>�� delete ���� �� ��� -> ���� �߻�
	// �̸� ���� �ϰ� �ʹٸ� shared_ptr�� �����Ǵ� weak_ptr�� �����Ͽ�, ���� �ֱ� �˻� �ʿ�
	// delete �Ⱦ��� ���� ����
public:
	void Add_Object(std::shared_ptr<CGameObject > obj_ptr);
	void Delete_Object(std::shared_ptr<CGameObject > obj_ptr);

	void Animate_Objects_All(float fTimeElapsed);
		void Animate_Objects(Object_Type type, float fTimeElapsed);


	void Render_Objects_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
		void Render_Objects(Object_Type type, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	std::vector<std::shared_ptr<CGameObject>>* Object_Manager::Get_Object_List(Object_Type type);
	
	void Clear_Object_List_All();
	void Clear_Object_List(Object_Type type);
};

