#include <iostream>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "LobbyManager.h"
#include "Player.h"

#pragma comment(lib, "ws2_32.lib")

const int PORT = 9000;
const int BUFFER_SIZE = 1024;

extern std::unordered_map<int, SOCKET> clients;
extern std::mutex dataMutex;
extern LobbyManager lobbyManager; 

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

    while (true)
    {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        std::cout << "[서버] 클라이언트 연결 성공!" << std::endl;
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
