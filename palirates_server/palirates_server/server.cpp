#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <chrono>
#include "GameObjects.h"
#include "Lobby.h"

#pragma comment(lib, "ws2_32.lib")

const int PORT = 9000;
const int BUFFER_SIZE = 1024;

std::unordered_map<int, Player> players;
std::unordered_map<int, SOCKET> clients;
std::mutex dataMutex;
Lobby gameLobby;


void InitializeWinSock()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        exit(1);
    }
}

SOCKET SetupServerSocket()
{
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        exit(1);
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }
    return serverSocket;
}

void BroadcastPlayerState(int senderId, float x, float y, int movement)
{
    std::string updatePacket = "PLAYER_UPDATE," + std::to_string(senderId) + "," +
        std::to_string(x) + "," + std::to_string(y) + "," +
        std::to_string(movement);
    std::lock_guard<std::mutex> lock(dataMutex);
    for (auto& [id, socket] : clients) 
    {
        if (id != senderId) {
            send(socket, updatePacket.c_str(), updatePacket.size(), 0);
        }
    }
}

void HandleClient(SOCKET clientSocket, int clientId)
{
    char buffer[BUFFER_SIZE];
    players.emplace(std::piecewise_construct,
        std::forward_as_tuple(clientId),
        std::forward_as_tuple(clientId, 0, 0, "idle"));
    gameLobby.AddPlayer(clientId);

    while (true
        ) {
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            std::string packet(buffer);

            if (packet.find("LOGIN") != std::string::npos) 
            {
                std::string response = "LOGIN_SUCCESS," + std::to_string(clientId);
                send(clientSocket, response.c_str(), response.size(), 0);
            }
            else if (packet.find("READY") != std::string::npos)
            {
                gameLobby.SetReady(clientId);
                std::string response = "READY_CONFIRM," + std::to_string(clientId);
                send(clientSocket, response.c_str(), response.size(), 0);
            }
            else if (packet.find("START_GAME") != std::string::npos && gameLobby.AllReady()) 
            {
                std::string startResponse = "GAME_START";
                for (auto& [id, socket] : clients)
                {
                    send(socket, startResponse.c_str(), startResponse.size(), 0);
                }
                players[clientId].inGame = true;
            }
            else if (players[clientId].inGame && packet.find("PLAYER_MOVE") != std::string::npos)
            {
                int playerId, movement;
                float x, y;
                if (sscanf_s(buffer, "PLAYER_MOVE,%d,%f,%f,%d", &playerId, &x, &y, &movement) == 4) 
                {
                    std::lock_guard<std::mutex> lock(dataMutex);
                    players[playerId].update(x, y, "moving");
                    BroadcastPlayerState(playerId, x, y, movement);
                }
            }
        }
        else
        {
            break;
        }
    }
    closesocket(clientSocket);
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        players.erase(clientId);
        clients.erase(clientId);
    }
    gameLobby.RemovePlayer(clientId);
}

void AcceptClients(SOCKET serverSocket)
{
    int clientId = 0;
    while (true)
    {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET) 
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            while (clients.find(clientId) != clients.end()) 
            {
                clientId++;
            }
            clients[clientId] = clientSocket;
            std::thread(HandleClient, clientSocket, clientId).detach();
        }
    }
}

int main()\
{
    InitializeWinSock();
    SOCKET serverSocket = SetupServerSocket();
    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    AcceptClients(serverSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
