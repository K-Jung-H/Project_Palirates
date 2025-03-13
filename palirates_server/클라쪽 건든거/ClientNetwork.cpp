#include "ClientNetwork.h"
#include <iostream>
#include <mutex>

std::mutex networkMutex;

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
        std::cout << "[CLIENT] 서버 연결 성공\n";
        return true;
    }
    std::cerr << "[ERROR] 서버 연결 실패\n";
    return false;
}

void ClientNetwork::SendPacket(const std::string& data)
{
    std::lock_guard<std::mutex> lock(networkMutex);
    int result = send(serverSocket, data.c_str(), data.size(), 0);
    if (result == SOCKET_ERROR)
    {
        std::cerr << "[ERROR] 패킷 전송 실패: " << WSAGetLastError() << std::endl;
    }
    else
    {
        std::cout << "[CLIENT] 패킷 전송: " << data << std::endl;
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
    std::cout << "[CLIENT] 서버 연결 종료\n";
}
