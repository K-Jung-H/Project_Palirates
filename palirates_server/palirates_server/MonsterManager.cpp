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

std::unordered_map<int, SOCKET> clients;
std::mutex dataMutex;
LobbyManager lobbyManager;

void HandleGamePacket(SOCKET clientSocket, const std::string& packet, int clientId)
{
    if (packet.find("GAME_OVER") != std::string::npos)
    {
        int roomId;
        sscanf_s(packet.c_str(), "GAME_OVER,%d", &roomId);

        std::string gameOverPacket = "GAME_OVER," + std::to_string(roomId);

        // 같은 방에 있는 모든 유저들에게 "GAME_OVER" 패킷 전송
        std::vector<int> playersInRoom = lobbyManager.GetPlayersInSameRoom(clientId);
        for (int player : playersInRoom)
        {
            if (clients.find(player) != clients.end())
            {
                ::send(clients[player], gameOverPacket.c_str(), gameOverPacket.size(), 0);
            }
        }

        lobbyManager.EndGame(roomId); // 방을 삭제하고 모든 유저를 로비로 이동
    }
}
