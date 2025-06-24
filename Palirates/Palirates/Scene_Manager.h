#pragma once
#include "Scene.h"
#include "Object.h"
#include "Shader.h"


class Scene_Manager
{
private:
    std::shared_ptr<CScene> activeScene;
    std::unordered_map<std::string, std::shared_ptr<CScene>> sceneCache;
    
    std::shared_ptr<Text_UI_Renderer> text_ui_renderer;
    std::shared_ptr<Texture_UI_Renderer> texture_ui_renderer;

    std::shared_ptr<ID3D12RootSignature> Empty_GraphicsRootSignature;

    PostProcessBaseShader* MRT_shader = NULL;
    ScreenFade_Shader* Fade_shader = NULL;


    std::map<int, CPlayer*> players;

public:
    Scene_Manager();
    Scene_Manager(UINT nFrames, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight);
    ~Scene_Manager();

    ID3D12RootSignature* Create_EmptyRootSignature(ID3D12Device* pd3dDevice);


    bool Register_Scene(std::string_view sceneName, std::shared_ptr<CScene> scene);
    void Unload_Scene();

    std::shared_ptr<CScene>Load_Scene(std::string_view sceneName);

    bool Set_Active_Scene(std::string_view sceneName);
    bool Find_Scene(std::string_view sceneName);
    bool Get_Active_Scene_Mouse_State();
    bool Get_Active_Scene_Fade_State();

    std::shared_ptr<CScene> Get_Active_Scene() { return activeScene; }
    CScene* Get_Active_Scene_Ptr() { return activeScene.get(); }

    bool Set_Scene_Player(std::string_view sceneName, shared_ptr<CPlayer> player_ptr);
    void Set_Active_Scene_Main_Camera(const std::shared_ptr<CCamera>& newCamera);

    shared_ptr<CPlayer> Get_Active_Scene_Player();
    shared_ptr<CCamera> Get_Active_Scene_Main_Camera();
    std::shared_ptr<Particle_Manager> Get_Active_Scene_Particle_Manager();

    bool Check_Scene_Change_Signal();

    void Set_MRT_Shader(PostProcessBaseShader* shader_ptr) { MRT_shader = shader_ptr; }
    void Set_ScreenFade_Shader(ScreenFade_Shader* shader_ptr) { Fade_shader = shader_ptr; }

    void Build_Scene(Scene_Type scene_type, string scene_name, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    void Animate_Active_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float deltaTime);
    void Update_Active_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    void After_Update_Active_Objects();

    void Clear_Particles_Update_Result(ID3D12GraphicsCommandList* pd3dCommandList);


    void Update_UI();
    void Update_Texture_UI(float currentTime, float elapsedTime);

    void Prepare_Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    
    void Prepare_MRT_G_Buffer(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle);
    void Render_MRT_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    // Render Alpha obj
    void Prepare_Render_Transparent_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    void Render_Transparent_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    void Prepare_Render_Scene_ShadowMap(ID3D12GraphicsCommandList* pd3dCommandList);
    void Render_Scene_ShadowMap(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int n);


    void Prepare_Deffered_Render_Scene(ID3D12GraphicsCommandList* pd3dCommandList);
    void Deffered_Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


    void Post_Update_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    void Render_Scene_UI(UINT nFrame);
    void Render_Scene_Texture_UI(ID3D12GraphicsCommandList* cmdList, float currentTime, float elapsedTime);

    void Render_ScreenFade(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    void ReleaseUploadBuffers();


    //===============¼­¹ö===============
    //void AddPlayer(int playerId, CPlayer* player);
    CPlayer* GetPlayerById(int playerId);
    void RegisterRemotePlayer(int playerId, std::shared_ptr<CTerrainPlayer> player);
    void CreateRemotePlayer(int id, const DirectX::XMFLOAT3& pos, int state);
};
