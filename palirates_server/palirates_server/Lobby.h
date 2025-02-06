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
            std::cerr << "플레이어 " << playerId << " 이미 존재함.\n";
        }
    }

    void RemovePlayer(int playerId) {
        std::lock_guard<std::mutex> lock(lobbyMutex);
        if (players.find(playerId) != players.end()) 
        {
            players.erase(playerId);
        }
        else {
            std::cerr << "플레이어 " << playerId << " 존재하지 않음.\n";
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
        std::cerr << "플레이어 " << playerId << " 찾을 수 없음.\n";
        return false;
    }

    bool AllReady() 
    {
        std::lock_guard<std::mutex> lock(lobbyMutex);
        if (players.empty())
        {
            std::cerr << "로비에 플레이어가 없슴.\n";
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
                std::cerr << "잘못된 READY 요청(playerId: " << playerId << ")\n";
            }
        }
        else
        {
            std::cerr << "알 수 없는 패킷: " << packet << "\n";
        }
    }
};
