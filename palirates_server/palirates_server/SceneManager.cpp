#include "SceneManager.h"

void Scene_Manager::addScene(int clientId)
{
    scenes.emplace(clientId, Scene());
}

Scene* Scene_Manager::getScene(int clientId)
{
    if (scenes.find(clientId) != scenes.end())
        return &scenes[clientId];
    return nullptr;
}
