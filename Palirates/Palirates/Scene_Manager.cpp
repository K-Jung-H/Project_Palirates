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

void Build_Scene(Scene_Type scene_type, string scene_name)
{

}

bool Scene_Manager::Register_Scene(std::string_view sceneName, std::shared_ptr<CScene> scene)
{
    if (sceneCache.find(std::string(sceneName)) != sceneCache.end())
    {
        DebugOutput("[Scene_Manager] ERROR: Register scene failed - '" + std::string(sceneName) + " is exist.");
        return false;
    }

    //if (sceneChangeCallback) {
    //    scene->requestSceneChange = sceneChangeCallback;
    //}

    sceneCache[std::string(sceneName)] = scene;
    return true;
}

std::shared_ptr<CScene> Scene_Manager::Load_Scene(std::string_view sceneName)
{
    auto it = sceneCache.find(std::string(sceneName));
    if (it != sceneCache.end())
    {
        return it->second;
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

bool Scene_Manager::Find_Scene(std::string_view sceneName)
{
    auto it = sceneCache.find(std::string(sceneName));
    if (it != sceneCache.end())
    {
        return true;
    }

    DebugOutput("[Scene_Manager] ERROR:  Can't find " + std::string(sceneName));
    return false;
}

void Scene_Manager::Build_Scene(Scene_Type scene_type, string scene_name, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

    switch (scene_type)
    {

    case Lobby:
    {
        std::shared_ptr<Character_Select_Scene> character_select_scene = std::make_shared<Character_Select_Scene>();
        character_select_scene->BuildObjects(pd3dDevice, pd3dCommandList);

        Register_Scene(scene_name, character_select_scene);
        std::shared_ptr<Observer> select_scene_observer = std::make_shared<Observer>(pd3dDevice, pd3dCommandList, character_select_scene->Get_MRT_GraphicsRootSignature());
        select_scene_observer->SetPosition(XMFLOAT3{ 52.0f, 0.0f, 35.0f });
        select_scene_observer->Rotate(0.0f, -120.0f, 0.0f);
        Set_Scene_Player(scene_name, select_scene_observer);
    }
    break;

    case Board:
    {
        std::shared_ptr<Board_Scene> game_board_scene = std::make_shared<Board_Scene>();
        game_board_scene->BuildObjects(pd3dDevice, pd3dCommandList);

        Register_Scene(scene_name, game_board_scene);
        std::shared_ptr<Observer> game_board_observer = std::make_shared<Observer>(pd3dDevice, pd3dCommandList, game_board_scene->Get_MRT_GraphicsRootSignature());
        Set_Scene_Player(scene_name, game_board_observer);
    }
    break;

    case Stage_1:
    {
        std::shared_ptr<CScene> in_stage_scene = std::make_shared<CScene>();
        in_stage_scene->BuildObjects(pd3dDevice, pd3dCommandList);

        Register_Scene(scene_name, in_stage_scene);
        std::shared_ptr<CTerrainPlayer> pPlayer = std::make_shared<CTerrainPlayer>(pd3dDevice, pd3dCommandList, in_stage_scene->Get_MRT_GraphicsRootSignature(), in_stage_scene->m_pTerrain.get(), Captain);
        pPlayer->Set_Child(pPlayer->m_pRootModel);
        pPlayer->SetupWeaponCollider();
        pPlayer->SetPosition(XMFLOAT3(1500.0f, 0.0f, 692.0f));
        in_stage_scene->obj_manager->Add_Object(pPlayer, Object_Type::skinned);
        Set_Scene_Player(scene_name, pPlayer);
    }
    break;

    case Stage_2:
    {
    }
    break;

    case etc:
    default:
        break;
    }



#ifdef WRITE_TEXT_UI
        it->second->Build_Text_UI(text_ui_renderer.get());
#endif


}

bool Scene_Manager::Set_Scene_Player(std::string_view sceneName, shared_ptr<CPlayer> player_ptr)
{
    auto it = sceneCache.find(std::string(sceneName));
    if (it != sceneCache.end())
    {
        it->second->m_pPlayer = player_ptr;
        it->second->main_Camera = player_ptr->GetCamera();
        return true;
    }

    DebugOutput("[Scene_Manager] ERROR:  Can't find " + std::string(sceneName));
    return false;
}

shared_ptr<CPlayer> Scene_Manager::Get_Active_Scene_Player()
{
    if (activeScene)
        return activeScene->m_pPlayer;
    else
        DebugOutput("[Scene_Manager] ERROR:  Active_Scene is NULL");
    return NULL;

}

shared_ptr<CCamera> Scene_Manager::Get_Active_Scene_Main_Camera()
{
    if (activeScene)
        return activeScene->main_Camera;
    else
        DebugOutput("[Scene_Manager] ERROR:  Active_Scene is NULL");
    return NULL;

}

bool Scene_Manager::Check_Scene_Change_Signal()
{
    bool result = false;

    if (activeScene)
    {
        Change_Signal signal = activeScene->Get_Change_Signal();
        if (signal.type != etc && signal.scene_name != "")
            result = true;
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Active_Scene is NULL");


    return result;
}

void Scene_Manager::Set_Active_Scene_Main_Camera(const std::shared_ptr<CCamera>& newCamera)
{
    if (activeScene)
    {
        activeScene->main_Camera = newCamera;
    }
    else
    {
        DebugOutput("[Scene_Manager] ERROR: Cannot set main_Camera - Active_Scene is NULL");
    }
}

void Scene_Manager::Animate_Active_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
    if (activeScene) 
        activeScene->Animate_Objects(pd3dCommandList, fTimeElapsed);     
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

#ifdef RENDER_PARTICLE
    Particle_Manager* particle_manager = NULL;

    if (activeScene)
        particle_manager = activeScene->Get_Particle_Manager();
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

    if (particle_manager)
        particle_manager->AnimateObjects(pd3dCommandList, fTimeElapsed);
    else
        DebugOutput("[Scene_Manager] ERROR:  particle_manager is not exist");

#endif
}

void Scene_Manager::Update_Active_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (activeScene)
    {
        activeScene->Update_Objects(pd3dDevice, pd3dCommandList);
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

#ifdef RENDER_PARTICLE
    Particle_Manager* particle_manager = NULL;

    if (activeScene)
        particle_manager = activeScene->Get_Particle_Manager();
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

    if (particle_manager)
        particle_manager->Copy_CounterBuffer(pd3dCommandList);
    else
        DebugOutput("[Scene_Manager] ERROR:  particle_manager is not exist");

#endif
}

void Scene_Manager::After_Update_Active_Objects()
{
    if (activeScene)
    {
        activeScene->After_Update_Objects();
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

#ifdef RENDER_PARTICLE
    Particle_Manager* particle_manager = NULL;

    if (activeScene)
        particle_manager = activeScene->Get_Particle_Manager();
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

    if (particle_manager)
        particle_manager->Sync_AfterAnimateObjects();
    else
        DebugOutput("[Scene_Manager] ERROR:  particle_manager is not exist");

#endif

}


void Scene_Manager::Clear_Particles_Update_Result(ID3D12GraphicsCommandList* pd3dCommandList)
{
#ifdef RENDER_PARTICLE
    Particle_Manager* particle_manager = NULL;

    if (activeScene)
        particle_manager = activeScene->Get_Particle_Manager();
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

    if (particle_manager)
        particle_manager->Clear_CounterBuffer(pd3dCommandList);
    else
        DebugOutput("[Scene_Manager] ERROR:  particle_manager is not exist");

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

void Scene_Manager::Update_Texture_UI()
{
    if (activeScene)
    {
        activeScene->Update_Texture_UI();
    }
}

void Scene_Manager::Unload_Scene()
{
    activeScene.reset();
}


void Scene_Manager::Render_Scene_ShadowMap(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int n)
{
    if (activeScene)
    {
        activeScene->Shadow_Map_Render(pd3dDevice, pd3dCommandList, n);
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");
}



void Scene_Manager::Prepare_MRT_G_Buffer(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle)
{
    if (MRT_shader)
    {
        // Connect Multi_RenderTarget
        // nRenderTarget = 0 -> Not use BackBuffer in this time
        MRT_shader->Prepare_Multi_RenderTarget(pd3dCommandList, 0, pd3dRtvCPUHandles, pd3dDsvCPUHandle);
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  MRT_shader is not exist");

}
void Scene_Manager::Prepare_Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (activeScene)
    {
        activeScene->Prepare_Render(pd3dDevice, pd3dCommandList);
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

}
void Scene_Manager::Render_MRT_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (activeScene)
        activeScene->Render(pd3dDevice, pd3dCommandList);

    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

}


void Scene_Manager::Prepare_Render_Transparent_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (activeScene)
    {
        activeScene->Prepare_Transparent_Render(pd3dDevice, pd3dCommandList);
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

}
void Scene_Manager::Render_Transparent_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (activeScene)
        activeScene->Transparent_Render(pd3dDevice, pd3dCommandList);

    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");

}



void Scene_Manager::Prepare_Deffered_Render_Scene(ID3D12GraphicsCommandList* pd3dCommandList)
{
    //	Change Used RenderTarget Resource State
    if (MRT_shader)
        MRT_shader->OnPostRenderTarget(pd3dCommandList);
}
void Scene_Manager::Deffered_Render_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    if(MRT_shader)
        MRT_shader->Setting_Render(pd3dCommandList, 0);


    if (activeScene)
    {
        activeScene->UpdateShaderVariables_Light_Info(pd3dCommandList);
        activeScene->UpdateShaderVariables_Fog_Info(pd3dCommandList);
        activeScene->UpdateShaderVariables_ShadowMap(pd3dCommandList);
        activeScene->main_Camera->Update_Deffered_Render_ShaderVariables(pd3dCommandList);
    }

    Light_Material_Manager::UpdateGraphicsShaderVariables(pd3dCommandList);


    if(MRT_shader)
        MRT_shader->Render(pd3dCommandList, NULL);

}


void Scene_Manager::Post_Update_Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (activeScene)    
        activeScene->Post_Update(pd3dDevice, pd3dCommandList);
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

void Scene_Manager::Render_Scene_Texture_UI(ID3D12GraphicsCommandList* cmdList, float currentTime, float elapsedTime)
{
    if (activeScene) {
        if (activeScene->texture_ui_manager) {
            activeScene->currentTime = currentTime;
            activeScene->texture_ui_manager->RenderAll(cmdList, currentTime, elapsedTime);
        }
    }
    else
        DebugOutput("[Scene_Manager] ERROR:  Active Scene is not exist");
}

void Scene_Manager::ReleaseUploadBuffers()
{
    for (auto& pair : sceneCache)
    {
        pair.second->ReleaseUploadBuffers();
    }
}

//===============¼­¹ö===============
CPlayer* Scene_Manager::GetPlayerById(int playerId)
{
    auto it = players.find(playerId);
    if (it != players.end())
    {
        return it->second;
    }
    return nullptr;
}

void Scene_Manager::RegisterRemotePlayer(int playerId, std::shared_ptr<CTerrainPlayer> player)
{
    players[playerId] = player.get();
}
