#pragma once
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include <queue>
#include "SceneManager.h"
#include "DatabaseManager.h"
#include "Logger.h"
#include "Player.h"

#define NOMINMAX
#pragma comment(lib, "ws2_32.lib")


struct ClientSession
{
    SOCKET socket;
    bool is_connected = true;
    std::chrono::steady_clock::time_point lastPongTime;
};


class Server
{
private:
    // 씬 이름별로 단 하나만 관리
    std::unordered_map<std::string, std::shared_ptr<Scene>> scenes;

    std::unordered_map<int, ClientSession> clients;
    SOCKET listenSocket;
    Logger logger;

    std::unordered_map<int, int> controllerIdByScene;
    std::unordered_map<std::string, std::string> ParseKeyValueFields(const std::vector<std::string>& tokens, size_t startIndex);

    std::priority_queue<int, std::vector<int>, std::greater<int>> availableIds;
    std::unordered_set<int> activeClientIds;
    std::mutex idMutex;
    std::mutex clientsMutex;
    std::mutex characterMutex;

    int GetControllerId(std::shared_ptr<Scene> scene);

    bool allSelectedSent = false;

public:
    Server(int port);
    ~Server();

    int nextClientId = 0;

    void Start();
    void AcceptClients();
    void Server_Update();
    void ProcessClientPackets(SOCKET clientSocket, int clientId);
    void BroadcastPacket(const std::string& packet, int senderId);
    void SendInitialStates(int clientId);
    void BroadcastAllStates();
    void NotifyExistingPlayersAboutNew(int clientId);

    std::unordered_map<int, int> characterSelections;
    std::unordered_set<int> lockedCharacterIds;
    void CheckClientLiveness();

    static void BroadcastCharacterSelect(Server* pServer);

    int GetNewClientId();
    void DisconnectClient(int clientId);
    void ReleaseClientId(int clientId);

    // ==== 씬/플레이어 관리 ====
    void addPlayerToScene(const std::string& sceneName, int clientId, std::shared_ptr<Player> player);
    void removePlayerFromAllScenes(int clientId);

    float shipX = 0.0f, shipY = 0.0f, shipZ = 0.0f;
    float shipLookX = 0.0f, shipLookY = 1.0f, shipLookZ = 0.0f;
    int currentShipControllerId = -1;
};