#pragma once

#include "stdafx.h"
#include "Object.h"
#include "Mesh.h"
#include "Descriptor_Heap.h"

//==============================================================================
#define MAX_PARTICLES				900 * 64

#define FACE_LEFT    0 // -X
#define FACE_RIGHT   1 // +X
#define FACE_BOTTOM  2 // -Y
#define FACE_TOP     3 // +Y
#define FACE_BACK    4 // -Z
#define FACE_FRONT   5 // +Z

enum class Particle_Shader_Type
{
	continuous,
	interval,
	sand,
	spear_skill,
	twin_sword_skill,
	etc
};

enum class Particle_Type
{
	snow = 0,
	splash = 1,
	dragon_breath = 2,
	party = 3,
	sand = 4,
	sand_storm = 5,
	heal = 6,
	orbit = 7,
	diffuse_burst = 8,
	diffuse_continuous = 9,
	//=======================
	bleed = 10,
	//=======================
	env_snow = 20,
	env_sand = 21,
	//=======================
	weapon_spear_skill_1 = 30,
	weapon_twin_sword_skill_1 = 31,

	//=======================
	etc = -1
};

struct Particle_Sync_Data
{
	UINT particle_ID;
	XMFLOAT3 obj_pos;
	XMFLOAT3 obj_look;

	Particle_Type particle_type;
	float LifeTime;
	XMFLOAT3 area_extent;
	XMFLOAT3 main_direction;
	XMFLOAT3 focus_point;
	UINT particle_status_index;
};

struct Particle_Format
{
	Particle_Shader_Type shader_type = Particle_Shader_Type::etc;
	Particle_Type particle_type;
	UINT max_particles = MAX_PARTICLES;

	XMFLOAT3 area_xyz{};
	UINT EmitFaceIndex; // EmitFace (0~5: -X,+X,-Y,+Y,-Z,+Z)

	float MaxLifetime;

	XMFLOAT3 main_direction {};
	int init_velocity_value {};
	XMFLOAT3 acceleration {};

	XMFLOAT3 color{};
	float size;
};

struct Render_Instance
{
	XMFLOAT4 Position_and_Scale;        // xyz = Position, w = Scale
	XMFLOAT4 Velocity_and_Rotate;       // xyz = axis, w = angle
	XMFLOAT4 Color;                     // rgba
};

struct Particle_Info
{
	XMFLOAT3 Position;
	float Lifetime;

	XMFLOAT3 Velocity;
	float MaxLifetime;

	XMFLOAT3 Acceleration;
	float Rotate_Value;

	XMFLOAT3 Color;
	UINT EmitFaceIndex; // EmitFace (0~5: -X,+X,-Y,+Y,-Z,+Z)

	float Size;            
	UINT Type;
	UINT Active;
	UINT Sleep;
};

struct CB_Particle_Update_Info
{
	XMFLOAT4X4 world_matrix;

	XMFLOAT3 EmitRegionMin;
	float ElapsedTime;

	XMFLOAT3 EmitRegionMax;
	UINT Max_Particle_N;

	XMFLOAT3 Main_Direction;
	float Init_Velocity_Value;

	XMFLOAT3 focus_point;
	float focus_strength;

	UINT obb_num;
	UINT Reset_Flag;
	XMFLOAT2 padding0;
};

//==============================================================================

class Particle
{
public:
	Particle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, Particle_Format particle_format);
	virtual ~Particle();

private:
	// 주요 리소스
	// RWStructuredBuffer<Particle_Info> 0
	// RWStructuredBuffer<RenderInstance> 1

	CTexture* particle_buffer_texture = NULL;

	ID3D12Resource* Particle_Info_List_counterBuffer = NULL;
	ID3D12Resource* Particle_Info_List_readbackBuffer = NULL;

	ID3D12Resource* Render_Instance_counterBuffer = NULL;
	ID3D12Resource* Render_Instance_readbackBuffer = NULL;

	ID3D12Resource* CounterResetBuffer = NULL;

	ID3D12Resource* Debug_buffer = NULL;
	ID3D12Resource* Debug_ReadBack_buffer = NULL;
	ID3D12Resource* Debug_Reset_Buffer = NULL;

	// 버퍼 뷰
	D3D12_VERTEX_BUFFER_VIEW m_RenderInstanceVBV = {};

	// 기타
	UINT m_nMaxParticles = MAX_PARTICLES;

public:
	UINT N_Particle_Info_List = 0;
	UINT N_Render_Instance = 0;


public:


	// 버퍼 생성 및 해제
	void Create_Resource_Buffers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, Particle_Format particle_format);

	void UpdateBuffers(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseBuffers();

	Particle_Info* Init_Particle_Data(const Particle_Format& particle_info);

	// 렌더링용 VBV 업데이트
	D3D12_VERTEX_BUFFER_VIEW Update_Render_Instance_VBV();

	UINT Get_Particle_Max_Num() const { return m_nMaxParticles; }

	void Copy_CounterBuffer_Particle_Info(ID3D12GraphicsCommandList* pd3dCommandList);
	void Copy_CounterBuffer_Render_Instance(ID3D12GraphicsCommandList* pd3dCommandList);
	void Copy_DebugBuffer(ID3D12GraphicsCommandList* pd3dCommandList);


	void Copy_CounterBuffer_All(ID3D12GraphicsCommandList* pd3dCommandList)
	{
		Copy_CounterBuffer_Particle_Info(pd3dCommandList);
		Copy_CounterBuffer_Render_Instance(pd3dCommandList);
//		Copy_DebugBuffer(pd3dCommandList);
	}

	UINT Readback_CounterBuffer_Particle_Info_List();
	UINT Readback_CounterBuffer_Render_Instance();
	UINT Readback_DebugBuffer();

	void Readback_All()
	{
		Readback_CounterBuffer_Particle_Info_List();
		Readback_CounterBuffer_Render_Instance();
//		Readback_DebugBuffer();

	}

	void ResetCounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* counterBuffer);

	void Reset_Particle_Info_List_CounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList);
	void Reset_Instance_CounterBuffer(ID3D12GraphicsCommandList* pd3dCommandList);
	void Reset_Debug_Buffer(ID3D12GraphicsCommandList* pd3dCommandList);

};

//==============================================================================

class Particle_Shape_Mesh : public CStandardMesh
{
protected:
	ID3D12Resource* m_pd3dColorBuffer = NULL;
	ID3D12Resource* m_pd3dColorUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dColorBufferView;

public:
	Particle_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~Particle_Shape_Mesh();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) {};
	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num) {}
};

class Cube_Shape_Mesh : public Particle_Shape_Mesh
{
public:
	Cube_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fSize = 2.0f);
	virtual ~Cube_Shape_Mesh();

	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num);
};

class Cube_Chip_Shape_Mesh : public Particle_Shape_Mesh
{
public:
	Cube_Chip_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fSize = 2.0f, float fHeight = 0.5f);
	virtual ~Cube_Chip_Shape_Mesh();

	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num);
};

class Sphere_Shape_Mesh : public Particle_Shape_Mesh
{
public:
	Sphere_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fRadius = 2.0f, int nSlices = 20, int nStacks = 20);
	virtual ~Sphere_Shape_Mesh();

	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num);
};

class Tetrahedron_Shape_Mesh : public Particle_Shape_Mesh
{
public:
	Tetrahedron_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fSize = 2.0f);
	virtual ~Tetrahedron_Shape_Mesh();

	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num);
};

class Cross_Shape_Mesh : public Particle_Shape_Mesh
{
public:
	Cross_Shape_Mesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fSize = 3.0f);
	virtual ~Cross_Shape_Mesh();

	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num);
};

class Billboard_Shape_Mesh : public Particle_Shape_Mesh
{
public:
	Billboard_Shape_Mesh(ID3D12Device* pd3dDevice = NULL, ID3D12GraphicsCommandList* pd3dCommandList = NULL, float fSize = 2.0f);
	virtual ~Billboard_Shape_Mesh();

	virtual void Instancing_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView, int instance_num);
};

//==============================================================================

class Particle_Manager;

class ParticleObject : public CGameObject
{
private:
	bool wasResetFlagSent = false;
	bool bEmitEnabled = true;

protected:
	XMFLOAT3 m_xmf3Direction = { 0.0f, 0.0f, 1.0f }; // 기본 전방
	float m_fSpeed = 0.0f;
	XMFLOAT3 m_xmf3Velocity = { 0.0f, 0.0f, 0.0f };

private:
	Particle_Manager* owner_manager = nullptr;
	Particle* particle_data = NULL;
	Particle_Type p_type = Particle_Type::etc;

	shared_ptr<Particle_Shape_Mesh>  shape_mesh = NULL;
	shared_ptr<CMaterial> particle_Material = NULL;
	bool is_textured = false;

	//=============================

	bool is_local = true;
	XMFLOAT3 local_area_xyz {};

	XMFLOAT3 focus_point {};
	float focus_strength = 0.0f;

	XMFLOAT3 direction {};
	int Init_Velocity_Value {};
	
	float ElapsedTime = 0.0f;
	float Max_Lifetime = 0.0f;
	//=============================
	Particle_Shader_Type shader_type = Particle_Shader_Type::etc;

	UINT Particle_State_Func_Index = 0; //for Sand
public:

	ParticleObject();
	virtual ~ParticleObject();

	void ReleaseUploadBuffers();

	void Set_Shape(shared_ptr<Particle_Shape_Mesh>  mesh_ptr) { shape_mesh = mesh_ptr; }
	shared_ptr<Particle_Shape_Mesh>  Get_Shape() { return shape_mesh; }
	void Set_Particle_Data(Particle* new_particle_obj = NULL) { particle_data = new_particle_obj; }
	void Set_Max_Interval(float new_max_lifetime) { Max_Lifetime = new_max_lifetime; }
	void Init_Info(Particle_Format particle_info);

	virtual void SetMesh(CMesh* pMesh = NULL) { m_pMesh = NULL; }
	virtual void SetMaterial(CMaterial* pMaterial);
	virtual void SetMaterial(int nMaterial, CMaterial* pMaterial);
	virtual void Set_BaseTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* filename);
	

	virtual void Update_Compute_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	void Update_Interval(float fTimeElapsed);
	void Reset_Interval() { wasResetFlagSent = false, ElapsedTime = 0.0f, Max_Lifetime = 0.0f; }

	virtual void Animate(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	Particle* Get_Particle_Data() { return particle_data; }
	CB_Particle_Update_Info Get_Particle_Update_Info(float fTimeElapsed, bool is_emit_stage);
	UINT Get_Particle_Max_Num() { return particle_data->Get_Particle_Max_Num(); }

	void Update_Particle_State();

	void Set_Particle_State(UINT new_index) { Particle_State_Func_Index = new_index; }
	UINT Get_Particle_State() { return Particle_State_Func_Index; }

	void Set_Local_Coordinate() { is_local = true; }
	void Set_World_Coordinate() { is_local = false; }
	bool Is_Local_Coordinate() const { return is_local; }

	void Set_Area(XMFLOAT3 new_local_area) { local_area_xyz = new_local_area; }
	XMFLOAT3 Get_Area() { return local_area_xyz; }

	void Set_Focus_Point(XMFLOAT3 world_point);
	XMFLOAT3 Get_Focus_Point() { return focus_point; };

	void Set_Focus_Strength(float new_value) { focus_strength = new_value; }
	float Get_Focus_Strength() { return focus_strength; }

	void Set_Main_Direction(const XMFLOAT3& input);
	XMFLOAT3 Get_Main_Direction();

	void Set_Init_Velocity_Value(int new_velocity_value) { Init_Velocity_Value = new_velocity_value; }
	int Get_Init_Velocity_Value() { return Init_Velocity_Value; }

	std::pair<XMFLOAT3, XMFLOAT3> GetAABB() { return ::GetAABB(XMFLOAT3(0.0f,0.0f,0.0f), local_area_xyz); }
	UINT Get_Size_Particle_Info_List() { return particle_data->N_Particle_Info_List; }
	UINT Get_Size_Render_Instance() { return particle_data->N_Render_Instance; }

	void Set_Direction(XMFLOAT3& dir) { m_xmf3Direction = Vector3::Normalize(dir); }
	void Set_Speed(float speed) { m_fSpeed = speed; }

	void Set_Shader_Type(Particle_Shader_Type setting_shader_type) { shader_type = setting_shader_type; }
	Particle_Shader_Type Get_Shader_Type() { return shader_type; };

	void Set_OwnerManager(Particle_Manager* mgr) { owner_manager = mgr; }
	void Add_Destroy_Queue();

	Particle_Type Get_Particle_Type() { return p_type; }


	void EnableEmit() { bEmitEnabled = true; }
	void DisableEmit() { bEmitEnabled = false; }
	bool IsEmitEnabled() const { return bEmitEnabled; }
};