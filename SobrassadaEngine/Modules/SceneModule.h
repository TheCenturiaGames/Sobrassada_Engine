#pragma once

#include "Globals.h"
#include "LightsConfig.h"
#include "Module.h"
// TMP
#include "Scene/Components/CameraComponent.h"
#include "CameraModule.h"
#include "Scene.h"

#include <map>
#include <string>
#include <tuple>
#include <unordered_map>

class GameObject;
class Component;
class RootComponent;
class Octree;

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

    void LoadModel(const UID modelUID) { loadedScene != nullptr ? loadedScene->LoadModel(modelUID) : void(); }

    void SwitchPlayMode(bool play);

    void AddGameObject(UID uid, GameObject* newGameObject) const
    {
        loadedScene != nullptr ? loadedScene->AddGameObject(uid, newGameObject) : void();
    }

    void RegenerateTree() const { loadedScene->UpdateSpatialDataStruct(); }

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

    GameObject* GetSelectedGameObject() const
    {
        return loadedScene != nullptr ? loadedScene->GetSelectedGameObject() : nullptr;
    }

    const std::unordered_map<UID, GameObject*>* GetAllGameObjects() const
    {
        return loadedScene != nullptr ? &loadedScene->GetAllGameObjects() : nullptr;
    }

    UID GetGameObjectRootUID() const
    {
        return loadedScene != nullptr ? loadedScene->GetGameObjectRootUID() : INVALID_UID;
    }

    bool IsSceneLoaded() const { return loadedScene != nullptr; }
    

    Scene* GetScene() const { return loadedScene; }
    UID GetSceneUID() const { return loadedScene != nullptr ? loadedScene->GetSceneUID() : INVALID_UID; }
    const char* GetSceneName() const { return loadedScene != nullptr ? loadedScene->GetSceneName() : "Not loaded"; }
    LightsConfig* GetLightsConfig() { return loadedScene != nullptr ? loadedScene->GetLightsConfig() : nullptr; }
    bool GetInPlayMode() { return inPlayMode; }
    void SetMainCamera(CameraComponent* camera) { loadedScene->SetMainCamera(camera); }
    CameraComponent* GetMainCamera() { return loadedScene->GetMainCamera(); }
    const std::tuple<float, float>& GetWindowPosition() const { return loadedScene->GetWindowPosition(); };
    const std::tuple<float, float>& GetWindowSize() const { return loadedScene->GetWindowSize(); };
    const std::tuple<float, float>& GetMousePosition() const { return loadedScene->GetMousePosition(); };

    bool GetDoInputs() const { return loadedScene != nullptr ? loadedScene->GetDoInputs() : false; }
    bool GetDoInputsScene() const { return loadedScene != nullptr ? loadedScene->GetDoInputs() && !inPlayMode : false; }
    bool GetDoInputsGame() const { return loadedScene != nullptr ? loadedScene->GetDoInputs() && inPlayMode : false; }

    
    Octree* GetSceneOctree() const { return loadedScene != nullptr ? loadedScene->GetOctree() : nullptr; }

  private:
    Scene* loadedScene = nullptr;
    const std::string scenePath;
    bool inPlayMode = false;
};
