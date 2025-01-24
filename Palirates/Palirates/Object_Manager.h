#pragma once
#include "Object.h"

enum class Object_Type
{
	skinned,
	non_skinned,
	etc
};

class Object_Manager
{
private:
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

