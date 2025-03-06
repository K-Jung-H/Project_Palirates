#include <iostream>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "LobbyManager.h"
#include "Player.h"
#include "Monster.h"
#include "server.h"

#pragma comment(lib, "ws2_32.lib")

const int PORT = 9000;
const int BUFFER_SIZE = 1024;

std::unordered_map<int, SOCKET> clients;
std::mutex dataMutex;
LobbyManager lobbyManager;
std::unordered_map<int, Player> players;
std::unordered_map<int, Monster> monsters;

// 패킷 수신 확인
void HandleClient(SOCKET clientSocket, int clientId)
{
    char buffer[BUFFER_SIZE];

    while (true)
    {
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            std::string packet(buffer);
            std::cout << "[서버] 클라이언트 " << clientId << " 패킷 수신: " << packet << std::endl;

            HandleGamePacket(clientSocket, packet, clientId);
        }
        else if (bytesReceived == 0)
        {
            std::cout << "[서버] 클라이언트 " << clientId << " 연결 종료" << std::endl;
            closesocket(clientSocket);
            clients.erase(clientId);
            break;
        }
    }
}

// 클라이언트 패킷 처리
void HandleGamePacket(SOCKET clientSocket, const std::string& packet, int clientId)
{
    char command[50];
    float x, y, z;
    int monsterId, damage;
    char response[BUFFER_SIZE];

    // JOIN_LOBBY 처리
    if (packet == "JOIN_LOBBY")
    {
        std::cout << "[서버] 클라이언트 " << clientId << " 로비 입장" << std::endl;
        lobbyManager.JoinRoom(clientId, 1);
        sprintf_s(response, "JOIN_SUCCESS,%d", 1);
    }
    // READY 처리
    else if (packet == "READY")
    {
        std::cout << "[서버] 클라이언트 " << clientId << " 준비 완료" << std::endl;
        lobbyManager.StartGame(1);
    }

    // MOVE 처리
    else if (sscanf_s(packet.c_str(), "MOVE,%f,%f,%f", &x, &y, &z) == 3)
    {
        players[clientId].update(x, y, z, 1);
        sprintf_s(response, "PLAYER_UPDATE,%d,%.2f,%.2f,%.2f,1", clientId, x, y, z);
    }
    // ATTACK_MONSTER 처리
    else if (sscanf_s(packet.c_str(), "ATTACK_MONSTER,%d,%d", &monsterId, &damage) == 2)
    {
        if (monsters.find(monsterId) != monsters.end())
        {
            monsters[monsterId].hp -= damage;
            sprintf_s(response, "MONSTER_UPDATE,%d,%d", monsterId, monsters[monsterId].hp);
        }
    }
    // 모든 클라이언트에게 패킷 전송
    for (const auto& [id, socket] : clients)
    {
        send(socket, response, strlen(response), 0);
    }
}

int main()
{
    std::cout << "[서버] 서버를 시작합니다." << std::endl;

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cerr << "[서버] WSAStartup 실패! 오류 코드: " << result << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "[서버] 소켓 생성 실패!" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "[서버] 바인딩 실패!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "[서버] 리슨 실패!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[서버] 클라이언트 연결을 기다립니다..." << std::endl;

    int clientId = 1;

    while (true)
    {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "[서버] 클라이언트 연결 실패!" << std::endl;
            continue;
        }

        std::cout << "[서버] 클라이언트 " << clientId << " 연결 성공!" << std::endl;
        clients[clientId] = clientSocket;

        std::thread clientThread(HandleClient, clientSocket, clientId);
        clientThread.detach();

        clientId++;
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
