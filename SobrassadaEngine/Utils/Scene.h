#pragma once

#include "Globals.h"
#include "LightsConfig.h"

#include <map>
#include <unordered_map>

class GameObject;
class Component;
class RootComponent;
class AABBUpdatable;
class Octree;

class Scene
{
  public:
    Scene(UID sceneUID, const char* sceneName, UID rootGameObject);
    ~Scene();

    void LoadComponents(const std::map<UID, Component*>& loadedGameComponents);
    void LoadGameObjects(const std::unordered_map<UID, GameObject*>& loadedGameObjects);

    update_status Render(float deltaTime);
    update_status RenderEditor(float deltaTime);
    void RenderEditorControl();
    void RenderScene();
    void RenderGame();
    void RenderSelectedGameObjectUI();
    void RenderHierarchyUI(bool& hierarchyMenu);

    void UpdateSpatialDataStruct();

    void AddGameObject(UID uid, GameObject* newGameObject) { gameObjectsContainer.insert({uid, newGameObject}); }
    void AddComponent(UID uid, Component* newComponent) { gameComponents.insert({uid, newComponent}); }
    void RemoveGameObjectHierarchy(UID gameObjectUUID);
    void RemoveComponent(UID componentUID);

    const char* GetSceneName() const { return sceneName.c_str(); }
    UID GetSceneUID() const { return sceneUID; }
    UID GetGameObjectRootUID() const { return gameObjectRootUID; }
    GameObject* GetSelectedGameObject() { return GetGameObjectByUID(selectedGameObjectUID); }

    const std::unordered_map<UID, GameObject*>& GetAllGameObjects() const { return gameObjectsContainer; }
    const std::map<UID, Component*>& GetAllComponents() const { return gameComponents; }

    GameObject* GetGameObjectByUID(UID gameObjectUID); // TODO: Change when filesystem defined
    Component* GetComponentByUID(UID componentUID);

    AABBUpdatable* GetTargetForAABBUpdate(UID uid);
    LightsConfig* GetLightsConfig() { return lightsConfig; }

    bool GetStopPlay() const { return stopPlay; }

    void SetStopPlay(bool stop) { stopPlay = stop; }

  private:
    void CreateSpatialDataStruct();
    void CheckObjectsToRender(std::vector<GameObject*>& outRenderGameObjects) const;

  private:
    bool stopPlay = false;
    std::string sceneName;
    UID sceneUID;
    UID gameObjectRootUID;
    UID selectedGameObjectUID;

    std::map<UID, Component*> gameComponents; // TODO Move components to individual gameObjects
    std::unordered_map<UID, GameObject*> gameObjectsContainer;

    LightsConfig* lightsConfig = nullptr;
    Octree* sceneOctree        = nullptr;
};
