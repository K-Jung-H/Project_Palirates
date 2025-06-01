#pragma once
#include <unordered_map>
#include <iostream>
#include "GameCharacter.h"
#include "Player.h"
#include "Monster.h"

enum SceneState
{
    Character_Select,
    Game_Board,
    In_Stage
};



class Scene
{
private:
    std::unordered_map<int, GameCharacter> players;
    SceneState state;

public:

    Scene() : state(In_Stage) {}
    SceneState getState() const { return state; }
    void setState(SceneState newState) { state = newState; }

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

    GameCharacter* getPlayer(int id);

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
    void addMonster(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp, int state, Monster_Type type);

    void printScene();
    Player* getPlayerById(int id);
    std::unordered_map<int, Player*> playerMap;
    std::unordered_map<int, Monster> monsterMap;
    void updatePlayerAnimation(int playerId, const std::vector<float>& trackPositions, const std::vector<float>& trackWeights);
};
