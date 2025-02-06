#include "stdafx.h"
#include "Scene_Manager.h"
#include "Scene.h"


Scene_Manager::Scene_Manager(UINT nFrames, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
#ifdef WRITE_TEXT_UI
    text_ui_renderer = make_shared<Text_UI_Renderer>(nFrames, pd3dDevice, pd3dCommandQueue, ppd3dRenderTargets, nWidth, nHeight);
#endif
}

Scene_Manager::~Scene_Manager()
{
    sceneCache.clear();
}

bool Scene_Manager::Register_Scene(std::string_view sceneName, std::shared_ptr<CScene> scene)
{
    if (sceneCache.find(std::string(sceneName)) != sceneCache.end())
    {
        DebugOutput("[Scene_Manager] ERROR: Register scene failed - '" + std::string(sceneName) + " is exist.");
        return false;
    }

    sceneCache[std::string(sceneName)] = scene;
    return true;
}

std::shared_ptr<CScene> Scene_Manager::Load_Scene(std::string_view sceneName)
{
    auto it = sceneCache.find(std::string(sceneName));
    if (it != sceneCache.end())
    {
        activeScene = it->second;  // 기존 씬을 활성화
        return activeScene;
    }

    DebugOutput("[Scene_Manager] ERROR: Scene '" + std::string(sceneName) + "' not found.");
    return nullptr;
}

bool Scene_Manager::Set_Active_Scene(std::string_view sceneName)
{
    auto it = sceneCache.find(std::string(sceneName));
    if (it != sceneCache.end())
    {
        activeScene = it->second;
        return true;
    }

    DebugOutput("[Scene_Manager] ERROR:  Can't find " + std::string(sceneName));
    return false;
}


void Scene_Manager::Build_Scene(std::string_view sceneName, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    auto it = sceneCache.find(std::string(sceneName));
    if (it != sceneCache.end())
    {
        it->second->BuildObjects(pd3dDevice, pd3dCommandList);
#ifdef WRITE_TEXT_UI
        it->second->Build_Text_UI(text_ui_renderer.get());
#endif
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Can't find " + std::string(sceneName));

}

bool Scene_Manager::Set_Scene_Player(std::string_view sceneName, CPlayer* player_ptr)
{
    auto it = sceneCache.find(std::string(sceneName));
    if (it != sceneCache.end())
    {
        it->second->m_pPlayer = player_ptr;
        return true;
    }

    DebugOutput("[Scene_Manager] ERROR:  Can't find " + std::string(sceneName));
    return false;
}

void Scene_Manager::Update_Active_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
    if (activeScene) 
    {
        activeScene->AnimateObjects(fTimeElapsed);
        activeScene->Update_Objects(pd3dDevice, pd3dCommandList);
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");
}

void Scene_Manager::Update_UI()
{
#ifdef WRITE_TEXT_UI
    if (activeScene)
    {
        activeScene->Update_UI();
    }
#endif
}

void Scene_Manager::Unload_Scene()
{
    activeScene.reset();
}

void Scene_Manager::Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (activeScene) 
        activeScene->Render(pd3dDevice, pd3dCommandList, pCamera);
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

}

void Scene_Manager::Render_Scene_UI(UINT nFrame)
{
#ifdef WRITE_TEXT_UI
    if (activeScene)
        text_ui_renderer->Render(nFrame, activeScene->Get_Text_List());
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");
#endif
}

void Scene_Manager::ReleaseUploadBuffers()
{
    for (auto& pair : sceneCache)
    {
        pair.second->ReleaseUploadBuffers();
    }
}