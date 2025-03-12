#pragma once

#include "Resource.h"

class GameObject;

class ResourcePrefab : public Resource
{
  public:
    ResourcePrefab(UID uid, const std::string& name);
    ~ResourcePrefab() override;

    void LoadData(const std::vector<GameObject*> &objects) { gameObjects = objects; }
    GameObject* GetRootObject() const { return gameObjects[0]; }
    std::vector<GameObject*> GetGameObjectsVector() const { return gameObjects; }

  private:
    std::vector<GameObject*> gameObjects;
    // Maybe store all gameObjects and all components in two lists? With the root gameObject it is all saved anyway
};
