#pragma once

#include "Globals.h"
#include "LightsConfig.h"
#include "Module.h"
#include "Scene/AABBUpdatable.h"
// TMP
#include "Application.h"
#include "CameraModule.h"
#include "DebugDrawModule.h"
#include "Scene.h"

#include <map>
#include <string>
#include <unordered_map>

class GameObject;
class Component;
class RootComponent;

class SceneModule : public Module
{
  public:
    SceneModule();
    ~SceneModule() override;

    bool Init() override;
    update_status PreUpdate(float deltaTime) override;
    update_status Update(float deltaTime) override;
    update_status Render(float deltaTime) override;
    update_status RenderEditor(float deltaTime) override;
    update_status PostUpdate(float deltaTime) override;
    bool ShutDown() override;

    void CreateScene();
    void LoadScene(
        UID sceneUID, const char* sceneName, UID rootGameObject, const std::map<UID, Component*>& loadedGameComponents
    );
    void LoadGameObjects(const std::unordered_map<UID, GameObject*>& loadedGameObjects);
    void CloseScene();
    void CheckObjectsToRender();
    void SwitchPlayModeStateTo(bool wantedStatePlayMode);

    void AddGameObject(UID uid, GameObject* newGameObject) const
    {
        loadedScene != nullptr ? loadedScene->AddGameObject(uid, newGameObject) : void();
    }
    void AddComponent(UID uid, Component* newComponent) const
    {
        loadedScene != nullptr ? loadedScene->AddComponent(uid, newComponent) : void();
    }
    void RegenerateTree() { loadedScene->UpdateSpatialDataStruct(); };

    void RemoveComponent(UID componentUID) const
    {
        loadedScene != nullptr ? loadedScene->RemoveComponent(componentUID) : void();
    }

    void RenderHierarchyUI(bool& hierarchyMenu) const
    {
        loadedScene != nullptr ? loadedScene->RenderHierarchyUI(hierarchyMenu) : void();
    }

    void RemoveGameObjectHierarchy(UID gameObjectUUID) const
    {
        loadedScene != nullptr ? loadedScene->RemoveGameObjectHierarchy(gameObjectUUID) : void();
    }

    GameObject* GetGameObjectByUUID(UID gameObjectUUID) const
    {
        return loadedScene != nullptr ? loadedScene->GetGameObjectByUUID(gameObjectUUID) : nullptr;
    }
    Component* GetComponentByUID(UID componentUID) const
    {
        return loadedScene != nullptr ? loadedScene->GetComponentByUID(componentUID) : nullptr;
    }

    GameObject* GetSeletedGameObject() const
    {
        return loadedScene != nullptr ? loadedScene->GetSeletedGameObject() : nullptr;
    }

    const std::unordered_map<UID, GameObject*>* GetAllGameObjects() const
    {
        return loadedScene != nullptr ? &loadedScene->GetAllGameObjects() : nullptr;
    }
    const std::map<UID, Component*>* GetAllComponents() const
    {
        return loadedScene != nullptr ? &loadedScene->GetAllComponents() : nullptr;
    }
    UID GetGameObjectRootUID() const
    {
        return loadedScene != nullptr ? loadedScene->GetGameObjectRootUID() : CONSTANT_EMPTY_UID;
    }

    AABBUpdatable* GetTargetForAABBUpdate(UID uuid) const
    {
        return loadedScene != nullptr ? loadedScene->GetTargetForAABBUpdate(uuid) : nullptr;
    }

    UID GetSceneUID() const { return loadedScene != nullptr ? loadedScene->GetSceneUID() : CONSTANT_EMPTY_UID; }
    const char* GetSceneName() const { return loadedScene != nullptr ? loadedScene->GetSceneName() : "Not loaded"; }
    bool IsInPlayMode() const { return bInPlayMode; }
    LightsConfig* GetLightsConfig() { return loadedScene != nullptr ? loadedScene->GetLightsConfig() : nullptr; }

    bool GetDoInputs() const { return loadedScene != nullptr ? loadedScene->GetDoInputs() : false; }
    void LoadPrefab(const UID prefabUid) const { loadedScene != nullptr ? loadedScene->LoadPrefab(prefabUid) : void(); }

  private:
    Scene* loadedScene = nullptr;
    bool bInPlayMode   = false;
};
