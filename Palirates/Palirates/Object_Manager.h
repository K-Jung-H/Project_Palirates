#pragma once
#include "Object.h"
#include "Shader.h"


#define DEFAULT_INSTANCE_NUM 1
#define MAX_INSTANCING_NUM 10000  // 최대 인스턴스 개수 제한 

struct BoundingBox_Instance_Info
{
	XMFLOAT4X4 world_4x4transform;
	XMFLOAT4 box_color;
	UINT active;
};

class BoundingBox_Shader : public CShader
{
public:
	BoundingBox_Shader();
	virtual ~BoundingBox_Shader();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
};


class OBB_Drawer
{
protected:
	ID3D12Resource* Instance_info = NULL;
	BoundingBox_Instance_Info* Mapped_Instance_info = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;

	int rendering_max_num = DEFAULT_INSTANCE_NUM;

public:
	static CubeMesh* obb_Mesh;
	static BoundingBox_Shader* obb_shader;

	OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	~OBB_Drawer();

	void Create_OBB_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Release_OBB_Data_ShaderVariables();
	
	void Update_OBB_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::vector<std::shared_ptr<CGameObject>>gameobj_container);
	void FindOBBObjects(std::shared_ptr<CGameObject> obj, std::vector<std::shared_ptr<CGameObject>>& obb_obj_ptr_list, std::unordered_set<CGameObject*>& visited);
	bool Get_OBB_WorldMatrix(CGameObject* g_obj, XMFLOAT4X4* world_matrix);
	


	
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

	shared_ptr<OBB_Drawer> bounding_box_drawer;

	// 자료 구조를 unordered_map이나 unordered_set 로 관리하면, 애니메이션,렌더링 처리 순서를 관리할 수 있을거 같은데..

	std::vector<std::shared_ptr<CGameObject>> skinned_object_list;
	std::vector<std::shared_ptr<CGameObject>> non_skinned_object_list;

	// 다른 곳에서 실수로 shared_ptr<CGameObject>를 delete 연산 한 경우 -> 오류 발생
	// 이를 방지 하고 싶다면 shared_ptr에 대응되는 weak_ptr를 저장하여, 생명 주기 검사 필요
	// delete 안쓰면 문제 없음
public:
	Object_Manager();
	~Object_Manager();

	void Add_Object(std::shared_ptr<CGameObject > obj_ptr);
	void Delete_Object(std::shared_ptr<CGameObject > obj_ptr);

	void Animate_Objects_All(float fTimeElapsed);
		void Animate_Objects(Object_Type type, float fTimeElapsed);


	void Render_Objects_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
		void Render_Objects(Object_Type type, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	std::vector<std::shared_ptr<CGameObject>>* Object_Manager::Get_Object_List(Object_Type type);
	
	void Clear_Object_List_All();
	void Clear_Object_List(Object_Type type);

	//========================================================================
	void Create_OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	void Update_OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::vector<std::shared_ptr<CGameObject>>gameobj_container);
	void Render_OBB_Drawer(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

