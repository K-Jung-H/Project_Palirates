#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class ClientNetwork
{
private:
    SOCKET serverSocket;
    sockaddr_in serverAddr;

public:
    ClientNetwork();
    ~ClientNetwork();

    bool Connect(const std::string& ip, int port);
    void SendPlayerMove(int id, float x, float y, float z, int state);
    std::string ReceiveData();
    void Disconnect();
};
