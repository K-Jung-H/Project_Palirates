#pragma once
#include <unordered_map>
#include <memory>
#include "Scene.h"

class SceneManager
{
private:
    std::unordered_map<Scene_Type, std::shared_ptr<Scene>> scenes;

public:
    SceneManager();
    ~SceneManager();

    std::shared_ptr<Scene> getScene(Scene_Type type);
    void addScene(Scene_Type type);
    void removeScene(Scene_Type type);
    const std::unordered_map<Scene_Type, std::shared_ptr<Scene>>& getAllScenes() const;
};