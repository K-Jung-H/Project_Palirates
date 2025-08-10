#include "stdafx.h"
#include "server.h"

Server::Server(int port)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);

    for (int i = 0; i < 100; ++i)
        availableIds.push(i);

    scenes[Scene_Type::Lobby] = std::make_shared<Lobby_Scene>();
    scenes[Scene_Type::Board] = std::make_shared<Board_Scene>();
    scenes[Scene_Type::Stage_1] = std::make_shared<Stage_1_Scene>();
    scenes[Scene_Type::Stage_2] = std::make_shared<Stage_2_Scene>();
    scenes[Scene_Type::Stage_3] = std::make_shared<Stage_3_Scene>();
    scenes[Scene_Type::Stage_4] = std::make_shared<Stage_4_Scene>();
    scenes[Scene_Type::Stage_5] = std::make_shared<Stage_5_Scene>();
    scenes[Scene_Type::Stage_6] = std::make_shared<Stage_6_Scene>();
    scenes[Scene_Type::Stage_7] = std::make_shared<Stage_7_Scene>();

    activeScene = scenes[Scene_Type::Lobby];
}


Server::~Server()
{
    for (const auto& [id, session] : clients)
        closesocket(session->socket);
    closesocket(listenSocket);
    WSACleanup();
}


void Server::Start()
{

    std::thread([this]()
        {
            try
            {
                AcceptClients();
            }
            catch (const std::exception& e)
            {
                std::cerr << "[AcceptClients Thread EXCEPTION] " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "[AcceptClients Thread UNKNOWN EXCEPTION]" << std::endl;
            }
        }).detach();


    std::thread([this]()
        {
            try
            {
                Server_Update();
            }
            catch (const std::exception& e)
            {
                std::cerr << "[Server_Update Thread EXCEPTION] " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "[Server_Update Thread UNKNOWN EXCEPTION]" << std::endl;
            }
        }).detach();

    std::thread([this]()
        {
            try {
                while (true)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    CleanupInactiveClients();
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "[CleanupInactiveClients Thread EXCEPTION] " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "[CleanupInactiveClients Thread UNKNOWN EXCEPTION]" << std::endl;
            }
        }).detach();

    std::thread([this]()
        {
            while (true)
            {
                auto stage_scene = dynamic_pointer_cast<Stage_Scene>(activeScene);
                if (stage_scene)
                {
                    if (GetAsyncKeyState(VK_OEM_1) & 0x8000)  // ; key
                        stage_scene->server_DespawnMonster();
                    else if (GetAsyncKeyState(VK_OEM_7) & 0x8000)  // ' key
                        stage_scene->server_DespawnMonster_For_Clear();
                    else if (GetAsyncKeyState(VK_OEM_PERIOD) & 0x8000)  // . key
                        stage_scene->server_Fog_Control();
                    else if (GetAsyncKeyState(VK_OEM_2) & 0x8000)  // / key
                        stage_scene->server_Sand_Control();
                        //stage_scene->server_X_Ray_Control();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }).detach();
}


void Server::AcceptClients()
{
    while (true)
    {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) continue;

        int clientId = GetNewClientId();
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients[clientId] = std::make_shared<ClientSession>(clientSocket);
        }

        activeClientCount++;

        std::string idPacket = "CLIENT_ID," + std::to_string(clientId) + "\n";

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            Send_Custom(clients[clientId], idPacket, true);
        }

        std::thread(&Server::ProcessClientPackets, this, clientSocket, clientId).detach();
    }
}


void Server::ProcessClientPackets(SOCKET clientSocket, int clientId)
{
    char buffer[1024];
    while (true)
    {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0)
        {
            DisconnectClient(clientId);
            break;
        }

        buffer[bytesReceived] = '\0';


        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients[clientId]->lastActiveTime = std::chrono::steady_clock::now();


            std::lock_guard<std::mutex> logLock(clients[clientId]->packetLogMutex);
            clients[clientId]->lastReceivedPacket = buffer;
        }

        std::istringstream iss(buffer);
        std::string line;

        while (std::getline(iss, line))
        {
            std::istringstream linestream(line);
            std::string command;
            std::getline(linestream, command, ',');

            std::vector<std::string> tokens;
            tokens.push_back(command);
            std::string token;
            while (std::getline(linestream, token, ','))
                tokens.push_back(token);


            if (command == "PING")
            {
                HandlePingPacket(clientId, command, tokens);
            }
            else if (command == "FORCE_TO_CHANGE_SCENE" || command == "CHANGE_SCENE_READY")
            {
                HandleChangeScenePacket(clientId, command, tokens);
            }

            if (tokens.size() < 3) continue;

            int sceneTypeInt = std::stoi(tokens[1]);
            Scene_Type sceneType = static_cast<Scene_Type>(sceneTypeInt);

            clients[clientId]->client_scene_type = sceneType;

            switch (sceneType)
            {
            case Scene_Type::Lobby:
                HandleLobbyPacket(clientId, command, tokens);
                break;
            case Scene_Type::Board:
                HandleBoardPacket(clientId, command, tokens);
                break;
            case Scene_Type::Stage_1:
            case Scene_Type::Stage_2:
            case Scene_Type::Stage_3:
            case Scene_Type::Stage_4:
            case Scene_Type::Stage_5:
            case Scene_Type::Stage_6:
            case Scene_Type::Stage_7:
                HandleStagePacket(clientId, command, tokens);
                break;

            default:
                std::cerr << "[ERROR] Unknown scene type received: " << sceneTypeInt << std::endl;
                break;
            }
        }
    }

    activeClientCount--;

}

void Server::HandlePingPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{

}

void Server::HandleChangeScenePacket(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{
    if (command == "FORCE_TO_CHANGE_SCENE")
    {
        if (tokens.size() < 2) return;
        if (clientId != 0) return;
        int sceneTypeInt = std::stoi(tokens[1]);
        Scene_Type sceneType = static_cast<Scene_Type>(sceneTypeInt);

        Change_Scene_And_Init_Players(sceneType);

    }
    else if (command == "CHANGE_SCENE_READY") // 스테이지 클리어 후, 전환 신호 처리
    {
        auto active_scene = GetActiveScene();
        if (active_scene == nullptr)  return;
        /* auto stage_scene = dynamic_pointer_cast<Stage_Scene>(active_scene);
         if (stage_scene)
             stage_scene->Update_Clear_State(clientId, true);*/
        if (active_scene->GetSceneType() == Stage_1
            || Stage_2 || Stage_3 || Stage_4
            || Stage_5 || Stage_6 || Stage_7)
        {
            auto stage_scene = dynamic_pointer_cast<Stage_Scene>(active_scene);
            if (stage_scene)
                stage_scene->Update_Clear_State(clientId, true);
        }
    }
}


void Server::HandleLobbyPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 5)
    {
        std::cerr << "ERROR HandleLobbyPacket: 토큰 개수 부족" << std::endl;
        return;
    }

    // 토큰: [0] = "PLAYER_UPDATE", [1] = sceneType, [2] = clientId, [3] = selected_character_index, [4] = isReady
    int selected_character_index = std::stoi(tokens[3]);
    bool isReady = (tokens[4] == "1" || tokens[4] == "true");


    if (selected_character_index == -1)
        return;


    auto scene_It = scenes.find(Scene_Type::Lobby);
    if (scene_It == scenes.end()) return;

    std::shared_ptr<Scene> baseScene = scene_It->second;
    shared_ptr<Lobby_Scene> lobbyScene = dynamic_pointer_cast<Lobby_Scene>(baseScene);


    bool success = lobbyScene->SelectCharacter(clientId, selected_character_index, isReady);


    //====================================



    Scene_Type active_scene_type = activeScene->GetSceneType();
    bool diff_scene_player = clients[clientId]->client_scene_type != active_scene_type;  // 늦게 들어온 플레이어인 경우

    if (diff_scene_player)
    {
        Scene_Type new_scene_type = lobbyScene->CheckSceneTransition();
        if (new_scene_type != Scene_Type::None) // 씬 전환 조건이 충족 되면 해당 씬으로 전환하도록 할 것
        {
            std::lock_guard<std::mutex> lock(clientsMutex);

            std::string packet = "CHANGE_SCENE," + std::to_string(active_scene_type) + "\n";
            Send_Custom(clients[clientId], packet, true);

            if (active_scene_type == Scene_Type::Stage_1 || active_scene_type == Scene_Type::Stage_2
                || active_scene_type == Scene_Type::Stage_3 || active_scene_type == Scene_Type::Stage_4
                || active_scene_type == Scene_Type::Stage_5 || active_scene_type == Scene_Type::Stage_6
                || active_scene_type == Scene_Type::Stage_7)
            {
                std::shared_ptr<Stage_Scene> stage_scene = std::dynamic_pointer_cast<Stage_Scene>(activeScene);
                if (stage_scene)
                {
                    for (const auto& [id, session] : clients)
                    {
                        if (session->is_connected)
                            stage_scene->Add_Player(id);
                    }
                }
            }
        }
    }
}

void Server::HandleBoardPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 6)
    {
        std::cerr << " [ERROR] HandleBoardPacket: 토큰 개수 부족" << std::endl;
        return;
    }

    auto it = scenes.find(Scene_Type::Board);
    if (it == scenes.end()) return;

    std::shared_ptr<Scene> baseScene = it->second;
    std::shared_ptr<Board_Scene> boardScene = std::dynamic_pointer_cast<Board_Scene>(baseScene);
    if (!boardScene) return;


    uint32_t inputFlags = static_cast<uint32_t>(std::stoul(tokens[3]));
    float Selected_Stage = std::stoi(tokens[4]);
    bool is_Selected = (tokens[5] == "1" || tokens[5] == "true");



    boardScene->Update_KeyState(clientId, inputFlags);
    boardScene->Select_State(clientId, { Selected_Stage, is_Selected });

}

void Server::HandleStagePacket(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{
    int sceneTypeInt = std::stoi(tokens[1]);
    Scene_Type sceneType = static_cast<Scene_Type>(sceneTypeInt);

    auto active_scene = GetActiveScene();
    if (active_scene == nullptr)  return;
    if (active_scene->GetSceneType() != sceneType)  return;

    if (tokens.size() < 11) return;

    int trackCount = std::stoi(tokens[10]);

    // 기본(11) + 트랙데이터(3개씩) + bStateChange(1) + changedStateNum(1)
    int expectedMinTokens = 11 + (trackCount * 3) + 1 + 1;

    if (tokens.size() < expectedMinTokens)
        return;

    std::shared_ptr<Stage_Scene> stageScene = std::dynamic_pointer_cast<Stage_Scene>(active_scene);

    if (!stageScene)
        return;


    // inputFlags 처리
    uint32_t inputFlags = static_cast<uint32_t>(std::stoul(tokens[3]));

    // 위치 데이터
    XMFLOAT3 pos;
    pos.x = std::stof(tokens[4]);
    pos.y = std::stof(tokens[5]);
    pos.z = std::stof(tokens[6]);

    // 방향 데이터
    XMFLOAT3 look;
    look.x = std::stof(tokens[7]);
    look.y = std::stof(tokens[8]);
    look.z = std::stof(tokens[9]);

    // 애니메이션 트랙 데이터 시작 위치: 11
    std::vector<Animation_Sync> trackInfoList;
    for (int i = 0; i < trackCount; i++)
    {
        int track_base = 11 + (i * 3);

        Animation_Sync trackData;
        trackData.track_index = std::stoi(tokens[track_base]);
        trackData.weight = std::stof(tokens[track_base + 1]);
        trackData.track_position = std::stof(tokens[track_base + 2]);

        trackInfoList.push_back(trackData);
    }

    // bStateChange (마지막 토큰)
    bool bStateChange = (tokens[11 + (trackCount * 3)] == "1" || tokens[11 + (trackCount * 3)] == "true");

    int stateNum = stoi(tokens[11 + (trackCount * 3) + 1]);
    // Scene 업데이트 호출
    stageScene->update_player_State(clientId, inputFlags, pos, look, trackInfoList, bStateChange, stateNum);
}


void Server::Broadcast_Scene_State_All()
{
    if (!activeClientCount) return;

    std::unordered_map<Scene_Type, std::vector<std::shared_ptr<ClientSession>>> sceneClients;

    {
        std::lock_guard<std::mutex> lock(clientsMutex);

        // 1. 클라이언트를 씬 타입별로 분류
        for (auto& [clientId, session] : clients)
        {
            if (!session->is_connected) continue;
            sceneClients[session->client_scene_type].push_back(session);
        }
    }

    // 2. 씬 타입별로 필요한 경우에만 패킷 생성 + 송신
    for (auto& [sceneType, sessionList] : sceneClients)
    {
        if (sessionList.empty()) continue;

        std::string packet;
        if (!Build_Scene_Packet_By_Type(sceneType, packet))
            continue;

        for (auto& session : sessionList)
        {
            Send_Custom(session, packet, true);
        }
    }
}

bool Server::Build_Scene_Packet_By_Type(Scene_Type type, std::string& outPacket)
{
    auto it = scenes.find(type);
    if (it == scenes.end()) return false;

    std::shared_ptr<Scene> scene = it->second;
    if (!scene) return false;

    switch (type)
    {
    case Scene_Type::Lobby:
    {
        auto lobby = std::dynamic_pointer_cast<Lobby_Scene>(scene);
        if (!lobby) return false;

        outPacket = Build_LobbyScene_Packet(lobby);
        return true;
    }

    case Scene_Type::Board:
    {
        auto board = std::dynamic_pointer_cast<Board_Scene>(scene);
        if (!board) return false;

        outPacket = Build_BoardScene_Packet(board);
        return true;
    }

    case Scene_Type::Stage_1:
    case Scene_Type::Stage_2:
    case Scene_Type::Stage_3:
    case Scene_Type::Stage_4:
    case Scene_Type::Stage_5:
    case Scene_Type::Stage_6:
    case Scene_Type::Stage_7:
    {
        auto stage = std::dynamic_pointer_cast<Stage_Scene>(scene);
        if (!stage) return false;

        outPacket = Build_Stage_Scene_Packet(stage);
        return true;
    }



    default:
        return false;
    }
}

std::string Server::Build_LobbyScene_Packet(const std::shared_ptr<Lobby_Scene>& lobby)
{
    std::array<std::array<bool, MaxPlayer>, MaxPlayer> selections;
    std::array<int, MaxPlayer> readyStates;

    std::ostringstream oss;

    {
        std::lock_guard<std::recursive_mutex> lock(lobby->GetSceneMutex());
        selections = lobby->GetCharacterSelections();
        readyStates = lobby->GetCharacterReadyStates();
    }

    oss << "CHARACTER_SELECT_SCENE,";

    for (int charId = 0; charId < MaxPlayer; ++charId)
    {
        oss << std::to_string(charId) << "," << std::to_string(readyStates[charId]) << ",";

        bool hasSelection = false;
        for (int clientId = 0; clientId < MaxPlayer; ++clientId)
        {
            if (selections[charId][clientId])
            {
                if (hasSelection) oss << "|";
                oss << std::to_string(clientId);
                hasSelection = true;
            }
        }

        if (!hasSelection)
            oss << "-1";

        oss << ",";
    }

    std::string result = oss.str();
    if (!result.empty() && result.back() == ',')
        result.pop_back();

    result += "\n";

    return result;
}



std::string Server::Build_BoardScene_Packet(const std::shared_ptr<Board_Scene>& board)
{
    XMFLOAT3 pos;
    XMFLOAT3 look;

    std::lock_guard<std::recursive_mutex> lock(board->GetSceneMutex());
    {
        pos = board->Get_PirateShip_Position();
        look = board->Get_PirateShip_Look();
    }

    std::ostringstream oss;
    oss << "BOARD_SCENE,"
        << std::to_string(pos.x) << "," << std::to_string(pos.y) << "," << std::to_string(pos.z) << ","
        << std::to_string(look.x) << "," << std::to_string(look.y) << "," << std::to_string(look.z)
        << "\n";

    return oss.str();
}



std::string Server::Build_Stage_Scene_Packet(const std::shared_ptr<Stage_Scene>& stage)
{
    std::ostringstream oss;
    std::ostringstream players_data;

    int valid_player_count = 0;

    {
        std::lock_guard<std::recursive_mutex> lock(stage->GetSceneMutex());

        const auto& player_list = stage->Get_PlayerList();

        for (int id = 0; id < MaxPlayer; ++id)
        {
            const auto& player_ptr = player_list[id];
            if (!player_ptr)
                continue;

            ++valid_player_count;
            const XMFLOAT3 pos = player_ptr->GetPosition();
            const XMFLOAT3 look = player_ptr->GetLook();
            //const XMFLOAT3 look = player_ptr->CommandSetLook;

            const auto& anim_data = player_ptr->GetAnimationSyncData();
            const auto& track_list = anim_data.track_info_list;
            bool state_changed = anim_data.stateChanged || player_ptr->need_to_client_sync;
            int changedStateNum = anim_data.changedStateNum;
            float hp = player_ptr->GetHP();
            bool bBreathHit = player_ptr->BreathHit;

            players_data << std::to_string(id) << "," << std::to_string(Scene::player_model_list[id]) << ","
                << std::to_string(pos.x) << "," << std::to_string(pos.y) << "," << std::to_string(pos.z) << ","
                << std::to_string(look.x) << "," << std::to_string(look.y) << "," << std::to_string(look.z) << ","
                << std::to_string(track_list.size());

            /*if (!XMVector3Equal(XMLoadFloat3(&player_ptr->CommandSetLook), XMVectorZero())) {
                XMStoreFloat3(&player_ptr->CommandSetLook, XMVectorZero());
                cout << "cmd set look reset" << "\n";
            }*/

            for (const auto& track : track_list)
            {
                players_data << "," << std::to_string(track.track_index)
                    << "," << std::to_string(track.weight)
                    << "," << std::to_string(track.track_position);
            }

            players_data << "," << (state_changed ? "1" : "0");

            players_data << "," << changedStateNum;
            players_data << "," << hp;
            players_data << "," << bBreathHit << ",";
        }
    }

    std::string player_data_str = players_data.str();

    if (!player_data_str.empty() && player_data_str.back() == ',')
        player_data_str.pop_back();

    oss << "STAGE_1," << std::to_string(valid_player_count) << "," << player_data_str << "\n";

    //===================================================================

    const auto& monster_list = stage->GetMonsterList();

    if (!monster_list.empty())
    {
        std::lock_guard<std::recursive_mutex> lock(stage->GetSceneMutex());

        float list_size = monster_list.size();
        oss << "MONSTER_SNAPSHOT," << list_size;

        for (const auto& monster : monster_list) {
            if (!monster) continue;
            int mID = monster->GetID();
            oss << "," << mID;

            auto sync_Data = monster->MakeSyncData();
            oss << "," << sync_Data.position.x << "," << sync_Data.position.y << "," << sync_Data.position.z
                << "," << sync_Data.lookVector.x << "," << sync_Data.lookVector.y << "," << sync_Data.lookVector.z
                << "," << sync_Data.track_info_list.size();

            for (auto track_data : sync_Data.track_info_list)
            {
                oss << "," << to_string(track_data.track_index)
                    << "," << to_string(track_data.weight)
                    << "," << to_string(track_data.track_position);
            }
            oss << "," << sync_Data.stateChanged;
            oss << "," << sync_Data.hp;
        }

        oss << "\n";
    }

    const auto despawn_list = stage->FlushDespawnQueue();
    if (!despawn_list.empty()) {
        oss << "MONSTER_COMMAND," << despawn_list.size();
        for (int id : despawn_list) {
            oss << ",DESPAWN," << id;
        }
        oss << "\n";
    }

    const auto damage_list = stage->FlushDamageQueue();
    if (!damage_list.empty()) {
        oss << "MONSTER_COMMAND," << damage_list.size();
        for (MonsterHitInfo data : damage_list) {
            oss << ",HIT," << data.monsterID << "," << data.hitCmd;
        }
        oss << "\n";
    }

    const auto state_change_list = stage->FlushStateChangeQueue();
    if (!state_change_list.empty()) {
        oss << "P_S_CMD," << state_change_list.size();
        for (StateChangeInfo data : state_change_list) {
            oss << ",STATE_CHANGE," << data.ID << "," << data.stateNum;
            cout << "s change cmd send" << "\n";
        }
        oss << "\n";
    }

    //===================================================================

    const auto& particle_sync_data = stage->Get_Particle_Sync_Data();

    if (!particle_sync_data.created.empty())
    {
        std::ostringstream temp_p_create;
        temp_p_create << "PARTICLE_CREATE," << std::to_string(particle_sync_data.created.size()) << ",";

        for (const auto& obj : particle_sync_data.created)
        {
            UINT id = obj->Get_Particle_ID();
            XMFLOAT3 pos = obj->GetPosition();
            XMFLOAT3 look = obj->GetLook();
            Particle_Format fmt = obj->Get_Format();
            UINT type = static_cast<int>(fmt.particle_type);

            XMFLOAT3 area = fmt.area_xyz;
            XMFLOAT3 dir = fmt.main_direction;
            float life = fmt.lifetime;
            UINT status = obj->Get_Particle_Status();

            temp_p_create << std::to_string(id) << ","
                << std::to_string(type) << ","
                << std::to_string(pos.x) << "," << std::to_string(pos.y) << "," << std::to_string(pos.z) << ","
                << std::to_string(look.x) << "," << std::to_string(look.y) << "," << std::to_string(look.z) << ","
                << std::to_string(area.x) << "," << std::to_string(area.y) << "," << std::to_string(area.z) << ","
                << std::to_string(dir.x) << "," << std::to_string(dir.y) << "," << std::to_string(dir.z) << ","
                << std::to_string(life) << ","
                << std::to_string(status) << ",";
        }

        std::string line = temp_p_create.str();
        if (!line.empty() && line.back() == ',') line.pop_back();

        oss << line << "\n";
    }

    // ────────────────────────────────
    if (!particle_sync_data.pos_updated.empty())
    {
        std::ostringstream temp_p_update;
        temp_p_update << "PARTICLE_UPDATE," << std::to_string(particle_sync_data.pos_updated.size()) << ",";

        for (const auto& obj : particle_sync_data.pos_updated)
        {
            UINT id = obj->Get_Particle_ID();
            XMFLOAT3 pos = obj->GetPosition();
            XMFLOAT3 look = obj->GetLook();
            Particle_Format fmt = obj->Get_Format();
            UINT type = static_cast<int>(fmt.particle_type);

            XMFLOAT3 area = fmt.area_xyz;
            XMFLOAT3 dir = fmt.main_direction;
            float life = obj->Get_LifeTime();
            UINT status = obj->Get_Particle_Status();

            temp_p_update << std::to_string(id) << ","
                << std::to_string(type) << ","
                << std::to_string(pos.x) << "," << std::to_string(pos.y) << "," << std::to_string(pos.z) << ","
                << std::to_string(look.x) << "," << std::to_string(look.y) << "," << std::to_string(look.z) << ","
                << std::to_string(area.x) << "," << std::to_string(area.y) << "," << std::to_string(area.z) << ","
                << std::to_string(dir.x) << "," << std::to_string(dir.y) << "," << std::to_string(dir.z) << ","
                << std::to_string(life) << ","
                << std::to_string(status) << ",";
        }

        std::string line = temp_p_update.str();
        if (!line.empty() && line.back() == ',') line.pop_back();

        oss << line << "\n";
    }

    // ────────────────────────────────
    if (!particle_sync_data.removed.empty())
    {
        std::ostringstream temp_p_remove;
        temp_p_remove << "PARTICLE_REMOVE," << std::to_string(particle_sync_data.removed.size()) << ",";

        for (UINT id : particle_sync_data.removed)
            temp_p_remove << std::to_string(id) << ",";

        std::string line = temp_p_remove.str();
        if (!line.empty() && line.back() == ',') line.pop_back();

        oss << line << "\n";
    }

    //===================================================================
    {
        const auto& effect_status = stage->Get_Effect_Status();

        std::ostringstream temp_effect_status_data;
        temp_effect_status_data << "POST_EFFECT," << to_string(static_cast<int>(effect_status.motion_blur_active)) << ",";

        for (bool blur_active : effect_status.motion_blur_apply)
        {
            if (blur_active)
                temp_effect_status_data << "1" << ",";
            else
                temp_effect_status_data << "0" << ",";
        }

        temp_effect_status_data << std::to_string(static_cast<int>(effect_status.zoom_blur_active)) << ",";
        temp_effect_status_data << std::to_string(effect_status.zoom_w_position.x) << ",";
        temp_effect_status_data << std::to_string(effect_status.zoom_w_position.y) << ",";
        temp_effect_status_data << std::to_string(effect_status.zoom_w_position.z) << ",";
        temp_effect_status_data << std::to_string(static_cast<int>(effect_status.monster_x_ray)) << ",";
        temp_effect_status_data << std::to_string(static_cast<int>(effect_status.fog_trigger)) << ",";
        temp_effect_status_data << std::to_string(effect_status.fogStart) << ",";
        temp_effect_status_data << std::to_string(effect_status.fogEnd) << ",";
        temp_effect_status_data << std::to_string(effect_status.fogDensity);
        std::string line = temp_effect_status_data.str();

        oss << line << "\n";
    }

    if (stage->bStageClear) {
        oss << "STAGE_CLEAR," << 1;
        oss << "\n";
    }

    return oss.str();
}

void Server::Server_Update()
{
    m_gameTimer.Reset();
    float FPS = 300.0f;
    while (true)
    {
        m_gameTimer.Tick(FPS);
        float elapsedTime = m_gameTimer.GetTimeElapsed();

        Scene::active_client_num = activeClientCount;
        Check_Connected_Player();

        std::shared_ptr<Scene> scene = GetActiveScene();
        if (!scene) continue;


        scene->Update_Scene(elapsedTime);

        //==============================================
        // Handle Scene Chnage
        Scene_Type new_scene_type = scene->CheckSceneTransition();
        if (new_scene_type != Scene_Type::None)
        {
            Change_Scene_And_Init_Players(new_scene_type);
        }
        else
        {
            Broadcast_Scene_State_All();

            FlushSendQueues();
        }

        //PrintClientDebugInfo();
    }


}


void Server::Check_Connected_Player()
{
    if (activeClientCount == 0 && !serverResetDone)
    {
        serverResetDone = true;

        logger.Log("[INFO] No clients connected — resetting server state");

        {
            std::lock_guard<std::mutex> lock(activeSceneMutex);
            activeScene = scenes[Scene_Type::Lobby];
        }
    }
    else if (activeClientCount > 0 && serverResetDone)
    {
        serverResetDone = false;
    }
}


void Server::Change_Scene_And_Init_Players(Scene_Type new_scene_type)
{
    std::string packet = "CHANGE_SCENE," + std::to_string(static_cast<int>(new_scene_type)) + "\n";
    BroadcastPacket(packet);

    std::shared_ptr<Scene> new_scene;
    {
        std::lock_guard<std::mutex> lock(activeSceneMutex);
        auto it = scenes.find(new_scene_type);
        if (it != scenes.end())
        {
            activeScene = it->second;
            new_scene = activeScene;
        }
    }

    if (!new_scene)
        return;

    switch (new_scene_type)
    {
    case Scene_Type::Stage_1:
    case Scene_Type::Stage_2:
    case Scene_Type::Stage_3:
    case Scene_Type::Stage_4:
    case Scene_Type::Stage_5:
    case Scene_Type::Stage_6:
    case Scene_Type::Stage_7:
    {
        std::shared_ptr<Stage_Scene> stage_scene = std::dynamic_pointer_cast<Stage_Scene>(new_scene);
        if (stage_scene)
        {
            stage_scene->Init();

            std::lock_guard<std::mutex> lock(clientsMutex);
            for (const auto& [id, session] : clients)
            {
                if (session->is_connected)
                    stage_scene->Add_Player(id);
            }
 
        }
        break;
    }

    case Scene_Type::Board:
    {
        std::shared_ptr<Board_Scene> board_scene = std::dynamic_pointer_cast<Board_Scene>(new_scene);
        if (board_scene)
            board_scene->Init();
        break;
    }

    case Scene_Type::Lobby:

        break;

    default:

        logger.Log("[ERROR] Unknown scene type during ChangeSceneAndInitPlayers: " + std::to_string(static_cast<int>(new_scene_type)));
        break;
    }
}

void Server::CleanupInactiveClients()
{
    auto now = std::chrono::steady_clock::now();

    std::vector<int> toDisconnect;

    {
        std::lock_guard<std::mutex> lock(clientsMutex);

        for (const auto& [clientId, session] : clients)
        {
            if (!session->is_connected)
                continue;

            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - session->lastActiveTime);
            if (elapsed.count() > 10) // 10초 이상 무응답
            {
                logger.Log("[TIMEOUT] 클라이언트 " + std::to_string(clientId) + " 수신 없음 → 제거 대상");
                toDisconnect.push_back(clientId);
            }
        }
    }

    for (int clientId : toDisconnect)
    {
        DisconnectClient(clientId);
    }
}

void Server::DisconnectClient(int clientId)
{
    {
        std::lock_guard<std::mutex> lock(clientsMutex);

        removePlayerFromAllScenes(clientId);

        auto it = clients.find(clientId);
        if (it != clients.end())
        {
            closesocket(it->second->socket);
            clients.erase(it);
        }

        ReleaseClientId(clientId);
    }
    std::string disconnectPacket = "PLAYER_LEFT_GAME ," + std::to_string(clientId) + "\n";
    BroadcastPacket(disconnectPacket);
}

void Server::removePlayerFromAllScenes(int clientId)
{
    for (auto& [name, scene] : scenes)
        if (scene) scene->Remove_Player(clientId);
}

void Server::ReleaseClientId(int clientId)
{
    std::lock_guard<std::mutex> lock(idMutex);
    if (activeClientIds.erase(clientId))
        availableIds.push(clientId);
}


int Server::GetNewClientId()
{
    std::lock_guard<std::mutex> lock(idMutex);
    int id = availableIds.empty() ? nextClientId++ : availableIds.top();
    if (!availableIds.empty()) availableIds.pop();
    activeClientIds.insert(id);
    return id;
}



void Server::SetActiveScene(const Scene_Type scene_type)
{
    std::lock_guard<std::mutex> lock(activeSceneMutex);
    auto it = scenes.find(scene_type);
    if (it != scenes.end())
        activeScene = it->second;
}


std::shared_ptr<Scene> Server::GetActiveScene()
{
    std::lock_guard<std::mutex> lock(activeSceneMutex);
    return activeScene;
}

void Server::BroadcastPacket(const std::string& packet)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto& [id, session] : clients)
    {
        if (!session->is_connected) continue;
        Send_Custom(session, packet, true);
    }
}

void Server::Send_Custom(std::shared_ptr<ClientSession> session, const std::string& packet, bool saveLog)
{
    if (!session->is_connected) return;

    {
        std::lock_guard<std::mutex> lock(session->sendQueueMutex);
        session->sendQueue.push(packet);
    }

    if (saveLog)
    {
        std::lock_guard<std::mutex> logLock(session->packetLogMutex);
        session->lastSentPacket = packet;
    }
}


void Server::PrintClientDebugInfo()
{
//    system("cls");
    std::cout << "========= Server Frame Rate: " << m_gameTimer.GetFrameRate() << " FPS =========\n";


    if (activeClientCount == 0)
        return;


    std::lock_guard<std::mutex> lock(clientsMutex);


    for (const auto& [clientId, session] : clients)
    {
        if (!session->is_connected) continue;

        std::string recvLog;
        std::string sendLog;

        {
            std::lock_guard<std::mutex> logLock(session->packetLogMutex);
            recvLog = session->lastReceivedPacket;
            sendLog = session->lastSentPacket;
        }

        std::cout << "===============================================\n";
        std::cout << "Client ID - " << clientId << "\n";
        std::cout << "Receive Packet - " << recvLog << "\n";
        std::cout << "Send Packet    - " << sendLog << "\n";
    }

    std::cout << "===============================================\n\n";
}


void Server::FlushSendQueues()
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto& [clientId, session] : clients)
    {
        if (!session->is_connected) continue;

        std::lock_guard<std::mutex> sendLock(session->sendQueueMutex);
        while (!session->sendQueue.empty())
        {
            std::string packet = session->sendQueue.front();
            session->sendQueue.pop();

            send(session->socket, packet.c_str(), static_cast<int>(packet.length()), 0);

        }
    }
}

//========================================================================================

int main()
{
    Server server(9000);
    server.Start();

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}