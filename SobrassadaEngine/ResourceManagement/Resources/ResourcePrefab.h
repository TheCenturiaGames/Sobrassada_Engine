#pragma once

#include "Resource.h"

class GameObject;

class ResourcePrefab : public Resource
{
  public:
    ResourcePrefab(UID uid, const std::string& name);
    ~ResourcePrefab() override;

    void LoadData(GameObject* gameObject) { rootGameObject = gameObject; }

  private:
    GameObject* rootGameObject;
    // Maybe store all gameObjects and all components in two lists? With the root gameObject it is all saved anyway
};
