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

    for (int i = 0; i < 6; ++i) 
    {
        availableIds.push(i);
    }

    isCharacterAvailable.resize(6, true);
    hoveredByClient.resize(6, -1);
    selectedCharacterByClient.resize(6, -1);
}

void Server::AcceptClients()
{
    while (true)
    {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);


        if (clientSocket != INVALID_SOCKET)
        {
            int clientId = GetNewClientId();

            if (clientId == -1)
            {
                closesocket(clientSocket);
                std::cout << "최대인원초과, 연결 거부.\n";
                continue;
            }

            ClientSession session;
            session.socket = clientSocket;
            session.is_connected = true;
            session.lastPongTime = std::chrono::steady_clock::now();

            clients[clientId] = session;

            if (clientId >= hoveredByClient.size())
                hoveredByClient.resize(clientId + 1, -1);
            if (clientId >= selectedCharacterByClient.size())
                selectedCharacterByClient.resize(clientId + 1, -1);


            sceneManager.removeScene(clientId);
            sceneManager.addScene(clientId);
            std::shared_ptr<Scene> scene = sceneManager.getScene(clientId);

            char sendBuffer[256];
            sprintf_s(sendBuffer, "CLIENT_ID,%d", clientId);
            logger.Log("클라이언트 " + std::to_string(clientId) + " 연결됨.");

            scene->addPlayer(clientId, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 1.0f });

            int retval = send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
            if (retval == SOCKET_ERROR)
            {
                logger.Log("[서버] CLIENT_ID 전송 실패! 에러 코드: " + std::to_string(WSAGetLastError()));
            }
            else
            {
                logger.Log("[서버] CLIENT_ID 전송 성공! 보낸 데이터: " + std::string(sendBuffer));
            }

            try
            {
                std::thread(&Server::ProcessClientPackets, this, clientSocket, clientId).detach();
            }
            catch (const std::system_error& e)
            {
                logger.Log("[에러] 클라이언트 패킷 처리 스레드 생성 실패: " + std::string(e.what()));
            }
        }

    }
}

void Server::Server_Update()
{
    // 주기적 서버 갱신 로직 작성 가능
}


void Server::ProcessClientPackets(SOCKET clientSocket, int clientId)
{
    try {
        char buffer[1024];
        std::string recvBuffer;

        while (true)
        {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

            if (bytesReceived <= 0)
            {
                logger.Log("Client " + std::to_string(clientId) + " disconnected or recv failed");
                DisconnectClient(clientId);
                break;
            }

            buffer[bytesReceived] = '\0';
            recvBuffer += buffer;

            size_t pos;
            while ((pos = recvBuffer.find('\n')) != std::string::npos)
            {
                std::string packet = recvBuffer.substr(0, pos);
                recvBuffer.erase(0, pos + 1);

                std::vector<std::string> tokens;
                std::stringstream ss(packet);
                std::string token;

                while (std::getline(ss, token, ','))
                    tokens.push_back(token);

                if (tokens.empty()) continue;

                const std::string& cmd = tokens[0];

                if (cmd == "PLAYER_UPDATE")
                {
                    if (tokens.size() < 9)
                    {
                        continue;
                    }
                    int id = std::stoi(tokens[1]);
                    uint32_t keyMask = static_cast<uint32_t>(std::stoi(tokens[2]));
                    float lookX = std::stof(tokens[3]);
                    float lookY = std::stof(tokens[4]);
                    float lookZ = std::stof(tokens[5]);
                    int state = std::stoi(tokens[6]);
                    int trackCount = std::stoi(tokens[7]);

                    std::vector<float> trackPositions;
                    std::vector<float> trackWeights;
                    for (int i = 0; i < trackCount; ++i)
                    {
                        int base = 8 + i * 2;
                        if (base + 1 >= (int)tokens.size()) break;
                        trackPositions.push_back(std::stof(tokens[base]));
                        trackWeights.push_back(std::stof(tokens[base + 1]));
                    }

                    auto scene = sceneManager.getScene(clientId);
                    if (!scene) {
                        sceneManager.addScene(clientId);
                        scene = sceneManager.getScene(clientId);
                    }
                    scene->update_player_keyinput(id, keyMask);
                    scene->updatePlayerAnimation(clientId, trackPositions, trackWeights);

                    std::ostringstream oss;
                    oss << "PLAYER_UPDATE," << clientId << ","
                        << 0 << "," << 10 << "," << 0 << ","
                        << lookX << "," << lookY << "," << lookZ << ","
                        << state << "," << trackCount;
                    for (int i = 0; i < trackCount; ++i)
                        oss << "," << trackPositions[i] << "," << trackWeights[i];
                    oss << "\n";

                    BroadcastPacket(oss.str(), clientId);
                }
                else if (cmd == "ENTER_SCENE")
                {
                    if (tokens.size() < 2) continue;
                    std::string sceneName = tokens[1];
                    auto scene = sceneManager.getScene(clientId);
                    if (!scene)
                    {
                        sceneManager.addScene(clientId);
                        scene = sceneManager.getScene(clientId);
                    }
                    if (sceneName == "Character_Select")
                        scene->SetSceneType(Scene_Type::Lobby);
                    else if (sceneName == "Game_Stage_Board")
                        scene->SetSceneType(Scene_Type::Board);
                    else
                        scene->SetSceneType(Scene_Type::None);

                    logger.Log("[ENTER_SCENE] Client " + std::to_string(clientId) + " → " + sceneName);
                }
                else if (
                    (cmd == "CHARACTER_SELECT" || cmd == "CHARACTER_SELECT_REQUEST") && tokens.size() >= 3)
                {
                    int selClientId = std::stoi(tokens[1]);
                    int selectedCharId = std::stoi(tokens[2]);

                    if (!clients[selClientId].is_connected)
                        return;

                    if (lockedCharacterIds.find(selectedCharId) != lockedCharacterIds.end())
                    {
                        std::string rejectMsg = "CHARACTER_SELECT_DENIED\n";
                        send(clientSocket, rejectMsg.c_str(), (int)rejectMsg.length(), 0);
                        logger.Log("[DENIED] Character " + std::to_string(selectedCharId) + " already taken.");
                        return;
                    }

                    characterSelections[selClientId] = selectedCharId;
                    lockedCharacterIds.insert(selectedCharId);

                    std::string approveMsg = "CHARACTER_SELECT_APPROVED\n";
                    send(clientSocket, approveMsg.c_str(), (int)approveMsg.length(), 0);
                    logger.Log("[APPROVED] Character " + std::to_string(selectedCharId) + " assigned to client " + std::to_string(selClientId));

                    std::string statusMsg = "CHARACTER_STATUS," + std::to_string(selClientId) + "," + std::to_string(selectedCharId) + "\n";
                    BroadcastPacket(statusMsg, -1);

                    auto scene = sceneManager.getScene(clientId);
                    if (!scene)
                    {
                        sceneManager.addScene(clientId);
                        scene = sceneManager.getScene(clientId);
                    }
                    if (scene)
                    {
                        scene->SetSceneType(Scene_Type::Lobby);
                        scene->addPlayer(clientId, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

                        NotifyExistingPlayersAboutNew(clientId);
                        SendInitialStates(clientId);

                        int controllerId = GetControllerId(scene);
                        controllerIdByScene[clientId] = controllerId;

                        if (controllerId != -1)
                        {
                            std::string ctrlPacket = "SHIP_CONTROLLER_ID," + std::to_string(controllerId) + "\n";
                            BroadcastPacket(ctrlPacket, -1);
                        }
                        else
                        {
                            logger.Log("[WARN] ControllerId -1 returned for client " + std::to_string(clientId));
                        }
                    }
                }
                else if (cmd == "SHIP_SYNC")
                {
                    if (tokens.size() < 7) continue;

                    float shipX = std::stof(tokens[1]);
                    float shipY = std::stof(tokens[2]);
                    float shipZ = std::stof(tokens[3]);
                    float shipLookX = std::stof(tokens[4]);
                    float shipLookY = std::stof(tokens[5]);
                    float shipLookZ = std::stof(tokens[6]);

                    std::unordered_map<std::string, std::string> extraFields = ParseKeyValueFields(tokens, 7);

                    for (const auto& [key, value] : extraFields)
                        logger.Log("Extra Field: " + key + " = " + value);

                    std::ostringstream oss;
                    oss << "SHIP_SYNC," << shipX << "," << shipY << "," << shipZ
                        << "," << shipLookX << "," << shipLookY << "," << shipLookZ;

                    for (const auto& [key, value] : extraFields)
                        oss << "," << key << "=" << value;

                    oss << "\n";
                    BroadcastPacket(oss.str(), clientId);
                }
                else if (cmd == "PLAYER_LEAVE")
                {
                    logger.Log("Client " + std::to_string(clientId) + " left");
                    DisconnectClient(clientId);
                    return;
                }
                else if (cmd == "PING")
                {
                    send(clientSocket, "PONG\n", 5, 0);
                }
                else if (cmd == "PONG")
                {
                    clients[clientId].lastPongTime = std::chrono::steady_clock::now();
                }
                else if (cmd == "KEY_INPUT")
                {
                    if (tokens.size() < 2) continue;

                    int keyMask = std::stoi(tokens[1]);

                    auto scene = sceneManager.getScene(clientId);
                    if (!scene)
                    {
                        sceneManager.addScene(clientId);
                        scene = sceneManager.getScene(clientId);
                    }

                    std::shared_ptr<Player> player = scene->getPlayer(clientId);
                    if (!player) return;

                    player->key_input(keyMask);

                    std::cout << "test Value, ID : " << clientId << " value : " << player->test_value << "\n";
                }
                else
                {
                    logger.Log("Invalid packet format received: " + packet);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        logger.Log("[EXCEPTION] Client " + std::to_string(clientId) + " processing error: " + e.what());
        DisconnectClient(clientId);
    }
}


void Server::BroadcastPacket(const std::string& packet, int senderId)
{
    std::string finalizedPacket = packet;
    if (!finalizedPacket.empty() && finalizedPacket.back() != '\n')
        finalizedPacket += '\n';

    for (const auto& [id, session] : clients)
    {
        if (!session.is_connected || id == senderId) continue;

        int bytesSent = send(session.socket, finalizedPacket.c_str(), (int)finalizedPacket.length(), 0);
        if (bytesSent == SOCKET_ERROR)
            logger.Log("[ERROR] 클라이언트 " + std::to_string(id) + "에게 send 실패");
    }
}


void Server::BroadcastAllStates()
{
    for (const auto& [clientId, scene] : sceneManager.getAllScenes())
    {
        for (const auto& [playerId, player] : scene->getPlayers())
        {
            XMFLOAT3 player_look = player->GetLook();
            XMFLOAT3 player_pos = player->GetPosition();

            float safeLookY = (player_look.y == 0.0f) ? 1.0f : player_look.y;

 //           std::string packet = "PLAYER_UPDATE," + std::to_string(playerId) + "," +
 //               std::to_string(player_pos.x) + "," + std::to_string(player_pos.y) + "," + std::to_string(player_pos.z) + "," +
 //               std::to_string(player_look.x) + "," + std::to_string(safeLookY) + "," + std::to_string(player_look.z) + "," +
 //               std::to_string(static_cast<int>(player->GetState())) + "\n";
 //
 //           BroadcastPacket(packet, -1); // 전체 클라이언트에게 전송
        }
    }
}

void Server::SendInitialStates(int clientId)
{
    auto clientIt = clients.find(clientId);
    if (clientIt == clients.end() || !clientIt->second.is_connected) return;

    std::shared_ptr<Scene> myScene = sceneManager.getScene(clientId);
    if (!myScene) return;

    for (const auto& [otherId, scene] : sceneManager.getAllScenes())
    {
        if (otherId == clientId) continue;
        if (!scene) continue;

        auto character = scene->getPlayer(otherId);
        if (!character) continue;

        XMFLOAT3 look = character->GetLook();
        XMFLOAT3 pos = character->GetPosition();
        float safeLookY = (look.y == 0.0f) ? 1.0f : look.y;

        std::string createPacket = "PLAYER_CREATE," + std::to_string(otherId) + "\n";
        std::string updatePacket = "PLAYER_UPDATE," + std::to_string(otherId) + "," +
            std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z) + "," +
            std::to_string(look.x) + "," + std::to_string(safeLookY) + "," + std::to_string(look.z) + "," +
            std::to_string(static_cast<int>(character->GetState())) + "\n";

        send(clientIt->second.socket, createPacket.c_str(), createPacket.length(), 0);
        send(clientIt->second.socket, updatePacket.c_str(), updatePacket.length(), 0);

        auto charIt = characterSelections.find(otherId);
        if (charIt != characterSelections.end())
        {
            std::string charStatus = "CHARACTER_STATUS," + std::to_string(otherId) + "," + std::to_string(charIt->second) + "\n";
            send(clientIt->second.socket, charStatus.c_str(), charStatus.length(), 0);
        }
    }
}

void Server::NotifyExistingPlayersAboutNew(int newClientId)
{
    std::shared_ptr<Scene> scene = sceneManager.getScene(newClientId);
    if (!scene) return;

    auto character = scene->getPlayer(newClientId);
    if (!character)
    {
        logger.Log("[NotifyExistingPlayersAboutNew] scene->getPlayer(" + std::to_string(newClientId) + ") 실패");
        return;
    }

    XMFLOAT3 look = character->GetLook();
    XMFLOAT3 pos = character->GetPosition();
    float safeLookY = (look.y == 0.0f) ? 1.0f : look.y;

    std::string createPacket = "PLAYER_CREATE," + std::to_string(newClientId) + "\n";
    std::string updatePacket = "PLAYER_UPDATE," + std::to_string(newClientId) + "," +
        std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z) + "," +
        std::to_string(look.x) + "," + std::to_string(safeLookY) + "," + std::to_string(look.z) + "," +
        std::to_string(static_cast<int>(character->GetState())) + "\n";

    int charId = -1;
    auto it = characterSelections.find(newClientId);
    if (it != characterSelections.end())
        charId = it->second;

    for (const auto& [clientId, session] : clients)
    {
        if (clientId == newClientId || !session.is_connected) continue;

        send(session.socket, createPacket.c_str(), createPacket.length(), 0);
        send(session.socket, updatePacket.c_str(), updatePacket.length(), 0);

        if (charId != -1)
        {
            std::string charStatus = "CHARACTER_STATUS," + std::to_string(newClientId) + "," + std::to_string(charId) + "\n";
            send(session.socket, charStatus.c_str(), charStatus.length(), 0);
        }
    }

    //logger.Log("[서버] 기존 유저들에게 신규 클라이언트 " + std::to_string(newClientId) + " 상태 전송 완료");
}




Server::~Server()
{
    for (const auto& [id, socket] : clients)
    {
        closesocket(socket.socket);
    }

    closesocket(listenSocket);
    WSACleanup();
}

void Server::Start()
{
    std::thread(&Server::AcceptClients, this).detach();

    if (!sceneManager.getScene(0))
    {
        sceneManager.addScene(0);
    }

    shared_ptr<Scene> scene = sceneManager.getScene(0);
    if (!scene) return;

   // for (int i = 0; i < 10; ++i)
   // {
   //     std::cout << "몬스터 생성됨" << std::endl;
   //     int id = i + 100;
   //     float x = 10 * i;
   //     float y = 0.0f;
   //     float z = 5 * i;
   //     float lookX = 0.0f, lookY = 1.0f, lookZ = 0.0f;
   //     int hp = 100;
   //     int state = 0;
   //     Monster_Type type = static_cast<Monster_Type>(0);
   //
   //     scene->addMonster(id, x, y, z, lookX, lookY, lookZ, hp, state, type);
   // }
    //BroadcastAllStates();

}

int Server::GetControllerId(shared_ptr<Scene> scene)
{
    const auto& players = scene->getPlayers();
    if (players.empty()) return -1;

    int minId = INT_MAX;
    for (const auto& [id, player] : players)
    {
        if (id < minId)
            minId = id;
    }

    return minId;
}


//================================================================================
// 패킷 문자열 토큰 중 "key=value" 형태의 임의 필드를 파싱하는 유틸 함수
// tokens     ','로 나눈 전체 패킷 문자열 리스트
// startIndex key=value 필드를 읽기 시작할 시작 인덱스
// unordered_map<string, string> 형태로 파싱된 확장 필드(key-value 쌍)
//
// @details
// 패킷 내에서 기본 위치, 방향 등의 필드 외에 확장 가능한 데이터를 유연하게 전달하기 위해
// 사용되는 함수. 예를 들어 다음과 같은 패킷:
//
//   SHIP_SYNC,10,20,30,0,1,0,speed=3.5,boost=true
//
// 이 있을 경우, 앞의 7개 필드는 고정 필드로 처리하고
// 이후의 "speed=3.5", "boost=true" 같은 확장 필드를 모두 파싱해
// unordered_map 으로 반환한다.
//================================================================================

std::unordered_map<std::string, std::string> Server::ParseKeyValueFields(const std::vector<std::string>& tokens, size_t startIndex)
{
    std::unordered_map<std::string, std::string> result;

    for (size_t i = startIndex; i < tokens.size(); ++i)
    {
        const std::string& field = tokens[i];
        size_t eqPos = field.find('=');
        if (eqPos != std::string::npos && eqPos > 0 && eqPos + 1 < field.length())
        {
            std::string key = field.substr(0, eqPos);
            std::string value = field.substr(eqPos + 1);
            result[key] = value;
        }
    }

    return result;
}

void Server::CheckClientLiveness()
{
    auto now = std::chrono::steady_clock::now();

    for (auto it = clients.begin(); it != clients.end(); )
    {
        int clientId = it->first;
        ClientSession& session = it->second;

        if (!session.is_connected)
        {
            ++it;
            continue;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - session.lastPongTime);
        if (elapsed.count() > 10)
        {
            logger.Log("[Ping] 클라이언트 " + std::to_string(clientId) + " 타임아웃");

            closesocket(session.socket);
            std::shared_ptr<Scene> scene = sceneManager.getScene(clientId);
            if (scene) scene->removePlayer(clientId);

            std::string leavePacket = "PLAYER_LEAVE," + std::to_string(clientId) + "\n";
            BroadcastPacket(leavePacket, clientId);

            it = clients.erase(it);
        }
        else
        {
            std::string ping = "PING\n";
            send(session.socket, ping.c_str(), (int)ping.length(), 0);
            ++it;
        }
    }
}

void Server::BroadcastCharacterSelect(Server* pServer)
{
    if (!pServer) return;

    std::cout << "[THREAD] BroadcastCharacterSelect 스레드 시작됨\n";

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        bool hasCharacterSelectClients = false;
        for (const auto& [clientId, scenePtr] : pServer->sceneManager.getAllScenes())
        {
            if (scenePtr && scenePtr->GetSceneType() == Scene_Type::Lobby)
            {
                hasCharacterSelectClients = true;
                break;
            }
        }

        if (!hasCharacterSelectClients)
        {
            continue;
        }

        if (pServer->characterSelections.empty())
        {
            continue;
        }

        for (const auto& [clientId, scenePtr] : pServer->sceneManager.getAllScenes())
        {
            if (scenePtr && scenePtr->GetSceneType() == Scene_Type::Lobby)
            {
                auto it = pServer->characterSelections.find(clientId);
                int charId = (it != pServer->characterSelections.end()) ? it->second : -999;
                std::cout << "[SERVER][LOBBY] clientId=" << clientId << "  charId=" << charId << std::endl;
            }
        }

        for (const auto& [selectedClientId, charId] : pServer->characterSelections)
        {
            std::string packet = "CHARACTER_STATUS," + std::to_string(selectedClientId) + "," + std::to_string(charId) + "\n";
            for (const auto& [targetId, session] : pServer->clients)
            {
                if (session.is_connected)
                {
                    std::cout << "[SERVER][SEND] CHARACTER_STATUS to Client " << targetId << " : selectedClientId=" << selectedClientId << " charId=" << charId << std::endl;
                    send(session.socket, packet.c_str(), static_cast<int>(packet.length()), 0);
                }
            }
        }

        bool allSelected = true;
        int lobbyCount = 0;
        for (const auto& [clientId, scenePtr] : pServer->sceneManager.getAllScenes())
        {
            if (scenePtr && scenePtr->GetSceneType() == Scene_Type::Lobby)
            {
                lobbyCount++;
                auto it = pServer->characterSelections.find(clientId);
                if (it == pServer->characterSelections.end() || it->second == -1)
                {
                    allSelected = false;
                    break;
                }
            }
        }

        std::cout << "[SERVER][STATUS] lobbyCount=" << lobbyCount << "  allSelected=" << (allSelected ? "true" : "false") << "  allSelectedSent=" << (pServer->allSelectedSent ? "true" : "false") << std::endl;
        std::cout << "[SERVER][characterSelections] { ";
        for (const auto& [id, charId] : pServer->characterSelections)
        {
            std::cout << id << ":" << charId << " ";
        }
        std::cout << "}" << std::endl;

        if (allSelected && lobbyCount > 0 && !pServer->allSelectedSent)
        {
            std::cout << "[SERVER][BROADCAST] ENTER_SCENE → 모든 클라에 전송 (lobbyCount=" << lobbyCount << ")\n";
            std::cout << "[SERVER][BROADCAST] 대상 클라 목록: ";
            for (const auto& [targetId, session] : pServer->clients)
                if (session.is_connected)
                    std::cout << targetId << " ";
            std::cout << std::endl;

            std::string startMsg = "ENTER_SCENE,Game_Stage_Board\n";
            for (const auto& [targetId, session] : pServer->clients)
                if (session.is_connected)
                    send(session.socket, startMsg.c_str(), (int)startMsg.length(), 0);

            pServer->allSelectedSent = true;
            std::cout << "[서버] 모든 유저가 선택 완료 → ENTER_SCENE,Game_Stage_Board 전송\n";
        }

        if (!allSelected) pServer->allSelectedSent = false;
    }
}


int main()
{
    Server server(9000);
    server.Start();


    std::thread characterStatusThread(&Server::BroadcastCharacterSelect, &server);
    characterStatusThread.detach();

    while (true)
    {
        for (auto& [sceneId, scene] : server.getSceneManager().getAllScenes())
        {
            scene->update_player_Position();
        }
        //server.BroadcastAllStates();
        //server.CheckClientLiveness();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}


int Server::GetNewClientId()
{
    std::lock_guard<std::mutex> lock(idMutex);

    int id;
    if (!availableIds.empty())
    {
        id = availableIds.top();
        availableIds.pop();
    }
    else
    {
        id = nextClientId++;
    }
    activeClientIds.insert(id);
    return id;
}

void Server::ReleaseClientId(int clientId)
{
    std::lock_guard<std::mutex> lock(idMutex);

    size_t erased = activeClientIds.erase(clientId);
    if (erased > 0)
    {
        availableIds.push(clientId);
        //std::cout << "[DEBUG] Released client ID: " << clientId << " (pushed to availableIds)\n";
        //std::cout << "[DEBUG] availableIds size: " << availableIds.size() << ", nextClientId: " << nextClientId << std::endl;
    }
    else
    {
        std::cout << "[WARN] Tried to release unused client ID: " << clientId << std::endl;
    }
}


void Server::DisconnectClient(int clientId)
{
    {
        std::lock_guard<std::mutex> lock(clientsMutex);

        sceneManager.removeScene(clientId);

        auto it = clients.find(clientId);
        if (it != clients.end())
        {
            closesocket(it->second.socket);
            clients.erase(it);
        }
    }

    {
        std::lock_guard<std::mutex> lock(characterMutex);

        auto selIt = characterSelections.find(clientId);
        if (selIt != characterSelections.end()) {
            lockedCharacterIds.erase(selIt->second);
            characterSelections.erase(selIt);    
        }
    }

    ReleaseClientId(clientId);

    std::cout << "[INFO] Client disconnected: " << clientId << std::endl;
}