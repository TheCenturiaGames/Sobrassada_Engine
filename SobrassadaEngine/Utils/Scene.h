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
    void Save(
        rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator, UID saveUID = INVALID_UID,
        const std::string& newName = ""
    ) const;

    void LoadComponents() const;
    void LoadGameObjects(const std::unordered_map<UID, GameObject*>& loadedGameObjects);
    void LoadModel(const UID modelUID);
    void LoadPrefab(const UID prefabUid);

    update_status Render(float deltaTime) const;
    update_status RenderEditor(float deltaTime);

    void RenderEditorControl(bool& editorControlMenu);
    void RenderScene();
    void RenderSelectedGameObjectUI();
    void RenderHierarchyUI(bool& hierarchyMenu);

    void UpdateSpatialDataStruct();

    void AddGameObject(UID uid, GameObject* newGameObject) { gameObjectsContainer.insert({uid, newGameObject}); }
    void RemoveGameObjectHierarchy(UID gameObjectUUID);

    const char* GetSceneName() const { return sceneName; }
    UID GetSceneUID() const { return sceneUID; }
    UID GetGameObjectRootUID() const { return gameObjectRootUID; }
    GameObject* GetSelectedGameObject() { return GetGameObjectByUID(selectedGameObjectUID); }

    const std::unordered_map<UID, GameObject*>& GetAllGameObjects() const { return gameObjectsContainer; }
    const std::unordered_map<UID, Component*> GetAllComponents() const;

    GameObject* GetGameObjectByUID(UID gameObjectUID); // TODO: Change when filesystem defined

    LightsConfig* GetLightsConfig() { return lightsConfig; }
    const Octree* GetOctree() const { return sceneOctree; }

    bool GetDoInputs() const { return doInputs; }
    bool GetStopPlaying() const { return stopPlaying; }

    const std::tuple<float, float>& GetWindowPosition() const { return sceneWindowPosition; };
    const std::tuple<float, float>& GetWindowSize() const { return sceneWindowSize; };
    const std::tuple<float, float>& GetMousePosition() const { return mousePosition; };

    void SetSelectedGameObject(UID newSelectedGameObject) { selectedGameObjectUID = newSelectedGameObject; };

    void SetStopPlaying(bool stop) { stopPlaying = stop; }

  private:
    void CreateSpatialDataStruct();
    void CheckObjectsToRender(std::vector<GameObject*>& outRenderGameObjects) const;

  private:
    char sceneName[64];
    const UID sceneUID;
    UID gameObjectRootUID;
    UID selectedGameObjectUID;
    bool stopPlaying = false;
    bool doInputs    = false;

    std::unordered_map<UID, GameObject*> gameObjectsContainer;

    LightsConfig* lightsConfig                   = nullptr;
    Octree* sceneOctree                          = nullptr;

    // IMGUI WINDOW DATA
    std::tuple<float, float> sceneWindowPosition = std::make_tuple(0.f, 0.f);
    std::tuple<float, float> sceneWindowSize     = std::make_tuple(0.f, 0.f);
    std::tuple<float, float> mousePosition       = std::make_tuple(0.f, 0.f);
};
