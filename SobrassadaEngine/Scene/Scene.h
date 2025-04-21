#pragma once

#include "Globals.h"
#include "LightsConfig.h"

#include "Math/float4x4.h"
#include <functional>
#include <map>
#include <tuple>
#include <unordered_map>
#include <vector>

class GameObject;
class Component;
class RootComponent;
class Octree;
class ResourcePrefab;
class Quadtree;
class CameraComponent;
class CharacterControllerComponent;
enum class SaveMode;
enum MobilitySettings;

class Scene
{
  public:
    Scene(const char* sceneName);
    Scene(const rapidjson::Value& initialState, UID loadedSceneUID);

    ~Scene();

    void Init();
    void Save(
        rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator, SaveMode saveMode,
        UID newUID = INVALID_UID, const char* newName = nullptr
    );

    void LoadModel(const UID modelUID);
    void LoadPrefab(
        const UID prefabUid, const ResourcePrefab* prefab = nullptr, const float4x4& transform = float4x4::identity
    );
    void OverridePrefabs(UID prefabUID);

    update_status Update(float deltaTime);
    update_status Render(float deltaTime);
    update_status RenderEditor(float deltaTime);

    void RenderEditorControl(bool& editorControlMenu);
    void RenderScene(float deltaTime, CameraComponent* camera);
    void RenderSceneToFrameBuffer();
    void RenderSelectedGameObjectUI();
    void RenderHierarchyUI(bool& hierarchyMenu);

    bool IsStaticModified() const { return staticModified; };
    bool IsDynamicModified() const { return dynamicModified; };
    bool IsMultiselecting() const { return selectedGameObjects.size() > 0; };
    bool IsSceneFocused() const { return isFocused; };

    void UpdateStaticSpatialStructure();
    void UpdateDynamicSpatialStructure();

    void AddGameObject(UID uid, GameObject* newGameObject) { gameObjectsContainer.insert({uid, newGameObject}); }
    void RemoveGameObjectHierarchy(UID gameObjectUUID);

    void AddGameObjectToUpdate(GameObject* gameObject);
    void UpdateGameObjects();
    void ClearGameObjectsToUpdate();

    void AddGameObjectToSelection(UID gameObject, UID gameObjectParent);
    void ClearObjectSelection();
    void DeleteMultiselection();

    const std::string& GetSceneName() const { return sceneName; }
    UID GetSceneUID() const { return sceneUID; }
    UID GetGameObjectRootUID() const { return gameObjectRootUID; }
    GameObject* GetSelectedGameObject() { return GetGameObjectByUID(selectedGameObjectUID); }
    UID GetSelectedGameObjectUID() const { return selectedGameObjectUID; }

    const std::unordered_map<UID, GameObject*>& GetAllGameObjects() const { return gameObjectsContainer; }
    const std::vector<Component*> GetAllComponents() const;

    GameObject* GetGameObjectByUID(UID gameObjectUID); // TODO: Change when filesystem defined

    LightsConfig* GetLightsConfig() const { return lightsConfig; }
    CameraComponent* GetMainCamera() const { return mainCamera; }

    template <typename T> std::vector<T*> GetEnabledComponentsOfType() const;

    bool GetDoInputs() const { return doInputs; }
    bool GetDoMouseInputs() const { return doMouseInputs; }
    bool GetStopPlaying() const { return stopPlaying; }
    bool GetSceneVisible() const { return sceneVisible; }

    const std::tuple<float, float>& GetWindowPosition() const { return sceneWindowPosition; };
    const std::tuple<float, float>& GetWindowSize() const { return sceneWindowSize; };
    const std::tuple<float, float>& GetMousePosition() const { return mousePosition; };
    Octree* GetOctree() const { return sceneOctree; }
    Quadtree* GetDynamicTree() const { return dynamicTree; }
    UID GetMultiselectUID() const;
    GameObject* GetMultiselectParent() const { return multiSelectParent; }
    const std::map<UID, UID>& GetMultiselectedObjects() const { return selectedGameObjects; }
    const std::map<UID, MobilitySettings>& GetMultiselectedObjectsMobility() const
    {
        return selectedGameObjectsMobility;
    }

    void SetMainCamera(CameraComponent* camera) { mainCamera = camera; }
    void SetSelectedGameObject(UID newSelectedGameObject) { selectedGameObjectUID = newSelectedGameObject; };

    void SetStopPlaying(bool stop) { stopPlaying = stop; }

    void SetStaticModified() { staticModified = true; }
    void SetDynamicModified() { dynamicModified = true; }
    void SetMultiselectPosition(const float3& newPosition);

  private:
    void CreateStaticSpatialDataStruct();
    void CreateDynamicSpatialDataStruct();
    void CheckObjectsToRender(std::vector<GameObject*>& outRenderGameObjects, CameraComponent* camera) const;

  private:
    std::string sceneName                       = DEFAULT_SCENE_NAME;
    UID sceneUID                                = INVALID_UID;
    UID gameObjectRootUID                       = INVALID_UID;
    UID selectedGameObjectUID                   = INVALID_UID;
    CameraComponent* mainCamera                 = nullptr;
    bool stopPlaying                            = false;
    bool doInputs                               = false;
    bool doMouseInputs                          = false;
    bool sceneVisible                           = false;
    bool isFocused                              = false;

    std::unordered_map<UID, GameObject*> gameObjectsContainer;

    LightsConfig* lightsConfig                   = nullptr;
    Octree* sceneOctree                          = nullptr;
    Quadtree* dynamicTree                        = nullptr;

    // IMGUI WINDOW DATA
    std::tuple<float, float> sceneWindowPosition = std::make_tuple(0.f, 0.f);
    std::tuple<float, float> sceneWindowSize     = std::make_tuple(0.f, 0.f);
    std::tuple<float, float> mousePosition       = std::make_tuple(0.f, 0.f);

    bool staticModified                          = false;
    bool dynamicModified                         = false;

    std::vector<GameObject*> gameObjectsToUpdate;

    GameObject* multiSelectParent = nullptr;
    std::map<UID, UID> selectedGameObjects;
    std::map<UID, MobilitySettings> selectedGameObjectsMobility;
};
