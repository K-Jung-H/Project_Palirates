#pragma once
#include <unordered_map>
#include <string>
#include <mutex>
#include <vector>
#include <iostream>

class Lobby 
{
private:
    std::unordered_map<int, std::string> players;
    std::mutex lobbyMutex;
public:
    void AddPlayer(int playerId) {
        std::lock_guard<std::mutex> lock(lobbyMutex);
        if (players.find(playerId) == players.end()) 
        {
            players[playerId] = "Not Ready";
        }
        else {
            std::cerr << "�÷��̾� " << playerId << " �̹� ������.\n";
        }
    }

    void RemovePlayer(int playerId) {
        std::lock_guard<std::mutex> lock(lobbyMutex);
        if (players.find(playerId) != players.end()) 
        {
            players.erase(playerId);
        }
        else {
            std::cerr << "�÷��̾� " << playerId << " �������� ����.\n";
        }
    }

    bool SetReady(int playerId)
    {
        std::lock_guard<std::mutex> lock(lobbyMutex);
        if (players.find(playerId) != players.end()) 
        {
            players[playerId] = "Ready";
            return true;
        }
        std::cerr << "�÷��̾� " << playerId << " ã�� �� ����.\n";
        return false;
    }

    bool AllReady() 
    {
        std::lock_guard<std::mutex> lock(lobbyMutex);
        if (players.empty())
        {
            std::cerr << "�κ� �÷��̾ ����.\n";
            return false;
        }
        for (const auto& [id, status] : players) 
        {
            if (status != "Ready") return false;
        }
        return true;
    }

    void HandleLobbyPacket(int playerId, const std::string& packet)
    {
        if (packet == "READY") {
            if (!SetReady(playerId)) {
                std::cerr << "�߸��� READY ��û(playerId: " << playerId << ")\n";
            }
        }
        else
        {
            std::cerr << "�� �� ���� ��Ŷ: " << packet << "\n";
        }
    }
};
