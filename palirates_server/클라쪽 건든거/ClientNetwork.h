#pragma once
#include <winsock2.h>
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
    void SendPacket(const std::string& data);
    std::string ReceiveData();
    void Disconnect();
};
