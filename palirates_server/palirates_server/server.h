#pragma once
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <memory>
#include <string>
#include <vector>
#include "Scene.h"
#include "Logger.h"
#include "Player.h"
#include "Timer.h"

#pragma comment(lib, "ws2_32.lib")

struct ClientSession
{
    SOCKET socket;
    bool is_connected = true;
    std::chrono::steady_clock::time_point lastActiveTime;

    std::string lastReceivedPacket;
    std::string lastSentPacket;
    std::mutex packetLogMutex;

    ClientSession(SOCKET sock)
        : socket(sock) {}

    ~ClientSession()
    {

    };
};

class Server
{
public:
    Server(int port);
    ~Server();

    void Start();
    void AcceptClients();
    void ProcessClientPackets(SOCKET clientSocket, int clientId);
    void Broadcast_Scene_State_All();
    void Server_Update();

    void DisconnectClient(int clientId);
    void ReleaseClientId(int clientId);
    int GetNewClientId();

    void BroadcastPacket(const std::string& packet);

    void SetActiveScene(const Scene_Type scene_type);
    std::shared_ptr<Scene> GetActiveScene();

    void CleanupInactiveClients();
    void removePlayerFromAllScenes(int clientId);

    void Send_Custom(std::shared_ptr<ClientSession> session, const std::string& packet, bool saveLog);
    void PrintClientDebugInfo();

private:
    SOCKET listenSocket;
    Logger logger;

    std::unordered_map<int, shared_ptr<ClientSession>> clients;
    std::unordered_map<Scene_Type, std::shared_ptr<Scene>> scenes;

    std::priority_queue<int, std::vector<int>, std::greater<int>> availableIds;
    std::unordered_set<int> activeClientIds;

    std::mutex idMutex;
    std::mutex clientsMutex;
    std::mutex activeSceneMutex;

    std::atomic<int> activeClientCount = 0;


    CGameTimer m_gameTimer;
    std::shared_ptr<Scene> activeScene;

    int nextClientId = 0;
    bool allSelectedSent = false;
    bool HandleSceneBroadcast(std::string& outPacket);
    std::string Build_LobbyScene_Packet(const std::shared_ptr<Lobby_Scene>& lobby);
    std::string Build_BoardScene_Packet(const std::shared_ptr<Board_Scene>& board);
    std::string Build_Stage_1_Scene_Packet(const std::shared_ptr<Stage_Scene>& stage);
    std::string Build_Stage_2_Scene_Packet(const std::shared_ptr<Stage_Scene>& stage);

    void HandleLobbyPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens);
    void HandleBoardPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens);
    void HandleStage1Packet(int clientId, const std::string& command, const std::vector<std::string>& tokens);
    void HandleStage2Packet(int clientId, const std::string& command, const std::vector<std::string>& tokens);
};
