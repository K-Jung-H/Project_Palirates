#include "SceneManager.h"

void Scene_Manager::addScene(int clientId)
{
    bool isFirstScene = scenes.empty();

    scenes.emplace(clientId, Scene());

    Scene* scene = getScene(clientId);
    if (!scene) return;

    if (isFirstScene)
    {
        for (int i = 0; i < 10; ++i)
        {
            int monsterId = 10000 + i;
            float x = 100.0f + i * 10.0f;
            float z = 200.0f + i * 10.0f;
            float y = 0.0f;
            float lookX = 0.0f, lookY = 0.0f, lookZ = 1.0f;

             scene->addMonster(monsterId, x, y, z, lookX, lookY, lookZ, 100, 0, Monster_Type::Fishman);
        }
    }
}

Scene* Scene_Manager::getScene(int clientId)
{
    if (scenes.find(clientId) != scenes.end())
        return &scenes[clientId];
    return nullptr;
}