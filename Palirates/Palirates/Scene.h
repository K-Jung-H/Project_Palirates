//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once
#include "UI_Manager.h"
#include "Object_Manager.h"
#include "Particle_Manager.h"
#include "Descriptor_Heap.h"

#include "Shader.h"
#include "Shader_Compute.h"
#include "Player.h"

#define MAX_LIGHTS						16 

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

class Particle_Manager;
class ParticleObject;


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


class CScene
{
public:
    CScene();
    ~CScene();

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables_Light_Info(ID3D12GraphicsCommandList* pd3dCommandList);
	
	virtual void ReleaseShaderVariables();

	virtual void BuildDefaultLightsAndMaterials();
	virtual void Prepare_Basic_Elements(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);

	void ReleaseObjects();

	ID3D12RootSignature *Create_MRT_GraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature* Create_Transparent_GraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* Create_Plane_GraphicsRootSignature(ID3D12Device* pd3dDevice);

	
	shared_ptr<ID3D12RootSignature> Get_MRT_GraphicsRootSignature() { return(m_MRT_GraphicsRootSignature); }

	bool ProcessInput(UCHAR *pKeysBuffer);

	virtual void Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);	
	virtual void Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void After_Update_Objects();

	void Prepare_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
    virtual void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	void Prepare_Transparent_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void Transparent_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);


	void Post_Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	void ReleaseUploadBuffers();

	Particle_Manager* Get_Particle_Manager() { return particle_manager; }

	//CPlayer								*m_pPlayer = NULL;
	shared_ptr<CPlayer> m_pPlayer = NULL;

	bool bOBBRender{ false };

protected:
	static std::shared_ptr<ID3D12RootSignature> m_MRT_GraphicsRootSignature;
	static std::shared_ptr<ID3D12RootSignature> m_Transparent_GraphicsRootSignature;
	static std::shared_ptr<ID3D12RootSignature> m_Plane_GraphicsRootSignature;

public:
	Particle_Manager* particle_manager = NULL;
	std::shared_ptr<ParticleObject> test_sand = NULL;
	std::shared_ptr<ParticleObject> test_dragonfire = NULL;

	std::shared_ptr<Wave_Object> in_game_wave;


	Object_Manager* obj_manager = NULL;

	std::vector<std::shared_ptr<CShader>> Shader_list;

	CSkyBox								*m_pSkyBox = NULL;
	std::shared_ptr<CHeightMapTerrain> m_pTerrain;


	LIGHT								*m_pLights = NULL;
	int									m_nLights = 0;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;

	bool test_button = false;
	bool particle_test_button = false;

#ifdef WRITE_TEXT_UI
	Text_UI_Manager* text_ui_manager = NULL;
	void Build_Text_UI(Text_UI_Renderer* text_ui_renderer_ptr);
	std::vector<TextBlock*>* Get_Text_List();
	void Update_UI();
#endif
};

class Test_Scene : public CScene
{
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	virtual void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

};

class Character_Select_Scene : public CScene
{
private:
	UINT prev_index = -1;
	UINT select_index = 0;
	virtual void BuildDefaultLightsAndMaterials();
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	virtual void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void UpdatePlayerSelection(int new_index);
};

class Board_Scene : public CScene
{
private:
	std::shared_ptr<Boat_Object> pirate_ship;
	std::shared_ptr<Wave_Object> wave_plane;
	std::shared_ptr<ParticleObject> water_particle_1;
	std::shared_ptr<ParticleObject> water_particle_2;

	string camera_position = "";
	bool focus_button = false;
public:
	virtual void BuildDefaultLightsAndMaterials();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Animate_Objects(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed);
	virtual void Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void After_Update_Objects();

	void SetCameraTarget(std::string_view target);

	virtual void Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};

class Weapon_Select_Scene : public CScene
{
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}
};