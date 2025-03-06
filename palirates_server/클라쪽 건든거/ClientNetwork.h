#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include "InGameCharacterManager.h"

#pragma comment(lib, "ws2_32.lib")

class ClientNetwork
{
private:
    SOCKET clientSocket;
    const char* SERVER_IP = "127.0.0.1";
    const int SERVER_PORT = 9000;

public:
    ClientNetwork();
    ~ClientNetwork();

    bool ConnectToServer();
    void ProcessIncomingPackets(GameCharacterManager& characterManager);
    void SendPacket(const std::string& packet);
};
