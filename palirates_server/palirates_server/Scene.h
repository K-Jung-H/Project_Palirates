#pragma once
#include <unordered_map>
#include <iostream>
#include "GameCharacter.h"
#include "Player.h"
#include "Monster.h"

class Scene
{
private:
    std::unordered_map<int, GameCharacter> players;

public:
   
    Scene() = default;

    void updatePlayerPosition(int clientId, float x, float y, float z, float lookX, float lookY, float lookZ, EState state);

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
        //players[id] = GameCharacter();
    }

    void removePlayer(int id) 
    {
        players.erase(id);
    }

    const std::unordered_map<int, Monster>& getMonsters() const { return monsterMap; }
    void addMonster(int id, const Monster& monster);

    void printScene();
    Player* getPlayerById(int id);
    std::unordered_map<int, Player*> playerMap;
    std::unordered_map<int, Monster> monsterMap;
};
