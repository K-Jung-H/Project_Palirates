#pragma once
#pragma once
#include "Scene.h"

class Scene_Manager
{
private:
    std::shared_ptr<CScene> activeScene;
    std::unordered_map<std::string, std::shared_ptr<CScene>> sceneCache;
    
    std::shared_ptr<Text_UI_Renderer> text_ui_renderer;

public:
    Scene_Manager(UINT nFrames, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight);
    ~Scene_Manager();

    bool Register_Scene(std::string_view sceneName, std::shared_ptr<CScene> scene);
    void Unload_Scene();

    std::shared_ptr<CScene>Load_Scene(std::string_view sceneName);

    bool Set_Active_Scene(std::string_view sceneName);
    std::shared_ptr<CScene> Get_Active_Scene() { return activeScene; }
    CScene* Get_Active_Scene_Ptr() { return activeScene.get(); }

    bool Scene_Manager::Set_Scene_Player(std::string_view sceneName, CPlayer* player_ptr);


    void Build_Scene(std::string_view sceneName, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    void Update_Active_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float deltaTime);
    void Update_UI();


    void Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    void Render_Scene_UI(UINT nFrame);

    void ReleaseUploadBuffers();
};
