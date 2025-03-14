#include "Scene.h"

#include "Application.h"
#include "CameraModule.h"
#include "Component.h"
#include "DebugUtils.h"
#include "EditorUIModule.h"
#include "Framebuffer.h"
#include "GameObject.h"
#include "GameTimer.h"
#include "Importer.h"
#include "InputModule.h"
#include "LibraryModule.h"
#include "Octree.h"
#include "OpenGLModule.h"
#include "ResourceManagement/Resources/Resource.h"
#include "ResourceManagement/Resources/ResourceModel.h"
#include "ResourceManagement/Resources/ResourcePrefab.h"
#include "ResourcesModule.h"
#include "Scene/Components/ComponentUtils.h"
#include "Scene/Components/Standalone/MeshComponent.h"
#include "SceneModule.h"

#include "SDL_mouse.h"
#include "imgui.h"
#include "imgui_internal.h"
// guizmo after imgui include
#include "./Libs/ImGuizmo/ImGuizmo.h"

Scene::Scene(const char* sceneName) : sceneUID(GenerateUID())
{
    memcpy(this->sceneName, sceneName, strlen(sceneName));

    GameObject* sceneGameObject = new GameObject("SceneModule GameObject");
    selectedGameObjectUID = gameObjectRootUID = sceneGameObject->GetUID();

    gameObjectsContainer.insert({sceneGameObject->GetUID(), sceneGameObject});

    lightsConfig = new LightsConfig();
}

Scene::Scene(const rapidjson::Value& initialState, UID loadedSceneUID) : sceneUID(loadedSceneUID)
{
    const char* initName = initialState["Name"].GetString();
    memcpy(this->sceneName, initName, strlen(initName));
    gameObjectRootUID     = initialState["RootGameObject"].GetUint64();
    selectedGameObjectUID = gameObjectRootUID;

    // Deserialize GameObjects
    if (initialState.HasMember("GameObjects") && initialState["GameObjects"].IsArray())
    {
        const rapidjson::Value& gameObjects = initialState["GameObjects"];
        for (rapidjson::SizeType i = 0; i < gameObjects.Size(); i++)
        {
            const rapidjson::Value& gameObject = gameObjects[i];

            GameObject* newGameObject          = new GameObject(gameObject);
            gameObjectsContainer.insert({newGameObject->GetUID(), newGameObject});
        }
    }

    // Deserialize Lights Config
    if (initialState.HasMember("Lights Config") && initialState["Lights Config"].IsObject())
    {
        lightsConfig = new LightsConfig();
        lightsConfig->LoadData(initialState["Lights Config"]);
    }

    GLOG("%s scene loaded", sceneName);
}

Scene::~Scene()
{
    for (auto it = gameObjectsContainer.begin(); it != gameObjectsContainer.end(); ++it)
    {
        delete it->second;
    }
    gameObjectsContainer.clear();

    delete lightsConfig;
    delete sceneOctree;
    lightsConfig = nullptr;
    sceneOctree  = nullptr;

    GLOG("%s scene closed", sceneName);
}

void Scene::Init()
{
    GameObject* root = GetGameObjectByUID(gameObjectRootUID);
    if (root != nullptr)
    {
        root->UpdateTransformForGOBranch();
    }

    // When loading a scene, overrides all gameObjects that have a prefabUID. That is because if the prefab has been
    // modified, the scene file may have not, so the prefabs need to be updated when loading the scene again
    std::vector<UID> prefabs;
    for (const auto& gameObject : gameObjectsContainer)
    {
        if (gameObject.second->GetPrefabUID() == INVALID_UID) continue;

        // Add to prefabs UIDs if not existing, only once each
        std::vector<UID>::iterator it = std::find(prefabs.begin(), prefabs.end(), gameObject.second->GetPrefabUID());
        if (it == prefabs.end()) prefabs.emplace_back(gameObject.second->GetPrefabUID());
    }
    for (const UID prefab : prefabs)
    {
        OverridePrefabs(prefab);
    }

    lightsConfig->InitSkybox();
    lightsConfig->InitLightBuffers();

    UpdateSpatialDataStruct();
}

void Scene::Save(
    rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator, UID newUID, const std::string& newName
) const
{
    if (newUID != INVALID_UID)
    {
        targetState.AddMember("UID", newUID, allocator);
        targetState.AddMember("Name", rapidjson::Value(newName.c_str(), allocator), allocator);
    }
    else
    {
        targetState.AddMember("UID", sceneUID, allocator);
        targetState.AddMember("Name", rapidjson::Value(sceneName, allocator), allocator);
    }

    targetState.AddMember("RootGameObject", gameObjectRootUID, allocator);

    // Serialize GameObjects
    rapidjson::Value gameObjectsJSON(rapidjson::kArrayType);

    for (auto it = gameObjectsContainer.begin(); it != gameObjectsContainer.end(); ++it)
    {
        if (it->second != nullptr)
        {
            rapidjson::Value goJSON(rapidjson::kObjectType);

            it->second->Save(goJSON, allocator);

            gameObjectsJSON.PushBack(goJSON, allocator);
        }
    }

    // Add gameObjects to scene
    targetState.AddMember("GameObjects", gameObjectsJSON, allocator);

    // Serialize Lights Config
    LightsConfig* lightConfig = App->GetSceneModule()->GetLightsConfig();

    if (lightConfig != nullptr)
    {
        rapidjson::Value lights(rapidjson::kObjectType);

        lightConfig->SaveData(lights, allocator);

        targetState.AddMember("Lights Config", lights, allocator);
    }
    else GLOG("Light Config not found");
}

void Scene::LoadComponents() const
{
    lightsConfig->InitSkybox();
    lightsConfig->InitLightBuffers();
}

void Scene::LoadGameObjects(const std::unordered_map<UID, GameObject*>& loadedGameObjects)
{
    for (auto it = gameObjectsContainer.begin(); it != gameObjectsContainer.end(); ++it)
    {
        delete it->second;
    }
    gameObjectsContainer.clear();
    gameObjectsContainer.insert(loadedGameObjects.begin(), loadedGameObjects.end());

    GameObject* root = GetGameObjectByUID(gameObjectRootUID);
    if (root != nullptr)
    {
        GLOG("Init transform and AABB calculation");
        root->UpdateTransformForGOBranch();
    }

    UpdateSpatialDataStruct();
}

update_status Scene::Render(float deltaTime) const
{
    if (!debugShaderOptions[RENDER_WIREFRAME]) lightsConfig->RenderSkybox();
    lightsConfig->SetLightsShaderData();

    std::vector<GameObject*> objectsToRender;
    CheckObjectsToRender(objectsToRender);

    for (auto& gameObject : objectsToRender)
    {
        if (gameObject != nullptr)
        {
            gameObject->Render();
        }
    }

    for (const auto& gameObject : gameObjectsContainer)
    {
        gameObject.second->DrawGizmos();
    }

    return UPDATE_CONTINUE;
}

update_status Scene::RenderEditor(float deltaTime)
{
    if (App->GetEditorUIModule()->editorControlMenu) RenderEditorControl(App->GetEditorUIModule()->editorControlMenu);

    RenderScene();

    RenderSelectedGameObjectUI();
    lightsConfig->EditorParams();

    return UPDATE_CONTINUE;
}

void Scene::RenderEditorControl(bool& editorControlMenu)
{
    if (!ImGui::Begin("Editor Control", &editorControlMenu))
    {
        ImGui::End();
        return;
    }

    GizmoOperation& currentGizmoOperation = App->GetEditorUIModule()->GetCurrentGizmoOperation();
    int selectedOp                        = static_cast<int>(currentGizmoOperation);
    ImGui::PushItemWidth(150);
    ImGui::RadioButton("T", &selectedOp, 0);
    ImGui::SameLine();
    ImGui::RadioButton("R", &selectedOp, 1);
    ImGui::SameLine();
    ImGui::RadioButton("S", &selectedOp, 2);
    ImGui::PopItemWidth();

    if (selectedOp == 0) currentGizmoOperation = GizmoOperation::TRANSLATE;
    else if (selectedOp == 1) currentGizmoOperation = GizmoOperation::ROTATE;
    else if (selectedOp == 2) currentGizmoOperation = GizmoOperation::SCALE;

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    GizmoTransform& transformType = App->GetEditorUIModule()->GetTransformType();
    int selectedMode              = static_cast<int>(transformType);
    ImGui::PushItemWidth(100);
    ImGui::RadioButton("L", &selectedMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("W", &selectedMode, 1);
    ImGui::PopItemWidth();

    if (selectedMode == 0) transformType = GizmoTransform::LOCAL;
    else if (selectedMode == 1) transformType = GizmoTransform::WORLD;

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    float3& snapValues = App->GetEditorUIModule()->GetSnapValues();
    ImGui::PushItemWidth(150);
    ImGui::Text("Snap");
    ImGui::SameLine();
    ImGui::Checkbox("##snapEnabled", &App->GetEditorUIModule()->snapEnabled);
    ImGui::SameLine();
    ImGui::InputFloat3("##snap", &snapValues.x);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    GameTimer* gameTimer = App->GetGameTimer();

    float timeScale      = gameTimer->GetTimeScale();

    if (ImGui::Button("Play"))
    {
        App->GetSceneModule()->SwitchPlayMode(true);
        gameTimer->Start();
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause"))
    {
        gameTimer->TogglePause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Step"))
    {
        gameTimer->Step();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        stopPlaying = true;
        gameTimer->Reset();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::SliderFloat("Time scale", &timeScale, 0, 4)) gameTimer->SetTimeScale(timeScale);

    ImGui::SameLine();

    // RENDER OPTIONS
    if (ImGui::Button("Render options"))
    {
        ImGui::OpenPopup("RenderOptions");
    }

    if (ImGui::BeginPopup("RenderOptions"))
    {
        float listBoxSize = debugShaderOptions.size() + debugRenderOptions.size() + 0.5f;
        if (ImGui::BeginListBox(
                "##RenderOptionsList", ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeightWithSpacing() * listBoxSize)
            ))
        {
            ImGui::Checkbox(RENDER_LIGTHS, &debugShaderOptions[RENDER_LIGTHS]);
            if (ImGui::Checkbox("Render Wireframe", &debugShaderOptions[RENDER_WIREFRAME]))
            {
                App->GetOpenGLModule()->SetRenderWireframe(debugShaderOptions[RENDER_WIREFRAME]);
            }

            ImGui::Separator();

            for (auto& debugOption : debugRenderOptions)
            {
                if (ImGui::Checkbox(debugOption.first.c_str(), &debugOption.second))
                {
                }
            }

            ImGui::EndListBox();
        }

        ImGui::EndPopup();
    }
    if (App->GetSceneModule()->GetInPlayMode())
    {
        ImGui::SeparatorText("Playing");
        ImGui::Text("Frame count: %d", gameTimer->GetFrameCount());
        ImGui::SameLine();
        ImGui::Text("Game time: %.3f", gameTimer->GetTime() / 1000.0f);
        ImGui::SameLine();
        ImGui::Text("Delta time: %.3f", gameTimer->GetDeltaTime() / 1000.0f);
        // ImGui::Text("Unscaled game time: %.3f", gameTimer->GetUnscaledTime() / 1000.0f);
        // ImGui::Text("Unscaled delta time: %.3f", gameTimer->GetUnscaledDeltaTime() / 1000.0f);
        // ImGui::Text("Reference time: %.3f", gameTimer->GetReferenceTime() / 1000.0f);
    }
    ImGui::End();
}

void Scene::RenderScene()
{
    if (!ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::End();
        return;
    }

    // right click focus window
    if (ImGui::IsWindowHovered() &&
        (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
        ImGui::SetWindowFocus();

    // do inputs only if window is focused
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_DockHierarchy) &&
        ImGui::IsWindowHovered(ImGuiFocusedFlags_DockHierarchy))
        doInputs = true;
    else doInputs = false;

    const auto& framebuffer = App->GetOpenGLModule()->GetFramebuffer();

    ImGui::SetCursorPos(ImVec2(0.f, 0.f));

    ImGui::Image(
        (ImTextureID)framebuffer->GetTextureID(),
        ImVec2((float)framebuffer->GetTextureWidth(), (float)framebuffer->GetTextureHeight()), ImVec2(0.f, 1.f),
        ImVec2(1.f, 0.f)
    );

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(); // ImGui::GetWindowDrawList()

    float width  = ImGui::GetWindowWidth();
    float height = ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, width, height);

    ImVec2 windowSize = ImGui::GetWindowSize();
    if (framebuffer->GetTextureWidth() != windowSize.x || framebuffer->GetTextureHeight() != windowSize.y)
    {
        float aspectRatio = windowSize.y / windowSize.x;
        App->GetCameraModule()->SetAspectRatio(aspectRatio);
        framebuffer->Resize((int)windowSize.x, (int)windowSize.y);
    }

    ImVec2 windowPosition     = ImGui::GetWindowPos();
    ImVec2 imGuimousePosition = ImGui::GetMousePos();
    sceneWindowPosition       = std::make_tuple(windowPosition.x, windowPosition.y);
    sceneWindowSize           = std::make_tuple(windowSize.x, windowSize.y);
    mousePosition             = std::make_tuple(imGuimousePosition.x, imGuimousePosition.y);

    ImGui::End();
}

void Scene::RenderSelectedGameObjectUI()
{
    GameObject* selectedGameObject = GetSelectedGameObject();
    if (selectedGameObject != nullptr)
    {
        selectedGameObject->RenderEditor();
    }
}

void Scene::RenderHierarchyUI(bool& hierarchyMenu)
{
    if (!ImGui::Begin("Hierarchy", &hierarchyMenu))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Add GameObject"))
    {
        GameObject* parent = GetGameObjectByUID(selectedGameObjectUID);
        if (parent != nullptr)
        {
            GameObject* newGameObject = new GameObject(selectedGameObjectUID, "new Game Object");

            gameObjectsContainer.insert({newGameObject->GetUID(), newGameObject});
            parent->AddGameObject(newGameObject->GetUID());

            newGameObject->UpdateTransformForGOBranch();
        }
    }

    if (selectedGameObjectUID != gameObjectRootUID)
    {
        ImGui::SameLine();

        if (ImGui::Button("Delete GameObject"))
        {
            RemoveGameObjectHierarchy(selectedGameObjectUID);
            App->GetSceneModule()->RegenerateTree();
        }
    }

    GameObject* rootGameObject = GetGameObjectByUID(gameObjectRootUID);
    if (rootGameObject)
    {
        rootGameObject->RenderHierarchyNode(selectedGameObjectUID);
    }

    ImGui::End();
}

void Scene::RemoveGameObjectHierarchy(UID gameObjectUID)
{
    // TODO: Change when filesystem defined
    if (!gameObjectsContainer.count(gameObjectUID) || gameObjectUID == gameObjectRootUID) return;

    GameObject* gameObject = GetGameObjectByUID(gameObjectUID);

    for (UID childUID : gameObject->GetChildren())
    {
        RemoveGameObjectHierarchy(childUID);
    }

    UID parentUID = gameObject->GetParent();

    // TODO: change when filesystem defined
    if (gameObjectsContainer.count(parentUID))
    {
        GameObject* parentGameObject = GetGameObjectByUID(parentUID);
        parentGameObject->RemoveGameObject(gameObjectUID);
        selectedGameObjectUID = parentUID;
    }

    // First delete the gameObject, then remove it from the map. This is because when removing a light, it checks if its
    // parent gameObject is in the scene, so it has to be removed after deleting the pointer. I think this change
    // doesn't affect anything else
    delete gameObject;

    // TODO: change when filesystem defined
    gameObjectsContainer.erase(gameObjectUID);
}

const std::vector<Component*> Scene::GetAllComponents() const
{
    std::vector<Component*> collectedComponents;
    for (const auto& pair : gameObjectsContainer)
    {
        if (pair.second != nullptr)
        {
            for (const auto& component : pair.second->GetComponents())
            {
                collectedComponents.push_back(component.second);
            }
        }
    }
    return collectedComponents;
}

void Scene::CreateSpatialDataStruct()
{
    // PARAMETRIZED IN FUTURE
    float3 octreeCenter = float3::zero;
    float octreeLength  = 200;
    int nodeCapacity    = 5;
    sceneOctree         = new Octree(octreeCenter, octreeLength, nodeCapacity);

    for (const auto& objectIterator : gameObjectsContainer)
    {
        AABB objectBB = objectIterator.second->GetGlobalAABB();

        if (objectIterator.second->GetUID() == gameObjectRootUID) continue;
        if (objectBB.Size().x == 0 && objectBB.Size().y == 0 && objectBB.Size().z == 0) continue;

        sceneOctree->InsertElement(objectIterator.second);
    }
}

void Scene::UpdateSpatialDataStruct()
{
    delete sceneOctree;

    CreateSpatialDataStruct();
}

void Scene::CheckObjectsToRender(std::vector<GameObject*>& outRenderGameObjects) const
{
    std::vector<GameObject*> queriedObjects;
    const FrustumPlanes& frustumPlanes = App->GetCameraModule()->GetFrustrumPlanes();

    sceneOctree->QueryElements<FrustumPlanes>(frustumPlanes, queriedObjects);

    for (auto gameObject : queriedObjects)
    {
        OBB objectOBB = gameObject->GetGlobalOBB();

        if (frustumPlanes.Intersects(objectOBB)) outRenderGameObjects.push_back(gameObject);
    }
}

GameObject* Scene::GetGameObjectByUID(UID gameObjectUUID)
{
    if (gameObjectsContainer.count(gameObjectUUID))
    {
        return gameObjectsContainer[gameObjectUUID];
    }
    return nullptr;
}

void Scene::LoadModel(const UID modelUID)
{
    if (modelUID != INVALID_UID)
    {
        GLOG("Load model %d", modelUID);

        ResourceModel* newModel            = (ResourceModel*)App->GetResourcesModule()->RequestResource(modelUID);
        const Model& model                 = newModel->GetModelData();
        const std::vector<NodeData>& nodes = model.GetNodes();

        GameObject* object                 = new GameObject(GetGameObjectRootUID(), nodes[0].name);
        object->SetLocalTransform(nodes[0].transform);

        // Add the gameObject to the rootObject
        GetGameObjectByUID(GetGameObjectRootUID())->AddGameObject(object->GetUID());
        AddGameObject(object->GetUID(), object);

        std::vector<GameObject*> gameObjectsArray;
        gameObjectsArray.push_back(object);

        for (int i = 1; i < nodes.size(); ++i)
        {
            if (nodes[i].meshes.size() >
                0) // If has meshes, create a container object and one gameObject per mesh as children
            {
                GLOG("Node %s has %d meshes", nodes[i].name.c_str(), nodes[i].meshes.size());
                GameObject* gameObject =
                    new GameObject(gameObjectsArray[nodes[i].parentIndex]->GetUID(), nodes[i].name);
                // gameObject->SetLocalTransform(nodes[0].transform);

                gameObjectsArray.emplace_back(gameObject);
                GetGameObjectByUID(gameObjectsArray[nodes[i].parentIndex]->GetUID())
                    ->AddGameObject(gameObject->GetUID());
                AddGameObject(gameObject->GetUID(), gameObject);

                unsigned meshNum = 1;
                for (const auto& mesh : nodes[i].meshes)
                {
                    GameObject* meshObject = new GameObject(gameObject->GetUID(), "Mesh " + std::to_string(meshNum));
                    ++meshNum;

                    meshObject->CreateComponent(COMPONENT_MESH);
                    meshObject->AddModel(mesh.first, mesh.second);

                    meshObject->SetLocalTransform(nodes[i].transform);

                    gameObject->AddGameObject(meshObject->GetUID());
                    AddGameObject(meshObject->GetUID(), meshObject);
                }
            }
            else
            {
                GameObject* gameObject =
                    new GameObject(gameObjectsArray[nodes[i].parentIndex]->GetUID(), nodes[i].name);

                gameObject->SetLocalTransform(nodes[i].transform);

                gameObjectsArray.emplace_back(gameObject);
                GetGameObjectByUID(gameObjectsArray[nodes[i].parentIndex]->GetUID())
                    ->AddGameObject(gameObject->GetUID());
                AddGameObject(gameObject->GetUID(), gameObject);
            }
        }
        object->UpdateTransformForGOBranch();
    }
}

void Scene::LoadPrefab(const UID prefabUID, const ResourcePrefab* prefab, const float4x4& transform)
{
    if (prefabUID != INVALID_UID)
    {
        GLOG("Load prefab %d", prefabUID);

        const ResourcePrefab* resourcePrefab =
            prefab == nullptr ? (const ResourcePrefab*)App->GetResourcesModule()->RequestResource(prefabUID) : prefab;
        const std::vector<GameObject*>& referenceObjects = resourcePrefab->GetGameObjectsVector();
        const std::vector<int>& parentIndices            = resourcePrefab->GetParentIndices();

        std::vector<GameObject*> newObjects;
        newObjects.push_back(new GameObject(GetGameObjectRootUID(), referenceObjects[0]));
        if (prefab != nullptr)
        {
            // Set the root prefab the transform it had before
            newObjects[0]->SetLocalTransform(transform);
        }

        newObjects[0]->SetPrefabUID(prefabUID);
        GetGameObjectByUID(GetGameObjectRootUID())->AddGameObject(newObjects[0]->GetUID());
        AddGameObject(newObjects[0]->GetUID(), newObjects[0]);
        // Right now it is loaded to the root gameObject

        // Add the gameObject to the scene. The parents will always be added before the children
        for (int i = 1; i < referenceObjects.size(); ++i)
        {
            UID parentUID = newObjects[parentIndices[i]]->GetUID();
            newObjects.push_back(new GameObject(parentUID, referenceObjects[i]));
            newObjects[parentIndices[i]]->AddGameObject(newObjects[i]->GetUID());
            AddGameObject(newObjects[i]->GetUID(), newObjects[i]);
        }

        if (prefab == nullptr) App->GetResourcesModule()->ReleaseResource(resourcePrefab);
        newObjects[0]->UpdateTransformForGOBranch();

        // Get all scene lights, because if the prefab has lights when creating them they won't be added to the scene,
        // as the gameObject is still not part of the scene
        if (lightsConfig != nullptr) lightsConfig->GetAllSceneLights();
    }
}

void Scene::OverridePrefabs(const UID prefabUID)
{
    const ResourcePrefab* prefab = (const ResourcePrefab*)App->GetResourcesModule()->RequestResource(prefabUID);

    // If prefab is null, it no longer exists, then remove the prefab UID from all objects that may have it
    if (prefab == nullptr)
    {
        for (const auto& gameObject : gameObjectsContainer)
        {
            if (gameObject.second != nullptr) gameObject.second->SetPrefabUID(INVALID_UID);
        }
        return;
    }

    std::vector<UID> updatedObjects;
    std::vector<float4x4> transforms;

    for (const auto& gameObject : gameObjectsContainer)
    {
        if (gameObject.second != nullptr)
        {
            if (gameObject.second->GetPrefabUID() == prefabUID)
            {
                updatedObjects.push_back(gameObject.first);
                transforms.emplace_back(gameObject.second->GetGlobalTransform());
            }
        }
    }

    for (const UID object : updatedObjects)
    {
        RemoveGameObjectHierarchy(object);
    }

    for (const float4x4& transform : transforms)
    {
        LoadPrefab(prefabUID, prefab, transform);
    }

    App->GetResourcesModule()->ReleaseResource(prefab);
}