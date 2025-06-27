#include "stdafx.h"
#include "LobbyManager.h"

LobbyManager::LobbyManager() : nextRoomId(1) {}

int LobbyManager::CreateRoom()
{
    std::lock_guard<std::mutex> lock(lobbyMutex);
    int roomId = nextRoomId++;
    gameRooms[roomId] = std::make_shared<GameRoom>(roomId);
    std::cout << "[로비] 새로운 방 생성: " << roomId << std::endl;
    return roomId;
}

bool LobbyManager::JoinRoom(int playerId, int roomId)
{
    std::lock_guard<std::mutex> lock(lobbyMutex);
    if (gameRooms.find(roomId) != gameRooms.end())
    {
        if (gameRooms[roomId]->IsFull())
        {
            return false; // 방이 가득 참
        }

        gameRooms[roomId]->AddPlayer(playerId);
        playerRoomMap[playerId] = roomId;
        std::cout << "[로비] 플레이어 " << playerId << " 방 " << roomId << " 입장" << std::endl;
        return true;
    }
    return false;
}

void LobbyManager::RemovePlayerFromRoom(int playerId)
{
    std::lock_guard<std::mutex> lock(lobbyMutex);
    if (playerRoomMap.find(playerId) != playerRoomMap.end())
    {
        int roomId = playerRoomMap[playerId];
        gameRooms[roomId]->RemovePlayer(playerId);
        playerRoomMap.erase(playerId);
        std::cout << "[로비] 플레이어 " << playerId << " 방 " << roomId << " 퇴장" << std::endl;
    }
}

std::vector<int> LobbyManager::GetPlayersInSameRoom(int playerId)
{
    std::lock_guard<std::mutex> lock(lobbyMutex);
    if (playerRoomMap.find(playerId) != playerRoomMap.end())
    {
        int roomId = playerRoomMap[playerId];
        return gameRooms[roomId]->players;
    }
    return {};
}

void LobbyManager::StartGame(int roomId)
{
    std::lock_guard<std::mutex> lock(lobbyMutex);
    if (gameRooms.find(roomId) != gameRooms.end())
    {
        gameRooms[roomId]->state = IN_GAME_STATE;
        std::cout << "[게임] 방 " << roomId << " 게임 시작!" << std::endl;
    }
}

void LobbyManager::EndGame(int roomId)
{
    std::lock_guard<std::mutex> lock(lobbyMutex);
    if (gameRooms.find(roomId) != gameRooms.end())
    {
        for (int playerId : gameRooms[roomId]->players)
        {
            playerRoomMap.erase(playerId);
            std::cout << "[게임 종료] 플레이어 " << playerId << " 로비로 이동" << std::endl;
        }
        gameRooms.erase(roomId);
    }
}
