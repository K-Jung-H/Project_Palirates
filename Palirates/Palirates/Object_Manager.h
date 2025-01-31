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
	ID3D12Resource* m_pd3dcbOOBBInfo = NULL; // OOBB_INFO 에 대한 배열로 처리해서, 인스턴싱하는 걸로 하기
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

	// 자료 구조를 unordered_map이나 unordered_set 로 관리하면, 애니메이션,렌더링 처리 순서를 관리할 수 있을거 같은데..

	std::vector<std::shared_ptr<CGameObject>> skinned_object_list;
	std::vector<std::shared_ptr<CGameObject>> non_skinned_object_list;

	// 다른 곳에서 실수로 shared_ptr<CGameObject>를 delete 연산 한 경우 -> 오류 발생
	// 이를 방지 하고 싶다면 shared_ptr에 대응되는 weak_ptr를 저장하여, 생명 주기 검사 필요
	// delete 안쓰면 문제 없음
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

