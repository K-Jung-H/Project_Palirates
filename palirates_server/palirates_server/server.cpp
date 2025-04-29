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

  
        int clientId = clients.size();
        clients[clientId] = clientSocket;
        sceneManager.addScene(clientId);

        logger.Log("클라이언트 " + std::to_string(clientId) + " 연결됨.");

        char sendBuffer[256];
        sprintf_s(sendBuffer, sizeof(sendBuffer), "CLIENT_ID,%d", clientId);

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

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            std::string packet(buffer);
            logger.Log("클라이언트 " + std::to_string(clientId) + " 패킷 수신: " + packet);

            float x, y, z;
            int state;

            if (sscanf_s(packet.c_str(), "MOVE,%d,%f,%f,%f,%d", &clientId, &x, &y, &z, &state) == 5)
            {
                Scene* scene = sceneManager.getScene(clientId);
                if (scene)
                {
                    scene->updatePlayerPosition(clientId, x, y, z, state);
                }

                std::string response = "PLAYER_UPDATE," + std::to_string(clientId) + "," +
                    std::to_string(x) + "," + std::to_string(y) + "," +
                    std::to_string(z) + "," + std::to_string(state);

                logger.Log("클라이언트 " + std::to_string(clientId) + "에게 브로드캐스트: " + response);
                BroadcastPacket(response, clientId);
            }
            else
            {
                logger.Log("잘못된 패킷 형식 수신: " + packet);
            }
        }
        else if (bytesReceived == 0)
        {
            logger.Log("클라이언트 " + std::to_string(clientId) + " 연결 종료");
            std::string leavePacket = "PLAYER_LEAVE," + std::to_string(clientId);
            BroadcastPacket(leavePacket, clientId);
            closesocket(clientSocket);
            clients.erase(clientId);
            break;
        }
        else
        {
            logger.Log("recv() 오류 발생: " + std::to_string(WSAGetLastError()));
            break;
        }
    }
}
void Server::BroadcastPacket(const std::string& packet, int senderId)
{
    for (const auto& [id, socket] : clients)
    {
        if (id != senderId)
        {
            int bytesSent = send(socket, packet.c_str(), packet.length(), 0);
            if (bytesSent == SOCKET_ERROR)
            {
                logger.Log("[ERROR] 클라이언트 " + std::to_string(id) + "에게 send() 실패: " + std::to_string(WSAGetLastError()));
            }
            else
            {
                logger.Log("클라이언트 " + std::to_string(id) + "에게 패킷 전송 완료: " + packet);
            }
        }
    }
}

void Server::BroadcastAllStates()
{
    for (const auto& [clientId, scene] : sceneManager.getAllScenes())
    {
        for (const auto& [playerId, player] : scene.getPlayers())
        {
            std::string packet = "PLAYER_UPDATE," + std::to_string(playerId) + "," +
                std::to_string(player.x) + "," + std::to_string(player.y) + "," +
                std::to_string(player.z) + "," + std::to_string(player.state);

            BroadcastPacket(packet, -1); // -1이면 모든 클라이언트에게 전송
        }
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

        std::string packet = "PLAYER_UPDATE," + std::to_string(otherId) + "," +
            std::to_string(character->x) + "," +
            std::to_string(character->y) + "," +
            std::to_string(character->z) + "," +
            std::to_string(character->state);

        send(clients[clientId], packet.c_str(), packet.length(), 0);
    }
}


void Server::NotifyExistingPlayersAboutNew(int newClientId)
{
    Scene* scene = sceneManager.getScene(newClientId);
    if (!scene) return;

    const GameCharacter* character = scene->getPlayer(newClientId);
    if (!character) return;

    std::string packet = "PLAYER_UPDATE," + std::to_string(newClientId) + "," +
        std::to_string(character->x) + "," +
        std::to_string(character->y) + "," +
        std::to_string(character->z) + "," +
        std::to_string(character->state);

    for (const auto& [clientId, sock] : clients)
    {
        if (clientId == newClientId) continue;  // 자기 자신 제외
        send(sock, packet.c_str(), packet.length(), 0);
    }

    logger.Log("기존 유저들에게 신규 클라이언트 " + std::to_string(newClientId) + " 상태 전송 완료");
}


Server::~Server()
{
    for (const auto& [id, socket] : clients)
    {
		closesocket(socket);
	}

	closesocket(listenSocket);
	WSACleanup();
}

void Server::Start()
{
	std::thread(&Server::AcceptClients, this).detach();
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

//bool Server::ValidatePosition(float x, float y, float z)
//{
//    return 0;//(x >= -1000 && x <= 1000) && (y >= -1000 && y <= 1000) && (z >= -1000 && z <= 1000);
//}

