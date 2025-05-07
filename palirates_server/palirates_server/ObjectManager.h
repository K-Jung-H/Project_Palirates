#pragma once
#include <unordered_map>
#include "GameCharacter.h"
#include "Player.h"

class Object_Manager
{
private:
    std::unordered_map<int, GameCharacter> objects;

public:
    void addObject(int objectId, float x, float y, float z, float lookX, float lookY, float lookZ, EState state);
    void updateObject(int objectId, float x, float y, float z, float lookX, float lookY, float lookZ, EState state);
};
