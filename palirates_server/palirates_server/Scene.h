#pragma once
#include <unordered_map>
#include <iostream>
#include "GameCharacter.h"
#include "Player.h"

class Scene
{
private:
    std::unordered_map<int, GameCharacter> players;

public:
    Scene() = default;

    void updatePlayerPosition(int id, float x, float y, float z, int state);

    const std::unordered_map<int, GameCharacter>& getPlayers() const 
    {
        return players; 
    }

    const GameCharacter* getPlayer(int id) const 
    {
        auto it = players.find(id);
        return it != players.end() ? &it->second : nullptr;
    }

    void addPlayer(int id) 
    {
        players[id] = GameCharacter();
    }

    void removePlayer(int id) 
    {
        players.erase(id);
    }

    void printScene();
    Player* getPlayerById(int id);
    std::unordered_map<int, Player*> playerMap;
};
