#pragma once
#include "stdafx.h"
#include "Object.h"
#include "Player.h"
#include "Shader.h"

#define DEFAULT_INSTANCE_NUM 1
#define MAX_INSTANCING_NUM 10000  // 최대 인스턴스 개수 제한 

struct BoundingBox_Instance_Info;
struct Fixed_Object_Info;
struct Instance_Info;
struct GPU_OBB;

struct Instance_Info
{
	XMFLOAT4X4 world_4x4transform;
};

struct alignas(16) BoundingBox_Instance_Info
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

enum class Object_Type
{
	skinned,
	non_skinned,
	fixed,
	player,
	trail,
	etc
};


class BoundingBox_Shader : public CShader
{
public:
	BoundingBox_Shader();
	virtual ~BoundingBox_Shader();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
};


class Object_Manager;

class OBB_Drawer 
{
public:
	OBB_Drawer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
	~OBB_Drawer();

	void Create_OBB_Data_ShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void Release_OBB_Data_ShaderVariables();

	void Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Object_Type type, Object_Manager* obj_mgr);
	static bool Compute_Fixed_OBB_WorldMatrix(const BoundingOrientedBox& localOBB, const XMFLOAT4X4& objectWorld, XMFLOAT4X4& out_world);

	void Render(ID3D12GraphicsCommandList* cmdList, CCamera* camera);

private:
	void Update_From_Vector(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<CGameObject>>& obj_list);
	void Update_From_Map(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::unordered_map<std::string, Fixed_Object_Info>& obj_map);
	
	//bool Get_OBB_WorldMatrix(CGameObject* g_obj, XMFLOAT4X4* world_matrix);
	std::optional<BoundingOrientedBox> OBB_Drawer::Get_OBB_WorldMatrix(CGameObject* g_obj, XMFLOAT4X4* world_matrix);

	void FindOBBObjects(std::shared_ptr<CGameObject> obj, std::vector<std::shared_ptr<CGameObject>>& obb_list, std::unordered_set<CGameObject*>& visited);

private:
	static CubeMesh* obb_Mesh;
	static BoundingBox_Shader* obb_shader;

	ID3D12Resource* Instance_info = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView{};
	BoundingBox_Instance_Info* Mapped_Instance_info = nullptr;

	int obb_instance_buffer_max_num = 64;
	int rendering_num = 0;
};


class Object_Manager
{
private:
	// Terrain and tile management
	std::shared_ptr<CHeightMapTerrain> terrain_ptr;
	std::unordered_map<int, std::vector<std::shared_ptr<CGameObject>>> obj_list_in_tile;
	void Synchronize_Active_Objects_and_Tile();

	// Wave object - unique per scene
	std::shared_ptr<Wave_Object> wave_obj_ptr;
	

	// Dynamic object lists
	std::vector<std::shared_ptr<CGameObject>> skinned_object_list;
	std::vector<std::shared_ptr<CGameObject>> non_skinned_object_list;
	std::vector<std::shared_ptr<CGameObject>> player_list;
	std::vector<std::shared_ptr<CGameObject>> trail_obj_list;


	// Static object map
	std::unordered_map<std::string, Fixed_Object_Info> fixed_obj_info_map;
	std::unordered_set<std::string> unique_mesh_names;
	void Add_Object_To_Unordered_Map(std::shared_ptr<CGameObject> obj_ptr, std::unordered_map<std::string, Fixed_Object_Info>& container);

	// OBB drawer map per object type
	std::unordered_map<Object_Type, std::shared_ptr<OBB_Drawer>> obb_drawer_map;

public:
	// Constructor / Destructor
	Object_Manager();
	~Object_Manager();

	// Object management
	void Add_Object(std::shared_ptr<CGameObject> obj_ptr, Object_Type type);
	void Delete_Object(std::shared_ptr<CGameObject> obj_ptr);
	void Clear_Object_List(Object_Type type);
	void Clear_Object_List_All();

	// Terrain setter
	void Set_Terrain_Object(std::shared_ptr<CHeightMapTerrain> obj_ptr) { terrain_ptr = obj_ptr; }

	// Wave setter
	void Set_Wave_Object(std::shared_ptr<Wave_Object> obj_ptr) { wave_obj_ptr = obj_ptr; }
	std::shared_ptr<Wave_Object>  Get_Wave_Object() { return wave_obj_ptr; }
	void Render_Wave(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) { if (wave_obj_ptr != NULL) wave_obj_ptr->Render(pd3dCommandList, pCamera); }


	// Accessors for object lists
	std::vector<std::shared_ptr<CGameObject>>* Get_Object_List(Object_Type type);
	std::unordered_map<std::string, Fixed_Object_Info>* Get_Object_List_Map(Object_Type type);
	std::vector<std::shared_ptr<CGameObject>> Gather_All_Fixed_Objects();

	// Animation and logic update
	void Animate_Objects_All(float fTimeElapsed);
	void Animate_Objects(Object_Type type, float fTimeElapsed);
	void Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Post_Update(Object_Type type);
	void Post_Update_All();

	// Visibility / culling
	void Check_Culling(CCamera* pCamera, Object_Type obj_type);
	void Check_Culling_All(CCamera* pCamera);

	// Rendering
	void Render_Terrain(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);
	void Render_Objects(Object_Type type, ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);
	void Render_Objects_All(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);
	void Render_Transparent_Objects_All(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);

	// Instancing update flag
	static std::shared_ptr<CShader> instance_shader;
	static bool do_instance_update;
	static void Reserve_Update() { do_instance_update = true; }

	static std::shared_ptr<CShader> trail_shader;

	//============[OBB]===================


	// OBB drawer management
	void Create_OBB_Drawer(Object_Type type, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
	void Create_OBB_Drawers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
	void Update_OBB_Drawer(Object_Type type, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void Update_OBB_Drawers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void Render_OBB_Drawers(ID3D12GraphicsCommandList* cmdList, CCamera* camera);


	void Classify_Objects_By_Tile();


	std::vector<GPU_OBB> m_OBBDataArray;
	std::vector<GPU_OBB> Extract_Fixed_OBBs();
	void Update_Fixed_OBBs() { m_OBBDataArray = Extract_Fixed_OBBs(); }
	const std::vector<GPU_OBB>& Get_Fixed_OBBs() const { return m_OBBDataArray; }

};
