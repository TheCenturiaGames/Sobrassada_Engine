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
#include <tuple>

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
    void CloseScene();
    void LoadScene(
        UID sceneUID, const char* sceneName, UID rootGameObject, const std::map<UID, Component*>& loadedGameComponents
    );
    void LoadGameObjects(const std::unordered_map<UID, GameObject*>& loadedGameObjects);
    void SwitchPlayMode(bool play);

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

    void RemoveGameObjectHierarchy(UID gameObjectUID) const
    {
        loadedScene != nullptr ? loadedScene->RemoveGameObjectHierarchy(gameObjectUID) : void();
    }

    GameObject* GetGameObjectByUID(UID gameObjectUID) const
    {
        return loadedScene != nullptr ? loadedScene->GetGameObjectByUID(gameObjectUID) : nullptr;
    }
    Component* GetComponentByUID(UID componentUID) const
    {
        return loadedScene != nullptr ? loadedScene->GetComponentByUID(componentUID) : nullptr;
    }

    GameObject* GetSelectedGameObject() const
    {
        return loadedScene != nullptr ? loadedScene->GetSelectedGameObject() : nullptr;
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

    bool IsSceneLoaded() const { return loadedScene != nullptr; }
    bool IsInPlayMode() const { return isPlayMode; }
    UID GetSceneUID() const { return loadedScene != nullptr ? loadedScene->GetSceneUID() : CONSTANT_EMPTY_UID; }
    const char* GetSceneName() const { return loadedScene != nullptr ? loadedScene->GetSceneName() : "Not loaded"; }
    LightsConfig* GetLightsConfig() { return loadedScene != nullptr ? loadedScene->GetLightsConfig() : nullptr; }
    const std::tuple<float, float>& GetWindowPosition() const { return loadedScene->GetWindowPosition(); };
    const std::tuple<float, float>& GetWindowSize() const { return loadedScene->GetWindowSize(); };
    const std::tuple<float, float>& GetMousePosition() const { return loadedScene->GetMousePosition(); };

    bool GetDoInputsScene() const { return loadedScene != nullptr ? loadedScene->GetDoInputs() && !isPlayMode : false; }
    bool GetDoInputsGame() const { return loadedScene != nullptr ? loadedScene->GetDoInputs() && isPlayMode : false; }

  private:
    Scene* loadedScene = nullptr;
    bool isPlayMode    = false;
};
