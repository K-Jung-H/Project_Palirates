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
#include "SceneManager.h"
#include "Logger.h"
#include "Player.h"

#pragma comment(lib, "ws2_32.lib")

struct ClientSession
{
    SOCKET socket;
    bool is_connected = true;
    std::chrono::steady_clock::time_point lastActiveTime;
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
    void BroadcastAllStates();
    void Server_Update();

    void DisconnectClient(int clientId);
    void ReleaseClientId(int clientId);
    int GetNewClientId();

    void BroadcastPacket(const std::string& packet, int senderId);

    void SetActiveScene(const Scene_Type scene_type);
    std::shared_ptr<Scene> GetActiveScene();

    void CleanupInactiveClients();
    void addPlayerToScene(const Scene_Type scene_type, int clientId, std::shared_ptr<Player> player);
    void removePlayerFromAllScenes(int clientId);


private:
    SOCKET listenSocket;
    Logger logger;

    std::unordered_map<int, ClientSession> clients;
    std::unordered_map<Scene_Type, std::shared_ptr<Scene>> scenes;

    std::unordered_map<int, int> characterSelections;
    std::unordered_set<int> lockedCharacterIds;

    std::priority_queue<int, std::vector<int>, std::greater<int>> availableIds;
    std::unordered_set<int> activeClientIds;

    std::mutex idMutex;
    std::mutex clientsMutex;
    std::mutex characterMutex;
    std::mutex activeSceneMutex;

    std::shared_ptr<Scene> activeScene;

    int nextClientId = 0;
    bool allSelectedSent = false;

    void HandleSceneBroadcast();
    void BroadcastLobbyScene(const std::shared_ptr<Scene>& scene);
    void BroadcastStage1Scene(const std::shared_ptr<Scene>& scene);
    void BroadcastStage2Scene(const std::shared_ptr<Scene>& scene);
    void BroadcastBoardScene(const std::shared_ptr<Scene>& scene);

    void HandleLobbyPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens);
    void HandleStage1Packet(int clientId, const std::string& command, const std::vector<std::string>& tokens);
    void HandleBoardPacket(int clientId, const std::string& command, const std::vector<std::string>& tokens);
};
