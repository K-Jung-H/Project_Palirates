#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <mutex>
#include <winsock2.h>
#include "GameObjects.h"

#pragma comment(lib, "ws2_32.lib")

const int PORT = 12345;
const int BUFFER_SIZE = 1024;

std::map<int, Player> players;
std::vector<Monster> monsters;
std::vector<Particle> particles;
std::mutex dataMutex;

void initializeMonsters() // 몬스터 초기화 예시
{
    monsters.emplace_back(1, 100.0f, 200.0f, "idle");
    monsters.emplace_back(2, 500.0f, 600.0f, "idle");
    // Add BossMonster
    monsters.emplace_back(BossMonster(3, 800.0f, 900.0f, "aggressive"));
}

void handleClient(SOCKET clientSocket, int clientId) 
{
    char buffer[BUFFER_SIZE];
    while (true) {
        int received = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (received <= 0)
        {
            std::cout << "Client disconnected: " << clientId << std::endl;
            closesocket(clientSocket);
            std::lock_guard<std::mutex> lock(dataMutex);
            players.erase(clientId);
            return;
        }

        // 데이터 파싱 (예시"x,y,action" 형식)
        buffer[received] = '\0';
        float x, y;
        char action[32];
        sscanf(buffer, "%f,%f,%s", &x, &y, action);

        std::lock_guard<std::mutex> lock(dataMutex);
        players[clientId].update(x, y, action);
    }
}

void updateGameLogic()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            // 몬스터 업데이트
            for (auto& monster : monsters) 
            {
                monster.update();
            }

            // 충돌 처리
            for (const auto& player : players)
            {
                for (const auto& monster : monsters) 
                {
                    if (abs(player.second.pos.x - monster.pos.x) < 10 &&
                        abs(player.second.pos.y - monster.pos.y) < 10)
                    {
                        particles.emplace_back(player.second.pos.x, player.second.pos.y, "collision");
                    }
                }
            }
        }

        // 30FPS로 주기적 업데이트
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

void broadcastData(SOCKET clientSocket)
{
    while (true)
    {
        std::lock_guard<std::mutex> lock(dataMutex);

        std::string data;
        for (const auto& player : players) 
        {
            data += "Player " + std::to_string(player.first) + ": " +
                std::to_string(player.second.pos.x) + "," +
                std::to_string(player.second.pos.y) + "," +
                player.second.action + "\n";
        }

        for (const auto& monster : monsters)
        {
            data += "Monster " + std::to_string(monster.id) + ": " +
                std::to_string(monster.pos.x) + "," +
                std::to_string(monster.pos.y) + "," +
                monster.action + "\n";
        }

        for (const auto& particle : particles) 
        {
            data += "Particle at (" + std::to_string(particle.pos.x) + ", " +
                std::to_string(particle.pos.y) + "): " + particle.type + "\n";
        }

        send(clientSocket, data.c_str(), data.size(), 0);

        particles.clear();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "Server started on port " << PORT << std::endl;

    initializeMonsters();

    std::vector<std::thread> clientThreads;
    std::thread gameLogicThread(updateGameLogic);

    while (true)
    {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        static int clientId = 0;

        std::lock_guard<std::mutex> lock(dataMutex);
        players[clientId] = Player(clientId, 0.0f, 0.0f, "idle");

        clientThreads.emplace_back(handleClient, clientSocket, clientId);
        clientId++;
    }

    for (auto& t : clientThreads)
    {
        if (t.joinable()) t.join();
    }

    gameLogicThread.join();

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
