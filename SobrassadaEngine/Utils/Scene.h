#pragma once

#include "Globals.h"
#include "LightsConfig.h"

#include <map>
#include <tuple>
#include <unordered_map>

class GameObject;
class Component;
class RootComponent;
class AABBUpdatable;
class Octree;
class CameraComponent;

class Scene
{
  public:
    Scene(UID sceneUID, const char* sceneName, UID rootGameObject);
    ~Scene();

    void Save() const;
    void LoadComponents(const std::map<UID, Component*>& loadedGameComponents);
    void LoadGameObjects(const std::unordered_map<UID, GameObject*>& loadedGameObjects);

    update_status Update(float deltaTime);
    update_status Render(float deltaTime);
    update_status RenderEditor(float deltaTime);
    void RenderScene();
    void RenderSelectedGameObjectUI();
    void RenderHierarchyUI(bool& hierarchyMenu);

    void UpdateSpatialDataStruct();

    void AddGameObject(UID uid, GameObject* newGameObject) { gameObjectsContainer.insert({uid, newGameObject}); }
    void AddComponent(UID uid, Component* newComponent) { gameComponents.insert({uid, newComponent}); }
    void RemoveGameObjectHierarchy(UID gameObjectUUID);
    void RemoveComponent(UID componentUID);

    const char* GetSceneName() const { return sceneName.c_str(); }
    UID GetSceneUID() const { return sceneUID; }
    UID GetGameObjectRootUID() const { return gameObjectRootUUID; }
    GameObject* GetSeletedGameObject() { return GetGameObjectByUUID(selectedGameObjectUUID); }

    const std::unordered_map<UID, GameObject*>& GetAllGameObjects() const { return gameObjectsContainer; }
    const std::map<UID, Component*>& GetAllComponents() const { return gameComponents; }

    GameObject* GetGameObjectByUUID(UID gameObjectUUID); // TODO: Change when filesystem defined
    Component* GetComponentByUID(UID componentUID);

    AABBUpdatable* GetTargetForAABBUpdate(UID uuid);
    LightsConfig* GetLightsConfig() { return lightsConfig; }
    void SetMainCamera(CameraComponent* camera) { mainCamera = camera; }
    CameraComponent* GetMainCamera() { return mainCamera; }

    const std::tuple<float, float>& GetWindowPosition() const { return sceneWindowPosition; };
    const std::tuple<float, float>& GetWindowSize() const { return sceneWindowSize; };
    const std::tuple<float, float>& GetMousePosition() const { return mousePosition; };
    const Octree* GetOctree() const { return sceneOctree; }

    void SetSelectedGameObject(UID newSelectedGameObject) { selectedGameObjectUUID = newSelectedGameObject; };
    
    bool GetDoInputs() const { return doInputs; }

  private:
    void CreateSpatialDataStruct();
    void CheckObjectsToRender(std::vector<GameObject*>& outRenderGameObjects) const;

  private:
    std::string sceneName;
    UID sceneUID;
    UID gameObjectRootUUID;
    UID selectedGameObjectUUID;
    CameraComponent* mainCamera;

    std::map<UID, Component*> gameComponents; // TODO Move components to individual gameObjects
    std::unordered_map<UID, GameObject*> gameObjectsContainer;

    LightsConfig* lightsConfig                   = nullptr;
    Octree* sceneOctree                          = nullptr;

    // IMGUI WINDOW DATA
    std::tuple<float, float> sceneWindowPosition = std::make_tuple(0.f, 0.f);
    std::tuple<float, float> sceneWindowSize     = std::make_tuple(0.f, 0.f);
    std::tuple<float, float> mousePosition       = std::make_tuple(0.f, 0.f);
    bool doInputs = false;
};
