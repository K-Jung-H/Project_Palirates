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

        std::cout << "[DEBUG] 클라이언트 " << clientId << " 소켓 수락됨" << std::endl;
        std::cout << "[INFO] 클라이언트 " << clientId << " 현재 접속 수: " << clients.size() << std::endl;

        logger.Log("클라이언트 " + std::to_string(clientId) + " 연결됨.");

        std::cout << "[DEBUG] 클라이언트 " << clientId << " 스레드 시작" << std::endl;
        std::thread(&Server::ProcessClientPackets, this, clientSocket, clientId).detach();
    }
}



void Server::ProcessClientPackets(SOCKET clientSocket, int clientId)
{
    char buffer[1024];

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        std::cout << "[DEBUG] recv() 호출됨, 받은 바이트 수: " << bytesReceived << std::endl;

        if (bytesReceived <= 0)
        {
            std::cout << "[DEBUG] 수신 실패 또는 연결 종료, 에러 코드: " << WSAGetLastError() << std::endl;
        }

        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            std::string packet(buffer);
            logger.Log("[DEBUG] 수신된 데이터: " + packet);

            float x, y, z;
            int id, state;

            if (sscanf_s(packet.c_str(), "MOVE,%d,%f,%f,%f,%d", &id, &x, &y, &z, &state) == 5)
            {
                Scene* scene = sceneManager.getScene(clientId);
                scene->updatePlayerPosition(id, x, y, z, state);
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
            closesocket(clientSocket);
            clients.erase(clientId);
            std::cout << "[INFO] 클라이언트 " << clientId << " 연결 종료. 현재 접속 수: " << clients.size() << std::endl;
            break;
        }
        else
        {
            std::cout << "[DEBUG] 수신 실패 또는 연결 종료, 에러 코드: " << WSAGetLastError() << std::endl;

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
    std::cout << "[INFO] 현재 접속 중인 클라이언트 수: " << clients.size() << std::endl;
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
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}


//bool Server::ValidatePosition(float x, float y, float z)
//{
//    return 0;//(x >= -1000 && x <= 1000) && (y >= -1000 && y <= 1000) && (z >= -1000 && z <= 1000);
//}