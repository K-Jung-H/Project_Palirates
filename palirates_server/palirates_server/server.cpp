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
    scenes[Scene_Type::Stage_1] = std::make_shared<Stage_Scene>();
    scenes[Scene_Type::Stage_2] = std::make_shared<Stage_Scene>();

    activeScene = scenes[Scene_Type::Lobby];
}


Server::~Server()
{
    for (const auto& [id, session] : clients)
        closesocket(session.socket);
    closesocket(listenSocket);
    WSACleanup();
}


void Server::Start()
{
    std::thread(&Server::AcceptClients, this).detach();
    std::thread(&Server::Server_Update, this).detach();

    std::thread([this]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            CleanupInactiveClients();
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
            clients[clientId] = { clientSocket, true, std::chrono::steady_clock::now() };
        }

        activeClientCount++; // 현재 활성화된 클라 스레드 개수

        std::string idPacket = "CLIENT_ID," + std::to_string(clientId) + "\n";
        send(clientSocket, idPacket.c_str(), static_cast<int>(idPacket.length()), 0);

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

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients[clientId].lastActiveTime = std::chrono::steady_clock::now();
           // std::cout << buffer << std::endl;
        }

        buffer[bytesReceived] = '\0';
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

            // 씬 타입 추출 (tokens[1]은 scene_type int로 가정)
            if (tokens.size() < 3) continue;

            int sceneTypeInt = std::stoi(tokens[1]);
            Scene_Type sceneType = static_cast<Scene_Type>(sceneTypeInt);

            switch (sceneType)
            {
            case Scene_Type::Lobby:
                HandleLobbyPacket(clientId, command, tokens);
                break;
            case Scene_Type::Board:
                HandleBoardPacket(clientId, command, tokens);
                break;
            case Scene_Type::Stage_1:
                HandleStage1Packet(clientId, command, tokens);
                break;
            case Scene_Type::Stage_2:
                break;
            default:
                std::cerr << "[ERROR] Unknown scene type received: " << sceneTypeInt << std::endl;
                break;
            }
        }
    }

    activeClientCount--; // 현재 활성화된 클라 스레드 개수

}



void Server::HandleLobbyPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 5)
    {
        std::cerr << "[ERROR] HandleLobbyPacket: 토큰 개수 부족" << std::endl;
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

    std::string response = success
        ? ("CHARACTER_SELECT_SUCCESS," + std::to_string(selected_character_index) + "," + (isReady ? "true" : "false") + "\n")
        : ("CHARACTER_SELECT_FAIL," + std::to_string(selected_character_index) + "\n");


    send(clients[clientId].socket, response.c_str(), static_cast<int>(response.length()), 0);
}

void Server::HandleBoardPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 6)
    {
        std::cerr << "[ERROR] HandleBoardPacket: 토큰 개수 부족" << std::endl;
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
    boardScene->Select_State(clientId, {Selected_Stage, is_Selected});

}

void Server::HandleStage1Packet(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{
    // 구현 필요
}


void Server::BroadcastAllStates()
{
    if (!activeClientCount)
        return;

    std::string packet;
    if (HandleSceneBroadcast(packet))
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (const auto& [clientId, session] : clients)
        {
            if (session.is_connected)
            {
                send(session.socket, packet.c_str(), static_cast<int>(packet.size()), 0);
            }
        }
    }
}


bool Server::HandleSceneBroadcast(std::string& outPacket)
{
    auto scene = GetActiveScene();
    if (!scene) return false;

    switch (scene->GetSceneType())
    {
    case Scene_Type::Lobby:
    {
        auto lobby = std::dynamic_pointer_cast<Lobby_Scene>(scene);
        if (!lobby) return false;

        outPacket = Build_LobbyScene_Packet(lobby);
        return true;
    }   break;

    case Scene_Type::Board:
    {
        auto board = std::dynamic_pointer_cast<Board_Scene>(scene);
        if (!board) return false;

        outPacket = Build_BoardScene_Packet(board);
        return true;
    }   break;

    case Scene_Type::Stage_1:
    {
        auto stage_1 = std::dynamic_pointer_cast<Stage_Scene>(scene);
        if (!stage_1) return false;

        outPacket = Build_Stage_1_Scene_Packet(stage_1);
        return true;
    }   break;

    case Scene_Type::Stage_2:
    {
        auto stage_2 = std::dynamic_pointer_cast<Stage_Scene>(scene);
        if (!stage_2) return false;

        outPacket = Build_Stage_2_Scene_Packet(stage_2);
        return true;
    }   break;

    default:
        return false;
    }
}

std::string Server::Build_LobbyScene_Packet(const std::shared_ptr<Lobby_Scene>& lobby)
{
    std::ostringstream oss;
    oss << "CHARACTER_SELECT_SCENE,";

    const auto& selections = lobby->GetCharacterSelections();
    const auto& readyStates = lobby->GetCharacterReadyStates();

    for (int charId = 0; charId < MaxPlayer; ++charId)
    {
        oss << charId << "," << readyStates[charId] << ",";

        bool hasSelection = false;
        for (int clientId = 0; clientId < MaxPlayer; ++clientId)
        {
            if (selections[charId][clientId])
            {
                if (hasSelection) oss << "|";
                oss << clientId;
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

    //std::cout << "[SERVER] Build_LobbyScene_Packet: " << result;
    return result;
}



std::string Server::Build_BoardScene_Packet(const std::shared_ptr<Board_Scene>& board)
{
    XMFLOAT3 pos = board->Get_PirateShip_Position();
    XMFLOAT3 look = board->Get_PirateShip_Look();

    std::ostringstream oss;
    oss << "BOARD_SCENE," << pos.x << "," << pos.y << "," << pos.z << "," << look.x << "," << look.y << "," << look.z << "\n";

    return oss.str();
}



std::string Server::Build_Stage_1_Scene_Packet(const std::shared_ptr<Stage_Scene>& stage)
{
    std::ostringstream oss;
    oss << "STAGE_1,";
    {

    }
    oss << "\n";
    return oss.str();
}

std::string Server::Build_Stage_2_Scene_Packet(const std::shared_ptr<Stage_Scene>& stage)
{
    std::ostringstream oss;
    oss << "STAGE_2,";
    {

    }
    oss << "\n";
    return oss.str();
}



void Server::Server_Update()
{
    CGameTimer gameTimer;
    gameTimer.Reset();  
    float FPS = 60.0f;
    while (true)
    {
        gameTimer.Tick(FPS);
        float elapsedTime = gameTimer.GetTimeElapsed(); 

        Scene::active_client_num = activeClientCount;

        std::shared_ptr<Scene> scene = GetActiveScene();
        if (!scene) return;


        scene->Update_Scene(elapsedTime); 

        //==============================================
        // Handle Scene Chnage
        Scene_Type new_scene_type = scene->CheckSceneTransition();
        if (new_scene_type != Scene_Type::None)
        {
            std::string packet = "CHANGE_SCENE," + std::to_string(new_scene_type) + "\n";
            BroadcastPacket(packet, -1);
            SetActiveScene(new_scene_type);
        }
        else
        {
            BroadcastAllStates();
        }


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
            if (!session.is_connected)
                continue;

            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - session.lastActiveTime);
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
    std::lock_guard<std::mutex> lock(clientsMutex);
    removePlayerFromAllScenes(clientId);
    auto it = clients.find(clientId);
    if (it != clients.end())
    {
        closesocket(it->second.socket);
        clients.erase(it);
    }
    ReleaseClientId(clientId);
}


void Server::ReleaseClientId(int clientId)
{
    std::lock_guard<std::mutex> lock(idMutex);
    if (activeClientIds.erase(clientId))
        availableIds.push(clientId);

    auto it = scenes.find(Scene_Type::Lobby);
    if (it == scenes.end()) return;              

    std::shared_ptr<Scene> baseScene = it->second;

    auto lobbyScene = std::dynamic_pointer_cast<Lobby_Scene>(baseScene);
    if (!lobbyScene) return;                    
    auto SelectArray = lobbyScene->GetCharacterSelections();
    std::cout << "셀렉 어레이 : " << std::endl;
    for (int i = 0; i < MaxPlayer; ++i)
    {
        std::cout << SelectArray[i][clientId] << std::endl;
        if (SelectArray[i][clientId]) {
            SelectArray[i][clientId] = false;
            std::cout << "셀렉 클라 아이디 찾음 : " << SelectArray[i][clientId] << std::endl;
        }
    }
    auto ReadyArray = lobbyScene->GetCharacterReadyStates();
    if (ReadyArray[clientId]) {
        ReadyArray[clientId] = -1;
        std::cout << "레디 어레이 : " << std::endl;
        for (int i = 0; i < MaxPlayer; ++i) {
            std::cout <<  ReadyArray[i] << std::endl;
        }
        std::cout << "레디 클라 아이디 찾음 : " << ReadyArray[clientId] << std::endl;
    }
}


int Server::GetNewClientId()
{
    std::lock_guard<std::mutex> lock(idMutex);
    int id = availableIds.empty() ? nextClientId++ : availableIds.top();
    if (!availableIds.empty()) availableIds.pop();
    activeClientIds.insert(id);
    return id;
}


void Server::BroadcastPacket(const std::string& packet, int senderId)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (const auto& [id, session] : clients)
    {
        if (!session.is_connected || id == senderId) continue;
        send(session.socket, packet.c_str(), static_cast<int>(packet.length()), 0);
    }
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




void Server::removePlayerFromAllScenes(int clientId)
{
    for (auto& [name, scene] : scenes)
        if (scene) scene->removePlayer(clientId);
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