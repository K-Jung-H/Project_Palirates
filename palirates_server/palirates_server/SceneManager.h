#pragma once
#include <unordered_map>
#include <memory>
#include "Scene.h"

class SceneManager
{
private:
    std::unordered_map<int, std::shared_ptr<Scene>> scenes;

public:
    std::shared_ptr<Scene> getScene(int clientId);
    void addScene(int clientId);
    void removeScene(int clientId);
    const std::unordered_map<int, std::shared_ptr<Scene>>& getAllScenes() const;
};