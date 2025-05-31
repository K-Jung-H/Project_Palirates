#pragma once
#include <unordered_map>
#include "Scene.h"
#include <vector>

class Scene_Manager
{
private:
    std::unordered_map<int, Scene> scenes;

public:
    void addScene(int clientId);
    Scene* getScene(int clientId);
   std::unordered_map<int, Scene>& getAllScenes() { return scenes; }
};
