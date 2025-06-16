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
}

void Server::AcceptClients()
{
    while (true)
    {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);

        int clientId = nextClientId++;

        ClientSession session;
        session.socket = clientSocket;
        session.is_connected = true;
        session.lastPongTime = std::chrono::steady_clock::now();
        clients[clientId] = session;


        sceneManager.addScene(clientId);
        shared_ptr<Scene> scene = sceneManager.getScene(clientId);

        char sendBuffer[256];
        sprintf_s(sendBuffer, "CLIENT_ID,%d", clientId);
        logger.Log("클라이언트 " + std::to_string(clientId) + " 연결됨.");

        int retval = send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
        if (retval == SOCKET_ERROR)
        {
            logger.Log("[서버] CLIENT_ID 전송 실패! 에러 코드: " + std::to_string(WSAGetLastError()));
        }
        else
        {
            logger.Log("[서버] CLIENT_ID 전송 성공! 보낸 데이터: " + std::string(sendBuffer));
        }

        // SendInitialStates(clientId);
        // NotifyExistingPlayersAboutNew(clientId);

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
                logger.Log("클라이언트 " + std::to_string(clientId) + " 연결 종료 또는 recv 실패");
                break;
            }

            buffer[bytesReceived] = '\0';
            recvBuffer += buffer;

            size_t pos;
            while ((pos = recvBuffer.find('\n')) != std::string::npos)
            {
                std::string packet = recvBuffer.substr(0, pos);
                recvBuffer.erase(0, pos + 1);

                logger.Log("클라이언트 " + std::to_string(clientId) + " 패킷 수신: " + packet);

                int id, state;
                float x, y, z;
                float lookX, lookY, lookZ;

                if (packet.rfind("PLAYER_UPDATE,", 0) == 0)
                {
                    std::istringstream iss(packet);
                    std::string token;
                    std::vector<std::string> tokens;

                    while (std::getline(iss, token, ','))
                        tokens.push_back(token);

                    if (tokens.size() >= 10)
                    {
                        id = std::stoi(tokens[1]);
                        x = std::stof(tokens[2]);
                        y = std::stof(tokens[3]);
                        z = std::stof(tokens[4]);
                        lookX = std::stof(tokens[5]);
                        lookY = std::stof(tokens[6]);
                        lookZ = std::stof(tokens[7]);
                        state = std::stoi(tokens[8]);
                        int trackCount = std::stoi(tokens[9]);

                        std::vector<float> trackPositions;
                        std::vector<float> trackWeights;
                        for (int i = 0; i < trackCount; ++i)
                        {
                            int base = 10 + i * 2;
                            if (base + 1 >= tokens.size()) break;
                            trackPositions.push_back(std::stof(tokens[base]));
                            trackWeights.push_back(std::stof(tokens[base + 1]));
                        }

                        if (trackPositions.empty() || trackWeights.empty())
                        {
                            trackPositions.push_back(0.0f);
                            trackWeights.push_back(1.0f);
                            trackCount = 1;
                        }

                        shared_ptr<Scene> scene = sceneManager.getScene(clientId);
                        if (!scene) {
                            sceneManager.addScene(clientId);
                            scene = sceneManager.getScene(clientId);
                        }

                        scene->updatePlayerPosition(clientId, x, y, z, lookX, lookY, lookZ, static_cast<Player_State>(state));
                        scene->updatePlayerAnimation(clientId, trackPositions, trackWeights);

                        std::ostringstream oss;
                        oss << "PLAYER_UPDATE," << clientId << ","
                            << x << "," << y << "," << z << ","
                            << lookX << "," << lookY << "," << lookZ << ","
                            << state << "," << trackCount;

                        for (int i = 0; i < trackCount; ++i)
                        {
                            oss << "," << trackPositions[i] << "," << trackWeights[i];
                        }
                        oss << "\n";

                        BroadcastPacket(oss.str(), clientId);
                    }
                }
                else if (packet.rfind("CHARACTER_SELECT,", 0) == 0)
                {
                    std::istringstream iss(packet);
                    std::string token;
                    std::vector<std::string> tokens;

                    while (std::getline(iss, token, ','))
                        tokens.push_back(token);

                    if (tokens.size() >= 2)
                    {
                        int selectedCharId = std::stoi(tokens[1]);

                        // 이미 선택된 캐릭터인지 확인
                        if (lockedCharacterIds.find(selectedCharId) != lockedCharacterIds.end())
                        {
                            logger.Log("[REJECTED] Character " + std::to_string(selectedCharId) + " already selected.");
                            return;
                        }

                        characterSelections[clientId] = selectedCharId;
                        lockedCharacterIds.insert(selectedCharId);

                        std::string lockPacket = "CHARACTER_LOCKED," + std::to_string(selectedCharId);
                        BroadcastPacket(lockPacket, -1);

                        std::string confirmPacket = "CHARACTER_SELECTED," + std::to_string(clientId) + "," + std::to_string(selectedCharId);
                        BroadcastPacket(confirmPacket, -1);

                        logger.Log("[SELECTED] Client " + std::to_string(clientId) + " selected character " + std::to_string(selectedCharId));

                        shared_ptr<Scene> scene = sceneManager.getScene(clientId);
                        if (scene)
                        {
                            scene->addPlayer(clientId, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
                        }

                        NotifyExistingPlayersAboutNew(clientId);
                        SendInitialStates(clientId);

                        int controllerId = GetControllerId(scene);
                        controllerIdByScene[clientId] = controllerId;

                        if (controllerId != -1)
                        {
                            std::string packet = "SHIP_CONTROLLER_ID," + std::to_string(controllerId);
                            BroadcastPacket(packet, -1);
                        }
                        else
                        {
                            logger.Log("[경고] GetControllerId() 실패로 -1 반환됨");
                        }
                    }
                }
                else if (packet.rfind("SHIP_SYNC,", 0) == 0)
                {
                    std::istringstream iss(packet);
                    std::string token;
                    std::vector<std::string> tokens;

                    while (std::getline(iss, token, ','))
                        tokens.push_back(token);

                    if (tokens.size() >= 7)
                    {
                        shipX = std::stof(tokens[1]);
                        shipY = std::stof(tokens[2]);
                        shipZ = std::stof(tokens[3]);
                        shipLookX = std::stof(tokens[4]);
                        shipLookY = std::stof(tokens[5]);
                        shipLookZ = std::stof(tokens[6]);

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
                }
                else if (packet.rfind("PLAYER_LEAVE,", 0) == 0)
                {
                    logger.Log("클라이언트 " + std::to_string(clientId) + " 퇴장 처리");
                    clients[clientId].is_connected = false;
                    closesocket(clients[clientId].socket);

                    shared_ptr<Scene> scene = sceneManager.getScene(clientId);
                    if (scene) scene->removePlayer(clientId);

                    std::string leavePacket = "PLAYER_LEAVE," + std::to_string(clientId) + "\n";
                    BroadcastPacket(leavePacket, clientId);

                    clients.erase(clientId);
                }
                else if (packet == "PING")
                {
                    send(clientSocket, "PONG\n", 5, 0);
                }
                else if (packet == "PONG")
                {
                    clients[clientId].lastPongTime = std::chrono::steady_clock::now();
                }
                else if (packet.rfind("KEY_INPUT,", 0) == 0)
                {
                    std::istringstream iss(packet);
                    std::string token;
                    std::vector<std::string> tokens;

                    while (std::getline(iss, token, ','))
                        tokens.push_back(token);

                    if (tokens.size() >= 2)
                    {
                        int keyMask = std::stoi(tokens[1]);

                        shared_ptr<Scene> scene = sceneManager.getScene(clientId);
                        if (!scene)
                        {
                            sceneManager.addScene(clientId);
                            scene = sceneManager.getScene(clientId);
                        }

                        std::shared_ptr<Player> player = scene->getPlayer(clientId);
                        if (!player) return;

                        player->key_input(keyMask);
                        player->update(); // 위치 갱신

                        XMFLOAT3 pos = player->GetPosition();
                        std::ostringstream oss;
                        oss << "POSITION_UPDATE," << clientId << ","
                            << pos.x << "," << pos.y << "," << pos.z << "\n";

                        BroadcastPacket(oss.str(), -1);
                    }
                }
                else
                {
                    logger.Log("잘못된 패킷 형식 수신: " + packet);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        logger.Log("[예외] 클라이언트 " + std::to_string(clientId) + " 처리 중 예외 발생: " + e.what());
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

            std::string packet = "PLAYER_UPDATE," + std::to_string(playerId) + "," +
                std::to_string(player_pos.x) + "," + std::to_string(player_pos.y) + "," + std::to_string(player_pos.z) + "," +
                std::to_string(player_look.x) + "," + std::to_string(safeLookY) + "," + std::to_string(player_look.z) + "," +
                std::to_string(static_cast<int>(player->GetState())) + "\n";

            BroadcastPacket(packet, -1); // 전체 클라이언트에게 전송
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
        if (!character)
        {
            continue;
        }

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

    for (const auto& [clientId, session] : clients)
    {
        if (clientId == newClientId || !session.is_connected) continue;

        send(session.socket, createPacket.c_str(), createPacket.length(), 0);
        send(session.socket, updatePacket.c_str(), updatePacket.length(), 0);
    }

    logger.Log("[서버] 기존 유저들에게 신규 클라이언트 " + std::to_string(newClientId) + " 상태 전송 완료");
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
    BroadcastAllStates();

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

int main()
{
    Server server(9000);
    server.Start();


    while (true)
    {
        for (auto& [sceneId, scene] : server.getSceneManager().getAllScenes())
        {
            scene->update_player_Position();
        }

        server.BroadcastAllStates();
        //server.CheckClientLiveness();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

