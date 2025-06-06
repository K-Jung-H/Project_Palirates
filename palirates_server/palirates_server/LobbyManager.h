#pragma once
#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>

enum GameState
{
    LOBBY_STATE,
    IN_GAME_STATE
};

struct GameRoom
{
    int roomId;
    std::vector<int> players;
    GameState state;
    const int maxPlayers = 6;

    GameRoom(int id) : roomId(id), state(LOBBY_STATE) {}

    bool AddPlayer(int playerId)
    {
        if (players.size() < maxPlayers)
        {
            players.push_back(playerId);
            return true;
        }
        return false;
    }

    void RemovePlayer(int playerId)
    {
        players.erase(std::remove(players.begin(), players.end(), playerId), players.end());
    }

    bool IsFull() const
    {
        return players.size() >= maxPlayers;
    }
};

class LobbyManager
{
private:
    std::unordered_map<int, std::shared_ptr<GameRoom>> gameRooms;
    std::unordered_map<int, int> playerRoomMap;
    int nextRoomId;
    std::mutex lobbyMutex;

public:
    LobbyManager();

    int CreateRoom();
    bool JoinRoom(int playerId, int roomId);
    void RemovePlayerFromRoom(int playerId);
    std::vector<int> GetPlayersInSameRoom(int playerId);
    void StartGame(int roomId);
    void EndGame(int roomId);
};
