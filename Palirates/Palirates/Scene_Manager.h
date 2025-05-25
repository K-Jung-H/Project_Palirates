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

    PostProcessBaseShader* MRT_shader = NULL;

    std::map<int, CPlayer*> players;

public:
    Scene_Manager();
    Scene_Manager(UINT nFrames, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight);
    ~Scene_Manager();

    bool Register_Scene(std::string_view sceneName, std::shared_ptr<CScene> scene);
    void Unload_Scene();

    std::shared_ptr<CScene>Load_Scene(std::string_view sceneName);

    bool Set_Active_Scene(std::string_view sceneName);
    std::shared_ptr<CScene> Get_Active_Scene() { return activeScene; }
    CScene* Get_Active_Scene_Ptr() { return activeScene.get(); }

    bool Set_Scene_Player(std::string_view sceneName, shared_ptr<CPlayer> player_ptr);
    void Set_Active_Scene_Main_Camera(const std::shared_ptr<CCamera>& newCamera);

    shared_ptr<CPlayer> Get_Active_Scene_Player();
    shared_ptr<CCamera> Get_Active_Scene_Main_Camera();
    
    void Set_Shader(PostProcessBaseShader* shader_ptr) { MRT_shader = shader_ptr; }

    void Build_Scene(std::string_view sceneName, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    void Animate_Active_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float deltaTime);
    void Update_Active_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    void After_Update_Active_Objects();

    void Clear_Particles_Update_Result(ID3D12GraphicsCommandList* pd3dCommandList);



    void Update_UI();
    void Update_Texture_UI();

    void Prepare_Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    
    void Prepare_MRT_G_Buffer(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle);
    void Render_MRT_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    // Render Alpha obj
    void Prepare_Render_Transparent_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    void Render_Transparent_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    
    void Prepare_Deffered_Render_Scene(ID3D12GraphicsCommandList* pd3dCommandList);
    void Deffered_Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


    void Post_Update_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    void Render_Scene_UI(UINT nFrame);
    void Render_Scene_Texture_UI(ID3D12GraphicsCommandList* cmdList);

    void ReleaseUploadBuffers();

    std::function<void(const std::string&)> sceneChangeCallback;

    void SetSceneChangeCallback(std::function<void(const std::string&)> fn) {
        sceneChangeCallback = fn;
    }

    //===============¼­¹ö===============
    //void AddPlayer(int playerId, CPlayer* player);
    CPlayer* GetPlayerById(int playerId);
    void RegisterRemotePlayer(int playerId, std::shared_ptr<CTerrainPlayer> player);
    void CreateRemotePlayer(int id, const DirectX::XMFLOAT3& pos, int state);
};
