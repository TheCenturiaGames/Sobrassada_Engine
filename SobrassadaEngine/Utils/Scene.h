#pragma once

#include "Globals.h"
#include "LightsConfig.h"

#include <map>
#include <tuple>
#include <unordered_map>

class GameObject;
class Component;
class RootComponent;
class Octree;

class Scene
{
  public:
    Scene(const char* sceneName);
    Scene(const rapidjson::Value& initialState, UID loadedSceneUID);

    ~Scene();

    void Init();
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;

    void LoadComponents() const;
    void LoadGameObjects(const std::unordered_map<UID, GameObject*>& loadedGameObjects);

    update_status Render(float deltaTime) const;
    update_status RenderEditor(float deltaTime);
    void RenderScene();
    void RenderSelectedGameObjectUI();
    void RenderHierarchyUI(bool& hierarchyMenu);

    void UpdateSpatialDataStruct();

    void AddGameObject(UID uid, GameObject* newGameObject) { gameObjectsContainer.insert({uid, newGameObject}); }
    void RemoveGameObjectHierarchy(UID gameObjectUUID);

    const char* GetSceneName() const { return sceneName; }
    UID GetSceneUID() const { return sceneUID; }
    UID GetGameObjectRootUID() const { return gameObjectRootUID; }
    GameObject* GetSeletedGameObject() { return GetGameObjectByUID(selectedGameObjectUID); }

    const std::unordered_map<UID, GameObject*>& GetAllGameObjects() const { return gameObjectsContainer; }

    GameObject* GetGameObjectByUID(UID gameObjectUUID); // TODO: Change when filesystem defined

    LightsConfig* GetLightsConfig() { return lightsConfig; }

    const std::tuple<float, float>& GetWindowPosition() const { return sceneWindowPosition; };
    const std::tuple<float, float>& GetWindowSize() const { return sceneWindowSize; };
    const std::tuple<float, float>& GetMousePosition() const { return mousePosition; };
    const Octree* GetOctree() const { return sceneOctree; }

    void SetSelectedGameObject(UID newSelectedGameObject) { selectedGameObjectUID = newSelectedGameObject; };

    bool GetDoInputs() const { return doInputs; }
    void LoadModel(const UID modelUID);

    const std::unordered_map<UID, Component*> GetAllComponents() const;

    const std::unordered_map<UID, Component*> GetAllComponents() const;

  private:
    void CreateSpatialDataStruct();
    void CheckObjectsToRender(std::vector<GameObject*>& outRenderGameObjects) const;

  private:
    char sceneName[64];
    const UID sceneUID;
    UID gameObjectRootUID;
    UID selectedGameObjectUID;

    std::unordered_map<UID, GameObject*> gameObjectsContainer;

    LightsConfig* lightsConfig                   = nullptr;
    Octree* sceneOctree                          = nullptr;

    // IMGUI WINDOW DATA
    std::tuple<float, float> sceneWindowPosition = std::make_tuple(0.f, 0.f);
    std::tuple<float, float> sceneWindowSize     = std::make_tuple(0.f, 0.f);
    std::tuple<float, float> mousePosition       = std::make_tuple(0.f, 0.f);
    bool doInputs                                = false;
};
