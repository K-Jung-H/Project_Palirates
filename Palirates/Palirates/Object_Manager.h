#pragma once
#include "Object.h"
#include "Player.h"
#include "Shader.h"


#define DEFAULT_INSTANCE_NUM 1
#define MAX_INSTANCING_NUM 10000  // 최대 인스턴스 개수 제한 

struct BoundingBox_Instance_Info;
struct Fixed_Object_Info;
struct Instance_Info;


struct Instance_Info
{
	XMFLOAT4X4 world_4x4transform;
};

struct BoundingBox_Instance_Info
{
	XMFLOAT4X4 world_4x4transform;
	XMFLOAT4 box_color;
};

struct Fixed_Object_Info
{

	std::vector<std::shared_ptr<CGameObject>> fixed_obj_list;
	std::shared_ptr<CMesh> obj_mesh;

	int rendering_num = 0;
	int instance_buffer_max_num = DEFAULT_INSTANCE_NUM;
	ID3D12Resource* Instance_info = NULL;
	Instance_Info* Mapped_Instance_info = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;

	void Create_Instance_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Update_Instance_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Release_Instance_Data_ShaderVariables();

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

	int obb_instance_buffer_max_num = DEFAULT_INSTANCE_NUM;
	int rendering_num = 1;

public:
	static CubeMesh* obb_Mesh;
	static BoundingBox_Shader* obb_shader;

	OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	~OBB_Drawer();

	void Create_OBB_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Release_OBB_Data_ShaderVariables();
	
	void Update_OBB_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::vector<std::shared_ptr<CGameObject>> gameobj_container);
	void Update_OBB_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::unordered_map<std::string, Fixed_Object_Info> gameobj_container);


	void FindOBBObjects(std::shared_ptr<CGameObject> obj, std::vector<std::shared_ptr<CGameObject>>& obb_obj_ptr_list, std::unordered_set<CGameObject*>& visited);
	bool Get_OBB_WorldMatrix(CGameObject* g_obj, XMFLOAT4X4* world_matrix);
	


	
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

};





enum class Object_Type
{
	skinned,
	non_skinned,
	fixed,
	player,
	trail,
	plane,
	etc
};

class Object_Manager
{
private:
	shared_ptr<OBB_Drawer> bounding_box_drawer;
	shared_ptr<CHeightMapTerrain> terrain_ptr;

	// 움직이는 객체들
	std::vector<std::shared_ptr<CGameObject>> skinned_object_list;
	std::vector<std::shared_ptr<CGameObject>> non_skinned_object_list;

	std::vector<std::shared_ptr<CTerrainPlayer>> player_list;



private:
	// 고정된 사물 객체
	std::unordered_map<std::string, Fixed_Object_Info> fixed_obj_info_map;		// 사물 객체 정보
	std::unordered_set<std::string> unique_mesh_names; 	// 사물 중복 검사

	void Add_Object_To_Unordered_Map(std::shared_ptr<CGameObject> obj_ptr, std::unordered_map<std::string, Fixed_Object_Info>& container);

private:
	std::unordered_map<int, std::vector<std::shared_ptr<CGameObject>>> obj_list_in_tile;
	void Synchronize_Active_Objects_and_Tile();


public:
	//test 
	Wave_Object* wave_obj = NULL;
	std::vector<std::shared_ptr<CGameObject>> plane_obj_list;

	

	std::vector<std::shared_ptr<CGameObject>> trail_obj_list;
	static std::shared_ptr<CShader> trail_shader;


	void Classify_Objects_By_Tile();

	static std::shared_ptr<CShader> instance_shader;
	static bool do_instance_update;
	static	void Reserve_Update() { do_instance_update = true; }

	Object_Manager();
	~Object_Manager();

	void Add_Object(std::shared_ptr<CGameObject> obj_ptr, Object_Type type);
	void Delete_Object(std::shared_ptr<CGameObject > obj_ptr);
	void Set_Terrain_Object(std::shared_ptr<CHeightMapTerrain > obj_ptr) { terrain_ptr = obj_ptr; }


	void Animate_Objects_All(float fTimeElapsed);
	void Animate_Objects(Object_Type type, float fTimeElapsed);

	void Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Check_Culling(CCamera* pCamera, Object_Type obj_type);
	void Check_Culling_All(CCamera* pCamera);

	
	void Render_Terrain(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void Render_Objects(Object_Type type, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void Render_Objects_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void Render_Transparent_Objects_All(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);



	void Post_Update(Object_Type type);
	void Post_Update_All();

	std::vector<std::shared_ptr<CGameObject>>* Get_Object_List(Object_Type type);
	std::unordered_map<std::string, Fixed_Object_Info>* Get_Object_List_Map(Object_Type type);

	std::vector<std::shared_ptr<CTerrainPlayer>>* Get_Player_List();

	void Clear_Object_List_All();
	void Clear_Object_List(Object_Type type);

	//========================================================================
	void Create_OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	void Update_OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::vector<std::shared_ptr<CGameObject>>gameobj_container);
	void Update_OBB_Drawer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::unordered_map<std::string, Fixed_Object_Info > gameobj_container);

	void Render_OBB_Drawer(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

