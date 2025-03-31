#include "stdafx.h"
#include "Scene_Manager.h"
#include "Scene.h"


Scene_Manager::Scene_Manager()
{
    activeScene = nullptr;
}

Scene_Manager::Scene_Manager(UINT nFrames, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{

#ifdef WRITE_TEXT_UI
    text_ui_renderer = make_shared<Text_UI_Renderer>(nFrames, pd3dDevice, pd3dCommandQueue, ppd3dRenderTargets, nWidth, nHeight);
#endif

}

Scene_Manager::~Scene_Manager()
{
    for (auto& pair : sceneCache)
    {
        pair.second->ReleaseObjects();
    }
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

//        material_reflectance_data_manager->Create_ShaderVariables(pd3dDevice, pd3dCommandList);

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

void Scene_Manager::Update_Active_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
    if (activeScene) 
    {
        activeScene->Animate_Objects(pd3dCommandList, fTimeElapsed);
        activeScene->Update_Objects(pd3dDevice, pd3dCommandList);
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");
}

void Scene_Manager::Update_Active_Particles(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
#ifdef RENDER_PARTICLE
    if (activeScene)
        activeScene->Animate_Particles(pd3dCommandList, fTimeElapsed);    
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");
#endif
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
void Scene_Manager::Prepare_MRT_G_Buffer(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle)
{
    if (MRT_shader)
    {
        // Connect Multi_RenderTarget
        // nRenderTarget = 0 -> Not use BackBuffer in this time, 
        MRT_shader->Prepare_Multi_RenderTarget(pd3dCommandList, 0, pd3dRtvCPUHandles, pd3dDsvCPUHandle);
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  MRT_shader is not exist");

}

void Scene_Manager::Prepare_Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (activeScene)
    {
        activeScene->Prepare_Render(pd3dDevice, pd3dCommandList, pCamera);
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

}


void Scene_Manager::Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (activeScene)
        activeScene->Render(pd3dDevice, pd3dCommandList, pCamera);

    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

}

void Scene_Manager::Prepare_Deffered_Render_Scene(ID3D12GraphicsCommandList* pd3dCommandList)
{
    //	Change Used RenderTarget Resource State
    if (MRT_shader)
        MRT_shader->OnPostRenderTarget(pd3dCommandList);
}

void Scene_Manager::Deffered_Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if(MRT_shader)
        MRT_shader->Setting_Render(pd3dCommandList, 0);


    if (activeScene)
        activeScene->UpdateShaderVariables_Light_Info(pd3dCommandList);


    if (pCamera)
        pCamera->Update_PostRender_ShaderVariables(pd3dCommandList);
    

    if(MRT_shader)
        MRT_shader->Render(pd3dCommandList, NULL);

}

void Scene_Manager::Prepare_Post_Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    
}

void Scene_Manager::Post_Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{

}

// 렌더링이 모두 끝나면, 데이터  처리 작업
void Scene_Manager::Finalize_Frame_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (activeScene)  
        activeScene->Finalize_Frame(pd3dDevice, pd3dCommandList, pCamera);
    
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

//===============서버===============
CPlayer* Scene_Manager::GetPlayerById(int playerId)
{
    auto it = players.find(playerId);
    if (it != players.end())
    {
        return it->second;
    }
    return nullptr;
}