#pragma once

#include "Globals.h"
#include "LightsConfig.h"
#include "Module.h"
// TMP
#include "CameraModule.h"
#include "Scene.h"

#include <map>
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
    void LoadScene(const rapidjson::Value& initialState, bool forceReload = false);
    void LoadGameObjects(const std::unordered_map<UID, GameObject*>& loadedGameObjects);
    void LoadComponents(const std::map<UID, Component*>& loadedGameComponents);
    void CloseScene();
    void CheckObjectsToRender();
    void SwitchPlayModeStateTo(bool wantedStatePlayMode);

    void AddGameObject(UID uid, GameObject* newGameObject) const
    {
        loadedScene != nullptr ? loadedScene->AddGameObject(uid, newGameObject) : void();
    }
    
    void RegenerateTree() const { loadedScene->UpdateSpatialDataStruct(); }

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
        return loadedScene != nullptr ? loadedScene->GetGameObjectByUID(gameObjectUUID) : nullptr;
    }

    GameObject* GetSeletedGameObject() const
    {
        return loadedScene != nullptr ? loadedScene->GetSeletedGameObject() : nullptr;
    }

    const std::unordered_map<UID, GameObject*>* GetAllGameObjects() const
    {
        return loadedScene != nullptr ? &loadedScene->GetAllGameObjects() : nullptr;
    }
    
    UID GetGameObjectRootUID() const
    {
        return loadedScene != nullptr ? loadedScene->GetGameObjectRootUID() : CONSTANT_EMPTY_UID;
    }

    UID GetSceneUID() const { return loadedScene != nullptr ? loadedScene->GetSceneUID() : CONSTANT_EMPTY_UID; }
    const char* GetSceneName() const { return loadedScene != nullptr ? loadedScene->GetSceneName() : "Not loaded"; }
    bool IsInPlayMode() const { return bInPlayMode; }
    LightsConfig* GetLightsConfig() const { return loadedScene != nullptr ? loadedScene->GetLightsConfig() : nullptr; }

    Scene* GetScene() const { return loadedScene; }

    bool GetDoInputs() const { return loadedScene != nullptr ? loadedScene->GetDoInputs() : false; }

  private:
    Scene* loadedScene = nullptr;
    bool bInPlayMode   = false;
    const std::string sceneLibraryPath;
};
