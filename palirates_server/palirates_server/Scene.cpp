#include "stdafx.h"
#include "Scene.h"

int Scene::active_client_num;
std::array<int, MaxPlayer> Scene::player_model_list = { -1, -1, -1, -1, -1, -1 };

Scene::Scene(Scene_Type type)
    : sceneType(type)
{
}

Scene_Type Scene::GetSceneType() const 
{
    return sceneType;
}

Scene_Type Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (Change_Scene_Trigger)
        return Scene_Type::Stage_1;
    else
        return Scene_Type::None;
}

void Scene::Update_Scene(float elapsedTime)
{

}

//======================================================
void Lobby_Scene::Init()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (int i = 0; i < MaxPlayer; ++i)
    {
        characterReady[i] = -1;
        for (int j = 0; j < MaxPlayer; ++j)
        {
            characterSelections[i][j] = false;
        }
    }

    // test
    std::shared_ptr<Monster> m = std::make_shared<Fishman>(1);
    Monster_List.push_back(m);
}

void Lobby_Scene::Update_Scene(float elapsedTime)
{
    for (auto m : Monster_List) {
        auto con = m->GetSkinnedAnimationController();
        if (con) {
            if (m->GetStateMachine())
                m->GetStateMachine()->update(elapsedTime);
            con->AdvanceTime(elapsedTime, m.get());
           /* for (int i = 0; i < con->m_nAnimationTracks; i++) {
                std::cout << con->m_pAnimationTracks[i].m_fWeight << " " << con->m_pAnimationTracks[i].m_fPosition;
            }
            std::cout << "\n";*/
        }
        else {
            //std::cout << "con 없음" << std::endl;
        }
    }
}

void Lobby_Scene::Remove_Player(int id)
{
    if (id < 0 || id >= MaxPlayer)
        return;

    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (int i = 0; i < MaxPlayer; ++i)
    {
        characterSelections[i][id] = false;

        if (characterReady[i] == id)
            characterReady[i] = -1;
    }
}

bool Lobby_Scene::SelectCharacter(int clientId, int characterId, bool isReady)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (characterId < 0 || characterId >= MaxPlayer) return false;
    if (clientId < 0 || clientId >= MaxPlayer) return false;

    for (int i = 0; i < MaxPlayer; ++i)
        characterSelections[i][clientId] = false;

    characterSelections[characterId][clientId] = true;

    if (isReady)
    {
        // 다른 누군가가 이미 Ready 한 상태면 실패
        if (characterReady[characterId] != -1 && characterReady[characterId] != clientId)
            return false;

        // Ready 상태 등록
        characterReady[characterId] = clientId;
    }
    else
    {
        // Ready 해제는 자기 자신인 경우만 해제
        if (characterReady[characterId] == clientId)
            characterReady[characterId] = -1;
    }

    return true;
}

bool Lobby_Scene::IsAllReadyAndValid()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (active_client_num == 0) return false;

    int readyCount = 0;

    for (int i = 0; i < MaxPlayer; ++i)
    {
        if (characterReady[i] != -1)
            ++readyCount;
    }

    return (readyCount == active_client_num);
}

Scene_Type Lobby_Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    Change_Scene_Trigger = IsAllReadyAndValid();

    if (Change_Scene_Trigger)
    {
        for (int characterId = 0; characterId < MaxPlayer; ++characterId) // Save - Client ID + Model Index
        {
            int clientId = characterReady[characterId];
            if (clientId != -1)
            {
                player_model_list[clientId] = characterId;
            }
        }


        return Scene_Type::Board;
    }
    else
        return Scene_Type::None;

}



//======================================================
void Board_Scene::Init()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if(pirate_ship)
        pirate_ship->SetPosition(0.0f, 0.0f, 1000.0f);

    for (int i = 0; i < MaxPlayer; i++)
    {
        player_keyState[i] = 0;
        stage_select_state[i] = { -1, false };
    }
}

void Board_Scene::Update_Scene(float elapsedTime)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (!pirate_ship) return;


    int fwd = 0, back = 0, left = 0, right = 0;

    for (int i = 0; i < MaxPlayer; ++i)
    {
        int modelId = Scene::player_model_list[i];
        if (modelId == -1) continue; 

        int weight = (modelId == 0) ? 2 : 1;  // 모델 0번이면 영향력 2배

        int32_t key = player_keyState[i];
        if (key & INPUT_W) fwd += weight;
        if (key & INPUT_S) back += weight;
        if (key & INPUT_A) left += weight;
        if (key & INPUT_D) right += weight;
    }

    if (fwd)
        pirate_ship->MoveForward(100.0f * fwd);
    if (back)
        pirate_ship->MoveForward(-100.0f * back);
    if (left)
        pirate_ship->Add_Rotate(-100.0f * left);
    if (right)
        pirate_ship->Add_Rotate(100.0f * right);

    pirate_ship->Animate(elapsedTime);
    pirate_ship->HandleBoundaryReflection(1500);

    // 씬 전환 체크
    Change_Scene_Trigger = IsAllReadyAndValid();
}

void Board_Scene::Remove_Player(int id)
{
    if (id < 0 || id >= MaxPlayer)
        return;

    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    player_keyState[id] = 0;
    stage_select_state[id] = { -1, false };
}


bool Board_Scene::IsAllReadyAndValid()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (active_client_num == 0) return false;

    int readyCount = 0;
    int selectedStage = -1;

    for (int i = 0; i < MaxPlayer; ++i)
    {
        const auto& [stage, is_ready] = stage_select_state[i];

        if (!is_ready)
            continue;

        if (readyCount == 0)
        {
            selectedStage = stage; // 기준 stage 설정
        }
        else if (stage != selectedStage)
        {
            return false; // 서로 다른 stage 선택
        }

        ++readyCount;
    }

    return (readyCount == active_client_num);
}


Scene_Type Board_Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);


    if (Change_Scene_Trigger)
        return Scene_Type::Stage_1;

    return Scene_Type::None;
}

void Board_Scene::Update_KeyState(int Client_ID, int32_t keyState)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (Client_ID >= 0 && Client_ID < MaxPlayer)
        player_keyState[Client_ID] = keyState; 
    else
        cout << "Error - [Update_KeyState]: Wrong_Index \n";
    
}

void Board_Scene::Select_State(int Client_ID, pair<int, bool> select_state)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (Client_ID >= 0 && Client_ID < MaxPlayer)
    {
        stage_select_state[Client_ID] = select_state; 
    }
    else
        cout << "Error - [Select_State]: Wrong_Index \n";

}




XMFLOAT3 Board_Scene::Get_PirateShip_Position() const
{
    if (pirate_ship)
        return pirate_ship->GetPosition();
    return XMFLOAT3(0.0f, 0.0f, 0.0f); 
}

XMFLOAT3 Board_Scene::Get_PirateShip_Look() const
{
    if (pirate_ship)
        return pirate_ship->GetLook();

    return XMFLOAT3(0.0f, 0.0f, 1.0f); 
}

//======================================================

Stage_Scene::Stage_Scene() : Scene (Scene_Type::Stage_1)
{
    std::shared_ptr<GameObject>scene = std::make_shared<GameObject>();
    scene = GameObject::Load_Scene("Scene/Scene_Name.bin");


    game_world.Init();
    game_world.Load_Scene_Data(scene);
    Init();
}

void Stage_Scene::Init()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (shared_ptr<Player> player_ptr : player_list)
        player_ptr.reset();

    

}

void Stage_Scene::Update_Scene(float elapsedTime)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    for (shared_ptr<Player> player_ptr : player_list)
    {
        if (player_ptr)
            game_world.Update_Collision(player_ptr);
    }

    for (auto m : Monster_List) {
        auto con = m->GetSkinnedAnimationController();
        if (con) {
            con->AdvanceTime(elapsedTime, m.get());

        }
    }
}

Scene_Type Stage_Scene::CheckSceneTransition()
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (Change_Scene_Trigger)
        return Scene_Type::Stage_1;
    else
        return Scene_Type::None;

}


const std::array<std::shared_ptr<Player>, MaxPlayer> Stage_Scene::Get_PlayerList() const
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    return player_list;
}

std::shared_ptr<Player> Stage_Scene::Get_Player(int id)
{
    if (id < 0 || id >= MaxPlayer)
        return nullptr;

    std::lock_guard<std::recursive_mutex> lock(sceneMutex);



    return player_list[id];
}

void Stage_Scene::Add_Player(int id)
{
    if (id < 0 || id >= MaxPlayer)
        return;

    std::lock_guard<std::recursive_mutex> lock(sceneMutex);
    
    player_list[id] = make_shared<Player>(player_model_list[id]);
}

void Stage_Scene::Remove_Player(int id)
{
    if (id < 0 || id >= MaxPlayer)
        return;

    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    player_list[id].reset();
}

void Stage_Scene::update_player_keyinput(int id, uint32_t keystate)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (id < 0 || id >= MaxPlayer || !player_list[id])
        return;

    player_list[id]->key_input(keystate);
}


void Stage_Scene::update_player_LookV(int id, XMFLOAT3 new_lookV)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (id < 0 || id >= MaxPlayer || !player_list[id])
        return;

    player_list[id]->SetLook(new_lookV);
}

void Stage_Scene::update_player_State(int clientId, uint32_t inputFlags, const XMFLOAT3& position, const XMFLOAT3& lookDirection, const std::vector<Animation_Sync>& tracks, bool stateChanged)
{
    std::lock_guard<std::recursive_mutex> lock(sceneMutex);

    if (clientId < 0 || clientId >= MaxPlayer || !player_list[clientId])
        return;



    player_list[clientId]->SetPosition(position);
    player_list[clientId]->SetLook(lookDirection);

    
    //    player_list[clientId]->key_input(inputFlags);

    if (!tracks.empty())
    {
        player_list[clientId]->SetTrackInfoList(tracks);
        player_list[clientId]->SetStateChanged(stateChanged);
    }
}