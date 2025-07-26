#pragma once
#include "Descriptor_Heap.h"
#include "Object_Manager.h"
#include "Particle_Manager.h"
#include "UI_Manager.h"

#include "Shader.h"
#include "Shader_Compute.h"
#include "Player.h"





#define MAX_LIGHTS						16 

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

class Object_Manager;
class Particle_Manager;
class ParticleObject;
struct Particle_Sync_Data;

class Particle_Shape_Mesh;

class Texture_UI_Manager;
struct TextureBlock;

struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};

struct LIGHTS
{
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

struct Fog_Info
{
	XMFLOAT3 fogColor;
	int Fog_Trigger;

	float fogStart;
	float fogEnd;
	float fogDensity;
	float noiseScale;

	float noiseStrength;
	float time;
	XMFLOAT2 padding0;
};



#define NUM_CASCADES 3

struct alignas(16) LightCamera_Info
{
	UINT shadow_pass;
	UINT light_type;
	UINT padding0;
	UINT padding1;

	XMFLOAT4X4 LightViewProjTex[NUM_CASCADES];

	XMFLOAT3 LightDirectionWS;
	float shadow_bias;

	XMFLOAT2 shadow_map_size;
	XMFLOAT2 inv_shadow_map_size;

	float cascadeSplits[NUM_CASCADES];
};


#define LIGHT_CAMERA_TYPE_DIRECTIONAL 0

#define _SHADOWMAP_WIDTH 2048 
#define _SHADOWMAP_HEIGHT 2048 

class Shadow_Camera : public CCamera
{
public:
	bool update_shadow = true;

private:
	shared_ptr<CMaterial> shadow_map;
	ID3D12Resource* m_pd3dcb_LightCamera = NULL;
	LightCamera_Info* m_pcb_MappedLightCamera = NULL;

	std::vector<XMFLOAT4X4> m_CascadeView;   
	std::vector<XMFLOAT4X4> m_CascadeProj;

	float m_CascadeSplits[NUM_CASCADES];

protected:
	XMFLOAT3 m_light_direction = { 0.0f, -1.0f, 0.0f };
	XMFLOAT3 m_light_position = { 0.0f, 0.0f, 0.0f };

public:
	Shadow_Camera();
	virtual ~Shadow_Camera();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Update_Render_ShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, int cascadeIdx);


	std::vector<XMFLOAT3> CalcFrustumCornersWorld(CCamera* mainCamera, float nearZ, float farZ);
	void SetupCSMCascades(const XMFLOAT3& light_direction, const std::vector<float>& splitDepths, CCamera* mainCamera);
	std::vector<float> GenerateCSMSplitDepths(float nearZ, float farZ, int numCascades, float lambda = 0.75f);

	D3D12_CPU_DESCRIPTOR_HANDLE Get_Shadow_Map_DSV(int n) const;
	ID3D12Resource* Shadow_Camera::Get_Shadow_Map_Resource(int n) const;

};


class CScene
{
public:
	static bool bOBBRender;
	static UINT select_index;
	static bool Mouse_Lock;
	static bool Screen_Fade;

	Scene_Type scene_type;

	Change_Signal c_signal;
	float current_time = 0.0f;
	bool bUpdateUI_HP{ false };
	bool bUpdateUI_Screen{ false };
	bool bHitSignal{ false };
	bool bMenuActive{ false };
	bool bStartAnimation{ false };
	bool bSelectStart{ false };
	bool bStageClear{ false };
	bool CameraZoomOutAnime{ false };

public:
	CScene();
	~CScene();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void UpdateUIHoverState(HWND hWnd);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables_Light_Info(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables_Fog_Info(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables_ShadowMap(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void ReleaseShaderVariables();

	virtual void BuildDefaultLightsAndMaterials();
	virtual void Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void ReleaseObjects();


	ID3D12RootSignature* Create_MRT_GraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* Create_Transparent_GraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* Create_Plane_GraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* Create_UI_GraphicsRootSignature(ID3D12Device* pd3dDevice);


	shared_ptr<ID3D12RootSignature> Get_MRT_GraphicsRootSignature() { return(m_MRT_GraphicsRootSignature); }

	bool ProcessInput(UCHAR* pKeysBuffer);

	virtual void Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	virtual void Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void After_Update_Objects();
	
	virtual void Render_Depth(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


	virtual void Prepare_Shadow_Map_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Shadow_Map_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int n);

	virtual void Prepare_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void Render_SkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void Prepare_Transparent_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Transparent_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


	void Post_Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void ReleaseUploadBuffers();

	virtual Change_Signal Get_Change_Signal();
	shared_ptr<Particle_Manager> Get_Particle_Manager() { return particle_manager; }

	std::shared_ptr<CPlayer> Get_Client_Player() { return m_pPlayer; }
	std::shared_ptr<CCamera> Get_MainCamera() { return main_Camera; }
	std::shared_ptr<Shadow_Camera> Get_ShadowCamera() { return shadow_camera; }

	void Set_Client_Player(std::shared_ptr<CPlayer> new_player) { m_pPlayer = new_player; }
	void Set_MainCamera(std::shared_ptr<CCamera> new_camera) { main_Camera = new_camera; }
	
protected:
	static std::shared_ptr<ID3D12RootSignature> m_MRT_GraphicsRootSignature;
	static std::shared_ptr<ID3D12RootSignature> m_Transparent_GraphicsRootSignature;
	static std::shared_ptr<ID3D12RootSignature> m_Plane_GraphicsRootSignature;
	static std::shared_ptr<ID3D12RootSignature> m_UI_GraphicsRootSignature;

protected:
	vector<std::shared_ptr<CGameObject>> player_start_position_list;

	std::shared_ptr<CPlayer> m_pPlayer = NULL;
	std::shared_ptr<CCamera> main_Camera = NULL;
	std::shared_ptr<Shadow_Camera> shadow_camera = NULL;

public:
	std::shared_ptr<Particle_Manager>particle_manager = NULL;

	std::shared_ptr<ParticleObject> test_sand = NULL;
	std::shared_ptr<ParticleObject> test_particle = NULL;
	std::shared_ptr<ParticleObject> test_bleeding = NULL;


	std::shared_ptr <Object_Manager> obj_manager = NULL;

	std::shared_ptr<CHeightMapTerrain> m_pTerrain = NULL;
	std::shared_ptr<CSkyBox>	m_pSkyBox = NULL;


	LIGHT* m_pLights = NULL;
	int									m_nLights = 0;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;

	shared_ptr<Fog_Info> fog_info = NULL;
	shared_ptr<CMaterial>fog_noise = NULL;

	bool test_button = false;
	bool particle_test_button = false;

#ifdef WRITE_TEXT_UI
	Text_UI_Manager* text_ui_manager = NULL;
	void Build_Text_UI(Text_UI_Renderer* text_ui_renderer_ptr);
	std::vector<TextBlock*>* Get_Text_List();
	void Update_UI();
#endif

	Texture_UI_Manager* texture_ui_manager = NULL;
	virtual void Build_Texture_UI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::shared_ptr<ID3D12RootSignature> pRootSignature);
	std::vector<TextureBlock*> Get_Texture_List();
	virtual void Update_Texture_UI(float currentTime, float elapsedTime);
	virtual std::shared_ptr<std::vector<MonsterUIData>> GetNearbyMonstersUIData(float maxDistance);
	virtual void Update_Monster_HP_bar(float currentTime, float elapsedTime, float maxDistance);

	virtual void Set_UI_Layer_Active(std::vector<TextureBlock*>& blocks, UILayer targetLayer, bool bEnable);
	virtual void Bind_Player_UI_Callback();
	virtual void Bind_Player_UI_Updata_Callback();

	void Add_Multi_Player(shared_ptr<CPlayer> new_player_ptr);
	void Remove_Multi_Player(int player_id);
	bool Sync_Player_Data(int player_id, const ServerSyncData& syncData);
	bool Sync_Player_Blur(int player_id, bool motion_blur_active);

	XMFLOAT3 Get_Start_Position_List(int player_id);


	void Create_Particle_Object(const Particle_Sync_Data& syncData);
	void Update_Particle_Object(const Particle_Sync_Data& syncData);
	void Remove_Particle_Object(UINT p_obj_id);

	virtual void Sync_Monster_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int monsterID, const ServerSyncData& syncData);
	void Monster_Set_Active_False(int monsterID);

	virtual void SpawnMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int id, const XMFLOAT3& pos = XMFLOAT3(0, 0, 0));
	virtual void DespawnMonster(int id);

	virtual void DamageMonster(int id, float damage);

	void Fog_Sync(Fog_Info fog_info);

};

class Character_Select_Scene : public CScene
{
private:
	int is_Ready = 0; 


	std::array<int, MaxPlayer> readyClientIds;
	std::array<std::bitset<MaxPlayer>, MaxPlayer> characterSelections;


public:
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void BuildDefaultLightsAndMaterials();
	virtual void Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


	virtual void Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	virtual void Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void UpdatePlayerSelection();

	virtual void Build_Texture_UI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::shared_ptr<ID3D12RootSignature> pRootSignature);

	int Get_Selected_Character_Index() const { return select_index; }
	int Get_Character_Select_Status() const { return is_Ready; }
	void Set_Character_Select_Status(bool new_is_selected) { is_Ready = new_is_selected; }

	void SetEnableCharactorSelectButton(bool Enable);

	const std::array<int, MaxPlayer>& GetReadyClientIds() const { return readyClientIds; }
	const std::array<std::bitset<MaxPlayer>, MaxPlayer>& GetCharacterSelections() const { return characterSelections; }

	void SetReadyClientIds(const std::array<int, MaxPlayer>& readyIds) { readyClientIds = readyIds; }
	void SetCharacterSelections(const std::array<std::bitset<MaxPlayer>, MaxPlayer>& selections) { characterSelections = selections; }
};

class Board_Scene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

private:
	std::vector<XMFLOAT3> island_points;

	std::shared_ptr<Boat_Object> pirate_ship;
	std::shared_ptr<Wave_Object> wave_plane;
	std::shared_ptr<ParticleObject> water_particle_1;
	std::shared_ptr<ParticleObject> water_particle_2;

	string camera_position = "";
	bool focus_button = false;
	float cameraYOffset{ 0.0f };

	bool bClosedByUser = false;

private:
	virtual void BuildDefaultLightsAndMaterials();
	virtual void Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


	virtual void Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	virtual void Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void After_Update_Objects();
	int Get_Closest_Island_Index(float range);

	void SetCameraTarget(std::string_view target);

	virtual void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Transparent_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void Build_Texture_UI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::shared_ptr<ID3D12RootSignature> pRootSignature);
	
	void OnMenuCloseButtonClicked(std::vector<TextureBlock*>& blocks);


	void SetcameraYOffset(float offset) { cameraYOffset = offset; }

public: // For Server Sync
	static void Reset_Sail_Status();
	void Sync_Boat_Server(XMFLOAT3 pos, XMFLOAT3 look);
	pair<int, bool> Get_Sail_Status();
private:
	static int nearest_stage_index;
	static bool is_stage_select;
};

class Stage_Scene : public CScene
{
public:
	static bool Change_Scene_Signal; // For send server
	static bool Stage_Clear_Signal; // For Get server
	static bool Monster_Depth_Render; // For Get server
private:
	virtual void BuildDefaultLightsAndMaterials() {}
	virtual void Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {}

public:
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {}

	virtual void Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	virtual void Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Render_Depth(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//=============================================================
	// Server Sync Func

	void Add_Multi_Player(shared_ptr<CPlayer> new_player_ptr);
	void Remove_Multi_Player(int player_id);
	void Sync_Player_Data(int player_id, const ServerSyncData& syncData);


	virtual void Sync_Monster_Data(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int monsterID, const ServerSyncData& syncData);

	virtual void SpawnMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int id, const XMFLOAT3& pos = XMFLOAT3(0, 0, 0));
	virtual void DespawnMonster(int id);
};

class Stage_1_Scene : public Stage_Scene
{
private:
	shared_ptr<ParticleObject> env_ash_particle = NULL;

	virtual void BuildDefaultLightsAndMaterials();
	virtual void Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

public:
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

};

class Stage_2_Scene : public Stage_Scene
{
private:
	shared_ptr<ParticleObject> env_sand_particle = NULL;

	
	virtual void BuildDefaultLightsAndMaterials();
	virtual void Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

public:
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

};

//===========================================================================

class Test_Scene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

private:
	virtual void Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	virtual void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

};