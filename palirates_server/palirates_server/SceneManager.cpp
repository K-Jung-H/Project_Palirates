#include "stdafx.h"
#include "SceneManager.h"

SceneManager::SceneManager()
{
    
}

SceneManager::~SceneManager()
{
    
}

std::shared_ptr<Scene> SceneManager::getScene(Scene_Type type)
{
    auto it = scenes.find(type);
    return (it != scenes.end()) ? it->second : nullptr;
}

void SceneManager::addScene(Scene_Type type)
{
    if (scenes.find(type) == scenes.end())
        scenes[type] = std::make_shared<Scene>(type);
}

void SceneManager::removeScene(Scene_Type type)
{
    scenes.erase(type);
}

const std::unordered_map<Scene_Type, std::shared_ptr<Scene>>& SceneManager::getAllScenes() const
{
    return scenes;
}