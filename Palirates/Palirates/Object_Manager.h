#pragma once
#include "stdafx.h"
#include "Object.h"
#include "Player.h"
#include "Shader.h"

#define DEFAULT_INSTANCE_NUM 1
#define MAX_INSTANCING_NUM 10000

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

	int instance_buffer_max_num = DEFAULT_INSTANCE_NUM;

	ID3D12Resource* Instance_info = NULL;
	Instance_Info* Mapped_Instance_info = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;
	int rendering_num = 0;


	ID3D12Resource* Shadow_Instance_info = nullptr;
	Instance_Info* Mapped_Shadow_Instance_info = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_d3dShadowInstancingBufferView;
	int shadow_instance_num = 0;

	void Create_Instance_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void Update_Instance_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
//	void Update_Instance_Data_AllObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList); // For Shadow-Map Render
	void Release_Instance_Data_ShaderVariables();

	void Create_Shadow_Instance_Buffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void Update_Shadow_Instance_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void Release_Shadow_Instance_Buffer();


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



class OBB_Renderer
{
private:
	static shared_ptr<CubeMesh> obb_Mesh;
	static shared_ptr<BoundingBox_Shader> obb_shader;

private:
	ID3D12Resource* Instance_info = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView{};
	BoundingBox_Instance_Info* Mapped_Instance_info = nullptr;

	int obb_instance_buffer_max_num = 64;
	int rendering_num = 0;

public:
	OBB_Renderer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
	~OBB_Renderer();

	void Create_OBB_Data_ShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void Release_OBB_Data_ShaderVariables();

	void Update_Fixed(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::unordered_map<std::string, Fixed_Object_Info>& fixed_obj_info_map);
	void Update_Dynamic(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,  std::vector<std::shared_ptr<CGameObject>>& obj_list);

	void Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<std::shared_ptr<CGameObject>>& obj_list);
	void Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::unordered_map<std::string, Fixed_Object_Info>& fixed_obj_info_map);
	
	void Render(ID3D12GraphicsCommandList* cmdList, CCamera* camera);
};

namespace DirectX
{
	inline bool operator==(const XMINT3& lhs, const XMINT3& rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}
}

struct XMINT3Hasher
{
	std::size_t operator()(const XMINT3& k) const noexcept
	{
		return std::hash<int>()(k.x) ^ std::hash<int>()(k.y << 1) ^ std::hash<int>()(k.z << 2);
	}
};

struct OBB_Info
{
	std::shared_ptr<CGameObject> object;
	std::shared_ptr<CMesh> mesh;
	BoundingOrientedBox obb;
	EObjectType type = EObjectType::None;
};

class OBBCollision_Manager
{
private:
	std::vector<OBB_Info> obb_objects;
	std::unordered_map<XMINT3, std::vector<UINT>, XMINT3Hasher> uniform_cell_map;
	float grid_cell_size = 100.0f;

private:
	XMINT3 Get_CellIndexFromPosition(const XMFLOAT3& pos) const;
	void Compute_CellBounds_From_OBB(const BoundingOrientedBox& obb, XMINT3& out_min_cell, XMINT3& out_max_cell) const;
	void Register_OBB_To_Cells(const BoundingOrientedBox& obb, UINT index);

public:
	OBBCollision_Manager() = default;
	~OBBCollision_Manager() = default;

	void Clear();

	void Update_OBB_Data(const std::unordered_map<std::string, Fixed_Object_Info>& fixed_obj_info_map);
	void Update_OBB_Data(const std::vector<shared_ptr<CGameObject>>& obj_list);

	void Build_UniformGrid(float cellSize); // Rebuilds the cell map from existing OBBs using the given cell size.
	void Add_OBB(const OBB_Info& info);

	std::vector<OBB_Info> Get_Nearby_OBBs(const BoundingOrientedBox& obb) const;
	std::vector<OBB_Info> Check_OBB_Collisions_By_Cell(const BoundingOrientedBox& obb) const; // check collision by cell

public:
	std::vector<OBB_Info> Get_Visible_OBBs_From_CameraFrustum(const BoundingFrustum& frustum) const;

};

class OBB_Manager 
{
private:
	unique_ptr<OBB_Renderer> obb_renderer;
	unique_ptr<OBBCollision_Manager> collision_manager;

public:
	OBB_Renderer* Get_OBB_Renderer() const { return obb_renderer.get(); }
	OBBCollision_Manager* Get_Collision_Manager() const { return collision_manager.get(); }


public:
	OBB_Manager(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
	~OBB_Manager();


	void Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<shared_ptr<CGameObject>>& obj_list);
	void Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::unordered_map<std::string, Fixed_Object_Info>& fixed_obj_info_map);

	void Render_OBB(ID3D12GraphicsCommandList* cmdList, CCamera* camera);


public:
	std::vector<OBB_Info> OBB_Manager::Check_OBB_Collision(const BoundingOrientedBox& obb) const;
	bool Resolve_Collision_Fixed(BoundingOrientedBox& playerOBB, XMVECTOR& inOutMoveDir) const; // If collision occurs -> Dir == OBB Sliding
	bool Calc_Slide_Move(const BoundingOrientedBox& playerOBB, XMVECTOR& inOutMoveDir, float minSpeed, int maxIterations) const; // not use
	XMVECTOR Resolve_Overlap(const BoundingOrientedBox& playerOBB) const; // not use

public:
	static XMMATRIX Build_OBB_WorldMatrix(const BoundingOrientedBox& obb, bool transpose = true);
	static XMMATRIX Build_Weapon_OBB_WorldMatrix(const BoundingOrientedBox& obb, CXMMATRIX customRot, bool transpose = true);

	static bool Get_OBB_WorldMatrix(CGameObject* g_obj, XMFLOAT4X4* world_matrix);
	static bool Compute_Fixed_OBB_WorldMatrix(const BoundingOrientedBox& localOBB, const XMFLOAT4X4& objectWorld, XMFLOAT4X4& out_world);
	static void FindOBBObjects(std::shared_ptr<CGameObject> obj, std::vector<std::shared_ptr<CGameObject>>& obb_list, std::unordered_set<CGameObject*>& visited);

};


class Object_Manager
{
private:
	// Terrain and tile management
	std::shared_ptr<CHeightMapTerrain> terrain_ptr;

	// Wave object - unique per scene
	std::shared_ptr<Wave_Object> wave_obj_ptr;
	

	// Dynamic object lists
	std::vector<std::shared_ptr<CGameObject>> skinned_object_list;
	std::unordered_map<int, size_t> id2idx;
	std::vector<std::shared_ptr<CGameObject>> non_skinned_object_list;
	std::vector<std::shared_ptr<CGameObject>> trail_obj_list;

	// Static object map
	std::unordered_map<std::string, Fixed_Object_Info> fixed_obj_info_map;
	std::unordered_set<std::string> unique_mesh_names;
	void Add_Object_To_Unordered_Map(std::shared_ptr<CGameObject> obj_ptr, std::unordered_map<std::string, Fixed_Object_Info>& container);

	// OBB drawer map per object type
	std::unique_ptr<OBB_Manager> fixed_obb_manager;
	std::unique_ptr<OBB_Manager> dynamic_obb_manager;

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
	std::unordered_map<int, size_t>& Get_Monster_Map() { return id2idx; };
	std::unordered_map<std::string, Fixed_Object_Info>* Get_Object_List_Map(Object_Type type);
	std::vector<std::shared_ptr<CGameObject>> Gather_All_Fixed_Objects();

	// Animation and logic update
	void Animate_Objects_All(float fTimeElapsed);
	void Animate_Objects(Object_Type type, float fTimeElapsed);
	void Update_ShadowMap_Fixed_Instance(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void Post_Update(Object_Type type);
	void Post_Update_All();

	//Player Map
	std::map<int, std::shared_ptr<CPlayer>> player_map;
	std::map<int, std::shared_ptr<CPlayer>> Get_Player_Map() { return player_map; }

	//Sync Server
	void Add_Player(std::shared_ptr<CPlayer> player_ptr);
	void Remove_Player(int player_id);
	bool Sync_Player_Data(int player_id, const ServerSyncData& syncData);


	// Visibility / culling
	void Check_Culling(CCamera* pCamera, Object_Type obj_type);
	void Check_Culling_All(CCamera* pCamera);
	void ReBuild_Fixed_Info(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	// ShadowMapping
	void Render_Terrain_Shadow(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);
	void Render_Objects_Shadow(Object_Type type, ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);
	void Render_Objects_Shadow_All(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);

	// Rendering
	void Render_Terrain(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);
	void Render_Objects(Object_Type type, ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);
	void Render_Objects_All(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);
	void Render_Transparent_Objects_All(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);

	void Render_Depth_and_Outline_ID(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);


	// Instancing update flag
	static std::shared_ptr<CShader> instance_shader;
	static bool do_instance_update;
	static void Reserve_Update() { do_instance_update = true; }

	static std::shared_ptr<CShader> trail_shader;

	//============[OBB]===================


	// OBB drawer management
	void Create_OBB_Manager(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
	void Update_OBB_Data(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Object_Type type);

	void Check_Player_Collision(shared_ptr<CPlayer> player_ptr);
	void Check_Dynamic_OBB_Collision(const shared_ptr<CGameObject>& obj_ptr);
	void Check_Dynamic_OBB_Collision(const shared_ptr<CPlayer>& player_ptr);

	void Check_Fixed_OBB_Collision(const shared_ptr<CGameObject> obj_ptr);

	void Check_Fixed_OBB_Camera_Culling(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* camera);
	void ApplyCulledOBBsToInstanceBuffers(const std::vector<OBB_Info>& culledOBBs);

	void Render_OBB(ID3D12GraphicsCommandList* cmdList, CCamera* camera);



	std::vector<GPU_OBB> m_OBBDataArray;
	std::vector<GPU_OBB> Extract_Fixed_OBBs();
	void Update_Fixed_OBBs() { m_OBBDataArray = Extract_Fixed_OBBs(); }
	const std::vector<GPU_OBB>& Get_Fixed_OBBs() const { return m_OBBDataArray; }

};
