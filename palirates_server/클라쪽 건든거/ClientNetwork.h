#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <mutex>


#pragma comment(lib, "ws2_32.lib")

class ClientNetwork
{
private:
    static ClientNetwork* instance;
    SOCKET serverSocket;
    sockaddr_in serverAddr;
    ClientNetwork();

public:
    ClientNetwork();
    ~ClientNetwork();

    bool Connect(const std::string& ip, int port);
    void SendPacket(const std::string& data);
    std::string ReceiveData();
    void Disconnect();
};
