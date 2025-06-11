#include "stdafx.h"
#include "SceneManager.h"

std::shared_ptr<Scene> SceneManager::getScene(int clientId)
{
    auto it = scenes.find(clientId);
    if (it != scenes.end())
        return it->second;
    return nullptr;
}

void SceneManager::addScene(int clientId)
{
    scenes[clientId] = std::make_shared<Scene>();
}

const std::unordered_map<int, std::shared_ptr<Scene>>& SceneManager::getAllScenes() const
{
    return scenes;
}