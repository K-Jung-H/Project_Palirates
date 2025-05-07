#include "ObjectManager.h"
#include "Player.h"

void Object_Manager::addObject(int objectId, float x, float y, float z, float lookX, float lookY, float lookZ, EState state)
{
    objects.emplace(objectId, GameCharacter(objectId, x, y, z, lookX, lookY, lookZ, state));
}

void Object_Manager::updateObject(int objectId, float x, float y, float z, float lookX, float lookY, float lookZ, EState state)
{
    if (objects.find(objectId) != objects.end())
    {
        objects[objectId].setPosition(x, y, z);
        objects[objectId].setLookVec(lookX, lookY, lookZ);
        objects[objectId].setState(state);
    }
}
