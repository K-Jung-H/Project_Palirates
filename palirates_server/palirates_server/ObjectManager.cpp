#include "ObjectManager.h"

void Object_Manager::addObject(int objectId, float x, float y, float z)
{
    objects.emplace(objectId, GameCharacter(objectId, x, y, z));
}

void Object_Manager::updateObject(int objectId, float x, float y, float z)
{
    if (objects.find(objectId) != objects.end())
    {
        objects[objectId].setPosition(x, y, z);
    }
}
