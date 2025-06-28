#include "stdafx.h"
#include "server.h"

const int MAX_CLIENTS = 6;

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
            std::cout << buffer << std::endl;
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
}



void Server::HandleLobbyPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 5)
    {
        std::cerr << "[ERROR] HandleLobbyPacket: 토큰 개수 부족" << std::endl;
        return;
    }

    int selected_character_index = std::stoi(tokens[3]);

    bool character_select_status = false;
    const std::string& statusStr = tokens[4];
    if (statusStr == "1" || statusStr == "true" || statusStr == "TRUE")
    {
        character_select_status = true;
    }

    // 디버깅 로그
    std::cout << "[LOBBY] clientId=" << clientId
        << " 선택 캐릭터=" << selected_character_index
        << " 준비 상태=" << (character_select_status ? "true" : "false") << std::endl;

    // 예시: LobbyScene에 저장
    auto lobbyIt = scenes.find(Scene_Type::Lobby);
    if (lobbyIt == scenes.end()) return;

    auto lobbyScene = std::dynamic_pointer_cast<Lobby_Scene>(lobbyIt->second);
    if (!lobbyScene) return;

    bool success = lobbyScene->SelectCharacter(clientId, selected_character_index, character_select_status);

    // 응답
    std::string response = success
        ? ("CHARACTER_SELECT_SUCCESS," + std::to_string(selected_character_index) + "," + (character_select_status ? "true" : "false") + "\n")
        : ("CHARACTER_SELECT_FAIL," + std::to_string(selected_character_index) + "\n");

    send(clients[clientId].socket, response.c_str(), static_cast<int>(response.length()), 0);
}

void Server::HandleBoardPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{
    // 구현 필요
}


void Server::HandleStage1Packet(int clientId, const std::string& command, const std::vector<std::string>& tokens)
{
    // 구현 필요
}



void Server::BroadcastAllStates()
{
    HandleSceneBroadcast();
}


void Server::HandleSceneBroadcast()
{
    auto scene = GetActiveScene();
    if (!scene) return;

    switch (scene->GetSceneType())
    {
    case Scene_Type::Lobby:
        BroadcastLobbyScene(scene);
        break;
    case Scene_Type::Stage_1:
        BroadcastStage1Scene(scene);
        break;
    case Scene_Type::Stage_2:
        BroadcastStage2Scene(scene);
        break;
    case Scene_Type::Board:
        BroadcastBoardScene(scene);
        break;
    default:
        break;
    }
}


void Server::BroadcastLobbyScene(const std::shared_ptr<Scene>& scene)
{
    // 구현 필요
}


void Server::BroadcastStage1Scene(const std::shared_ptr<Scene>& scene)
{
    // 구현 필요
}


void Server::BroadcastStage2Scene(const std::shared_ptr<Scene>& scene)
{
    // 구현 필요
}


void Server::BroadcastBoardScene(const std::shared_ptr<Scene>& scene)
{
    // 구현 필요
}


void Server::Server_Update()
{
    while (true)
    {
        std::shared_ptr<Scene> scene = GetActiveScene();
        if (!scene) return;

        scene->Update();



        //==============================================
        // Change_Scene

        Scene_Type new_scene_type = scene->CheckSceneTransition();
        if (new_scene_type != Scene_Type::None)
        {
            std::string packet = "CHANGE_SCENE," + std::to_string(new_scene_type) + "\n";

            BroadcastPacket(packet, -1);
            SetActiveScene(new_scene_type);
        }
        else // Default
        {
            BroadcastAllStates();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));  
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


void Server::addPlayerToScene(const Scene_Type scene_type, int clientId, std::shared_ptr<Player> player)
{
    auto it = scenes.find(scene_type);
    if (it != scenes.end())
        it->second->addPlayer(clientId, player);
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