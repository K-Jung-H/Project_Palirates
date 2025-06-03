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

    //dbManager.Connect();
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
        clients[clientId] = session;

        sceneManager.addScene(clientId);
        Scene* scene = sceneManager.getScene(clientId);
        if (scene)
        {
            scene->addPlayer(clientId);
        }

        char sendBuffer[256];
        sprintf_s(sendBuffer, "CLIENT_ID,%d", clientId);

        logger.Log("클라이언트 " + std::to_string(clientId) + " 연결됨.");

        //char sendBuffer[256];
        //sprintf_s(sendBuffer, sizeof(sendBuffer), "CLIENT_ID,%d", clientId);

        int retval = send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
        if (retval == SOCKET_ERROR)
        {
            logger.Log("[서버] CLIENT_ID 전송 실패! 에러 코드: " + std::to_string(WSAGetLastError()));
        }
        else
        {
            logger.Log("[서버] CLIENT_ID 전송 성공! 보낸 데이터: " + std::string(sendBuffer));
        }


        SendInitialStates(clientId);
        NotifyExistingPlayersAboutNew(clientId);

        std::thread(&Server::ProcessClientPackets, this, clientSocket, clientId).detach();
    }
}




void Server::ProcessClientPackets(SOCKET clientSocket, int clientId)
{
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

                    Scene* scene = sceneManager.getScene(clientId);
                    if (!scene) {
                        sceneManager.addScene(clientId);
                        scene = sceneManager.getScene(clientId);
                    }

                    if (!scene->getPlayer(clientId)) {
                        scene->addPlayer(clientId);
                    }

                    scene->updatePlayerPosition(clientId, x, y, z, lookX, lookY, lookZ, static_cast<EState>(state));
                    scene->updatePlayerAnimation(clientId, trackPositions, trackWeights);

                    std::ostringstream oss;
                    oss << "PLAYER_UPDATE," << clientId << "," << x << "," << y << "," << z
                        << "," << lookX << "," << lookY << "," << lookZ << "," << state
                        << "," << trackCount;
                    for (int i = 0; i < trackCount; ++i)
                    {
                        oss << "," << trackPositions[i] << "," << trackWeights[i];
                    }
                    oss << "\n";

                    BroadcastPacket(oss.str(), clientId);
                }
            }
            else if (packet.rfind("PLAYER_LEAVE,", 0) == 0)
            {
                logger.Log("클라이언트 " + std::to_string(clientId) + " 퇴장 처리");
                clients[clientId].is_connected = false;
                closesocket(clients[clientId].socket);

                Scene* scene = sceneManager.getScene(clientId);
                if (scene) scene->removePlayer(clientId);

                std::string leavePacket = "PLAYER_LEAVE," + std::to_string(clientId) + "\n";
                BroadcastPacket(leavePacket, clientId);

                clients.erase(clientId);

            }
            else
            {
                logger.Log("잘못된 패킷 형식 수신: " + packet);
            }
        }
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
        {
            logger.Log("[ERROR] 클라이언트 " + std::to_string(id) + "에게 send() 실패: " + std::to_string(WSAGetLastError()));
        }
        else
        {
            logger.Log("클라이언트 " + std::to_string(id) + "에게 패킷 전송 완료: " + finalizedPacket);
        }
    }
}

void Server::BroadcastAllStates()
{
    for (const auto& [clientId, scene] : sceneManager.getAllScenes())
    {
        for (const auto& [playerId, player] : scene.getPlayers())
        {

            float safeLookY = (player.lookY == 0.0f) ? 1.0f : player.lookY;

            std::string packet = "PLAYER_UPDATE," + std::to_string(playerId) + "," +
                std::to_string(player.x) + "," + std::to_string(player.y) + "," +
                std::to_string(player.z) + "," + std::to_string(player.lookX) + "," + std::to_string(safeLookY) + "," +
                std::to_string(player.lookZ) + "," + std::to_string(static_cast<int>(player.state)) + "\n";


            BroadcastPacket(packet, -1); // -1이면 모든 클라이언트에게 전송
        }

        //for (const auto& [monsterId, monster] : scene.getMonsters())
        //{
        //    std::ostringstream oss;
        //    oss << "MONSTER_UPDATE," << monster.id << ","
        //        << monster.x << "," << monster.y << "," << monster.z << ","
        //        << monster.lookX << "," << monster.lookY << "," << monster.lookZ << ","
        //        << monster.hp << "," << monster.state << "," << static_cast<int>(monster.type);
        //
        //    int trackCount = static_cast<int>(monster.trackPositions.size());
        //    oss << "," << trackCount;
        //    for (int i = 0; i < trackCount; ++i)
        //        oss << "," << monster.trackPositions[i] << "," << monster.trackWeights[i];
        //
        //    BroadcastPacket(oss.str(), -1);
        //}
    }
}


void Server::SendInitialStates(int clientId)
{
    Scene* myScene = sceneManager.getScene(clientId);
    if (!myScene) return;

    for (const auto& [otherId, scene] : sceneManager.getAllScenes())
    {
        if (otherId == clientId) continue;

        const GameCharacter* character = scene.getPlayer(otherId);
        if (!character) continue;

        std::string createPacket = "PLAYER_CREATE," + std::to_string(otherId) + "\n";
        send(clients[clientId].socket, createPacket.c_str(), createPacket.length(), 0);

        float safeLookY = (character->lookY == 0.0f) ? 1.0f : character->lookY;

        std::string updatePacket = "PLAYER_UPDATE," + std::to_string(otherId) + "," +
            std::to_string(character->x) + "," +
            std::to_string(character->y) + "," +
            std::to_string(character->z) + "," +
            std::to_string(character->lookX) + "," +
            std::to_string(safeLookY) + "," +
            std::to_string(character->lookZ) + "," +
            std::to_string(static_cast<int>(character->state)) + "\n";
        send(clients[clientId].socket, updatePacket.c_str(), updatePacket.length(), 0);
        logger.Log("[서버] (SendInitialStates) PLAYER_CREATE 전송: " + createPacket);


        //for (const auto& [monsterId, monster] : scene.getMonsters())
        //{
        //    std::string create = "MONSTER_CREATE," + std::to_string(monsterId) + "\n";
        //    send(clients[clientId].socket, create.c_str(), create.length(), 0);
        //
        //    std::string update = "MONSTER_UPDATE," + std::to_string(monsterId) + "," +
        //        std::to_string(monster.x) + "," + std::to_string(monster.y) + "," + std::to_string(monster.z) + "," +
        //        std::to_string(monster.lookX) + "," + std::to_string(monster.lookY) + "," + std::to_string(monster.lookZ) + "," +
        //        std::to_string(monster.hp) + "," + std::to_string(monster.state) + "," + std::to_string((int)monster.type) + "\n";
        //
        //
        //    send(clients[clientId].socket, update.c_str(), update.length(), 0);
        //
        //}
    }
}

void Server::NotifyExistingPlayersAboutNew(int newClientId)
{
    Scene* scene = sceneManager.getScene(newClientId);
    if (!scene) return;

    const GameCharacter* character = scene->getPlayer(newClientId);
    if (!character) return;

    float safeLookY = (character->lookY == 0.0f) ? 1.0f : character->lookY;

    std::string createPacket = "PLAYER_CREATE," + std::to_string(newClientId) + "\n";

    std::string packet = "PLAYER_UPDATE," + std::to_string(newClientId) + "," +
        std::to_string(character->x) + "," +
        std::to_string(character->y) + "," +
        std::to_string(character->z) + "," +
        std::to_string(character->lookX) + "," +
        std::to_string(safeLookY) + "," +
        std::to_string(character->lookZ) + "," +
        std::to_string(static_cast<int>(character->state)) + "\n";

    for (const auto& [clientId, sock] : clients)
    {
        if (clientId == newClientId) continue; // 자기 자신 제외

        send(clients[clientId].socket, createPacket.c_str(), createPacket.length(), 0);
        send(clients[clientId].socket, packet.c_str(), packet.length(), 0);
    }

    logger.Log("[서버] 기존 유저들에게 신규 클라이언트 " + std::to_string(newClientId) + " 상태 전송 완료");
    logger.Log("[서버] (NotifyExistingPlayersAboutNew) PLAYER_CREATE 전송: " + createPacket);
}

void Server::MonsterUpdate(int monsterId, float x, float y, float z, float lookX, float lookY, float lookZ, float aniPos, float aniWei)
{
    std::string packet = "MONSTER_UPDATE," + std::to_string(monsterId) + "," +
        std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + "," +
        std::to_string(lookX) + "," + std::to_string(lookY) + "," + std::to_string(lookZ) + "," +
        std::to_string(aniPos) + "," + std::to_string(aniWei) + "\n";
    BroadcastPacket(packet, -1); // -1이면 모든 클라이언트에게 전송
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

    Scene* scene = sceneManager.getScene(0);
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

int main()
{
    Server server(9000);
    server.Start();


    while (true)
    {
        server.BroadcastAllStates();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

