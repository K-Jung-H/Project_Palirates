#pragma once
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include "SceneManager.h"
#include "DatabaseManager.h"
#include "Logger.h"
#include "Player.h"

#pragma comment(lib, "ws2_32.lib")


struct ClientSession
{
    SOCKET socket;
    bool is_connected = true;
};

class Server
{
private:
    SOCKET listenSocket;
    std::unordered_map<int, ClientSession> clients;
    Scene_Manager sceneManager;
    //DatabaseManager dbManager;
    Logger logger;

public:
    Server(int port);
    ~Server();

    int nextClientId = 0;

    void Start();
    void AcceptClients();
    void ProcessClientPackets(SOCKET clientSocket, int clientId);
    void BroadcastPacket(const std::string& packet, int senderId);
    void SendInitialStates(int clientId);
    void BroadcastAllStates();
    void NotifyExistingPlayersAboutNew(int clientId);
    //bool ValidatePosition(float x, float y, float z);
};
