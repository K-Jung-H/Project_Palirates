#include "ClientNetwork.h"

ClientNetwork::ClientNetwork()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
		std::cerr << "소켓 생성 실패!" << std::endl;
		WSACleanup();
		return;
	}
}

ClientNetwork::~ClientNetwork()
{
	closesocket(clientSocket);
	WSACleanup();
}

bool ClientNetwork::ConnectToServer()
{
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
		std::cerr << "서버 연결 실패!" << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}

	std::cout << "[클라이언트] 서버 연결 성공!" << std::endl;
	return true;
}

void ClientNetwork::ProcessIncomingPackets(Object_Manager& objectManager)
{
	char buffer[1024];

	while (true)
	{
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesReceived > 0)
		{
			buffer[bytesReceived] = '\0';
			int playerId, state;
			float x, y, z;

			if (sscanf_s(buffer, "PLAYER_UPDATE,%d,%f,%f,%f,%d", &playerId, &x, &y, &z, &state) == 5)
			{
				objectManager.UpdatePlayerPosition(playerId, x, y, z, state);
			}
		}
	}
}

void ClientNetwork::SendPacket(const std::string& packet)
{
	send(clientSocket, packet.c_str(), packet.length(), 0);
}
