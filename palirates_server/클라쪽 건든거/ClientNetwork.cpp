#include "stdafx.h"
#include "ClientNetwork.h"
#include <iostream>

ClientNetwork::ClientNetwork()
{
    WSAStartup(MAKEWORD(2, 2), NULL);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
}

ClientNetwork::~ClientNetwork()
{
    Disconnect();
    WSACleanup();
}

bool ClientNetwork::Connect(const std::string& ip, int port)
{
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    if (connect(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == 0)
    {
        std::cout << "서버 연결 성공\n";
        return true;
    }
    std::cerr << "서버 연결 실패\n";
    return false;
}

void ClientNetwork::SendPlayerMove(int id, float x, float y, float z, int state)
{
    static float lastX = -9999, lastY = -9999, lastZ = -9999;
    static int lastState = -1;

    if (x != lastX || y != lastY || z != lastZ || state != lastState)  // 위치 변경 시에만 전송
    {
        std::string packet = "MOVE," + std::to_string(id) + "," +
            std::to_string(x) + "," + std::to_string(y) + "," +
            std::to_string(z) + "," + std::to_string(state);

        send(serverSocket, packet.c_str(), packet.size(), 0);

        lastX = x;
        lastY = y;
        lastZ = z;
        lastState = state;
    }
}

std::string ClientNetwork::ReceiveData()
{
    char buffer[1024];
    int bytesReceived = recv(serverSocket, buffer, sizeof(buffer), 0);
    return (bytesReceived > 0) ? std::string(buffer, bytesReceived) : "";
}

void ClientNetwork::Disconnect()
{
    closesocket(serverSocket);
    std::cout << "서버 연결 종료\n";
}
