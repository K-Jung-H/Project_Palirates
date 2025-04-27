#pragma once
#include <unordered_map>
#include <iostream>
#include "GameCharacter.h"

class Scene
{
private:
    std::unordered_map<int, GameCharacter> players;

public:
    Scene() = default;

    void updatePlayerPosition(int playerId, float x, float y, float z, int state);
    void printScene();
};
