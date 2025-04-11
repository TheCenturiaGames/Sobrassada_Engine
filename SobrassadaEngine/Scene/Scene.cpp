#include "Scene.h"

#include "Application.h"
#include "BatchManager.h"
#include "CameraComponent.h"
#include "CameraModule.h"
#include "Component.h"
#include "ComponentUtils.h"
#include "DebugDrawModule.h"
#include "EditorUIModule.h"
#include "Framebuffer.h"
#include "GameObject.h"
#include "GameTimer.h"
#include "GeometryBatch.h"
#include "Importer.h"
#include "InputModule.h"
#include "LibraryModule.h"
#include "ModelImporter.h"
#include "Octree.h"
#include "OpenGLModule.h"
#include "PhysicsModule.h"
#include "ProjectModule.h"
#include "Quadtree.h"
#include "Resource.h"
#include "ResourceModel.h"
#include "ResourcePrefab.h"
#include "ResourcesModule.h"
#include "SceneModule.h"
#include "Standalone/MeshComponent.h"
#include "Standalone/AnimationComponent.h"

#include "SDL_mouse.h"
#include "imgui.h"
#include "imgui_internal.h"
// guizmo after imgui include
#include "ImGuizmo.h"
#ifdef OPTICK
#include "optick.h"
#endif

Scene::Scene(const char* sceneName) : sceneUID(GenerateUID())
{
    this->sceneName             = sceneName;

    GameObject* sceneGameObject = new GameObject("SceneModule GameObject");
    selectedGameObjectUID = gameObjectRootUID = sceneGameObject->GetUID();

    gameObjectsContainer.insert({sceneGameObject->GetUID(), sceneGameObject});

    lightsConfig = new LightsConfig();
}

Scene::Scene(const rapidjson::Value& initialState, UID loadedSceneUID) : sceneUID(loadedSceneUID)
{
    this->sceneName       = initialState["Name"].GetString();
    gameObjectRootUID     = initialState["RootGameObject"].GetUint64();
    selectedGameObjectUID = gameObjectRootUID;

    App->GetPhysicsModule()->LoadLayerData(&initialState);

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

    GLOG("%s scene loaded", sceneName.c_str());

    App->GetResourcesModule()->GetBatchManager()->LoadData();
}

Scene::~Scene()
{
    for (auto it = gameObjectsContainer.begin(); it != gameObjectsContainer.end(); ++it)
    {
        delete it->second;
    }
    gameObjectsContainer.clear();

    selectedGameObjects.clear();

    delete lightsConfig;
    delete sceneOctree;
    delete dynamicTree;

    lightsConfig = nullptr;
    sceneOctree  = nullptr;
    dynamicTree  = nullptr;

    App->GetPhysicsModule()->EmptyWorld();
    GLOG("%s scene closed", sceneName.c_str());
}

void Scene::Init()
{
    for (auto& gameObject : gameObjectsContainer)
    {
        gameObject.second->Init();
    }

    GetGameObjectByUID(gameObjectRootUID)->UpdateTransformForGOBranch();

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

    // Initialize the skinning for all the gameObjects that need it
    for (const auto& gameObject : gameObjectsContainer)
    {
        MeshComponent* mesh = gameObject.second->GetMeshComponent();
        if (mesh != nullptr) mesh->InitSkin();
    }

    lightsConfig->InitSkybox();
    lightsConfig->InitLightBuffers();

    UpdateStaticSpatialStructure();
    UpdateDynamicSpatialStructure();

    multiSelectParent = new GameObject(GenerateUID(), "MULTISELECT_DUMMY");
    gameObjectsContainer.insert({multiSelectParent->GetUID(), multiSelectParent});
}

void Scene::Save(
    rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator, SaveMode saveMode, UID newUID,
    const char* newName
)
{
    if (newUID != INVALID_UID)
    {
        sceneUID = newUID;
    }
    if (newName != nullptr)
    {
        sceneName = newName;
    }

    targetState.AddMember("UID", sceneUID, allocator);
    targetState.AddMember("Name", rapidjson::Value(sceneName.c_str(), allocator), allocator);

    targetState.AddMember("RootGameObject", gameObjectRootUID, allocator);

    App->GetPhysicsModule()->SaveLayerData(targetState, allocator);

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
    LightsConfig* lightConfig = App->GetSceneModule()->GetScene()->GetLightsConfig();

    if (lightConfig != nullptr)
    {
        rapidjson::Value lights(rapidjson::kObjectType);

        lightConfig->SaveData(lights, allocator);

        targetState.AddMember("Lights Config", lights, allocator);
    }

    else GLOG("Light Config not found");

    // TODO Convert to parameter which can be set later manually instead of saving a scene as default "on scene save"
    if (saveMode != SaveMode::SavePlayMode) App->GetProjectModule()->SetAsStartupScene(sceneName);
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

    UpdateStaticSpatialStructure();
    UpdateDynamicSpatialStructure();
}

update_status Scene::Update(float deltaTime)
{
#ifdef OPTICK
    OPTICK_CATEGORY("Scene::Update", Optick::Category::GameLogic)
#endif
    for (auto& gameObject : gameObjectsContainer)
    {
        std::unordered_map<ComponentType, Component*> componentList = gameObject.second->GetComponents();
        for (auto& component : componentList)
        {
            component.second->Update(deltaTime);
        }
    }

    ImGuiWindow* window = ImGui::FindWindowByName(sceneName.c_str());
    if (window && !(window->Hidden || window->Collapsed)) sceneVisible = true;
    else sceneVisible = false;

    return UPDATE_CONTINUE;
}

update_status Scene::Render(float deltaTime)
{
    if (!App->GetDebugDrawModule()->GetDebugOptionValue((int)DebugOptions::RENDER_WIREFRAME))
        lightsConfig->RenderSkybox();
    lightsConfig->SetLightsShaderData();

    std::vector<GameObject*> objectsToRender;
    CheckObjectsToRender(objectsToRender);

    {
#ifdef OPTICK
        OPTICK_CATEGORY("Scene::MeshesToRender", Optick::Category::GameLogic)
#endif
        BatchManager* batchManager = App->GetResourcesModule()->GetBatchManager();
        std::vector<MeshComponent*> meshesToRender;

        for (const auto& gameObject : objectsToRender)
        {
            MeshComponent* mesh = gameObject->GetMeshComponent();
            if (mesh != nullptr && mesh->GetEnabled() && mesh->GetBatch() != nullptr) meshesToRender.push_back(mesh);
        }

        batchManager->Render(meshesToRender);
    }

    {
#ifdef OPTICK
        OPTICK_CATEGORY("Scene::GameObject::Render", Optick::Category::Rendering)
#endif
        for (const auto& gameObject : objectsToRender)
        {
            if (gameObject != nullptr)
            {
                gameObject->Render(deltaTime);
            }
        }
    }

    {
#ifdef OPTICK
        OPTICK_CATEGORY("Scene::GameObject::DrawGizmos", Optick::Category::Rendering)
#endif
        for (const auto& gameObject : gameObjectsContainer)
        {
            gameObject.second->DrawGizmos();
        }
    }

    DebugDrawModule* debugDraw = App->GetDebugDrawModule();

    for (auto& gameObjectIterator : selectedGameObjects)
    {
        GameObject* gameObject = GetGameObjectByUID(gameObjectIterator.first);

        const AABB aabb              = gameObject->GetHierarchyAABB();
        
        for (int i = 0; i < 12; ++i)
            debugDraw->DrawLineSegment(aabb.Edge(i), float3(1.f, 1.0f, 0.5f));
    }

    return UPDATE_CONTINUE;
}

update_status Scene::RenderEditor(float deltaTime)
{
    EditorUIModule* editor = App->GetEditorUIModule();
    if (editor->editorControlMenu) RenderEditorControl(editor->editorControlMenu);

    RenderScene();

    RenderSelectedGameObjectUI();
    if (editor->lightConfig) lightsConfig->EditorParams(editor->lightConfig);

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

    // RENDER OPTIONS
    if (ImGui::Button("Render options"))
    {
        ImGui::OpenPopup("RenderOptions");
    }

    if (ImGui::BeginPopup("RenderOptions"))
    {
        int stringCount   = sizeof(DebugStrings) / sizeof(char*);
        float listBoxSize = (float)stringCount + 0.5f;
        if (ImGui::BeginListBox(
                "##RenderOptionsList", ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeightWithSpacing() * listBoxSize)
            ))
        {
            const auto& debugBitset = App->GetDebugDrawModule()->GetDebugOptionValues();
            for (int i = 0; i < stringCount; ++i)
            {
                bool currentBitValue = debugBitset[i];
                if (ImGui::Checkbox(DebugStrings[i], &currentBitValue))
                {
                    App->GetDebugDrawModule()->FlipDebugOptionValue(i);
                    if (i == (int)DebugOptions::RENDER_WIREFRAME)
                        App->GetOpenGLModule()->SetRenderWireframe(currentBitValue);
                    else if (i == (int)DebugOptions::RENDER_PHYSICS_WORLD)
                        App->GetPhysicsModule()->SetDebugOption(currentBitValue);
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
    if (!ImGui::Begin(sceneName.c_str(), nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::End();
        return;
    }

    // right click focus window
    if (ImGui::IsWindowHovered() &&
        (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
        ImGui::SetWindowFocus();

    isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_DockHierarchy);

    // do inputs only if window is focused
    if (ImGui::IsWindowHovered(ImGuiFocusedFlags_DockHierarchy))
    {
        doMouseInputs = true;
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_DockHierarchy)) doInputs = true;
    }
    else
    {
        doInputs      = false;
        doMouseInputs = false;
    }

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
        if (App->GetSceneModule()->GetInPlayMode() && App->GetSceneModule()->GetScene()->GetMainCamera() != nullptr)
        {
            App->GetSceneModule()->GetScene()->GetMainCamera()->SetAspectRatio(aspectRatio);
        }
        else App->GetCameraModule()->SetAspectRatio(aspectRatio);
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

    std::stack<UID> toDelete;
    toDelete.push(gameObjectUID);
    GameObject* gameObject = GetGameObjectByUID(gameObjectUID);

    //
    if (gameObject->IsStatic()) SetStaticModified();
    else SetDynamicModified();

    std::vector<UID> collectedUIDs;

    // Collect all UIDs to delete
    while (!toDelete.empty())
    {
        UID currentUID = toDelete.top();
        toDelete.pop();

        GameObject* gameObject = GetGameObjectByUID(currentUID);
        if (gameObject == nullptr) continue;

        collectedUIDs.push_back(currentUID);

        for (UID childUID : gameObject->GetChildren())
        {
            toDelete.push(childUID);
        }
    }

    // Remove from parent
    UID parentUID = GetGameObjectByUID(gameObjectUID)->GetParent();
    if (gameObjectsContainer.count(parentUID))
    {
        GameObject* parentGameObject = GetGameObjectByUID(parentUID);
        parentGameObject->RemoveGameObject(gameObjectUID);
        selectedGameObjectUID = parentUID;
    }

    // Delete collected game objects
    for (UID uid : collectedUIDs)
    {
        if (GetGameObjectByUID(uid)->IsStatic()) SetStaticModified();
        else SetDynamicModified();

        delete gameObjectsContainer[uid];
        gameObjectsContainer.erase(uid);
    }
}

void Scene::AddGameObjectToUpdate(GameObject* gameObject)
{
    if (!gameObject->WillUpdate())
    {
        gameObject->SetWillUpdate(true);
        gameObjectsToUpdate.push_back(gameObject);
    }
}

void Scene::UpdateGameObjects()
{
    for (GameObject* gameObject : gameObjectsToUpdate)
    {
        if (gameObject)
        {
            gameObject->UpdateComponents();
            gameObject->SetWillUpdate(false);
        }
    }
    gameObjectsToUpdate.clear();
}

void Scene::AddGameObjectToSelection(UID gameObject, UID gameObjectParent)
{
    auto pairResult = selectedGameObjects.insert({gameObject, gameObjectParent});

    if (pairResult.second)
    {
        GameObject* selectedGameObject       = GetGameObjectByUID(gameObject);
        GameObject* selectedGameObjectParent = GetGameObjectByUID(gameObjectParent);

        //selectedGameObjectParent->RemoveGameObject(gameObject);

        multiSelectParent->AddGameObject(gameObject);

        selectedGameObject->SetParent(multiSelectParent->GetUID());
        selectedGameObject->UpdateLocalTransform(multiSelectParent->GetGlobalTransform());
        selectedGameObject->UpdateTransformForGOBranch();

        selectedGameObjectUID = multiSelectParent->GetUID();
    }
    else if (pairResult.first != selectedGameObjects.end())
    {
        multiSelectParent->RemoveGameObject(gameObject);

        GameObject* selectedGameObject       = GetGameObjectByUID(gameObject);
        GameObject* selectedGameObjectParent = GetGameObjectByUID(selectedGameObjects[gameObject]);

        selectedGameObject->SetParent(selectedGameObjectParent->GetUID());
        selectedGameObjectParent->AddGameObject(gameObject);

        if (selectedGameObjectParent->GetUID() != gameObjectRootUID)
        {
            selectedGameObject->UpdateLocalTransform(selectedGameObjectParent->GetGlobalTransform());
            selectedGameObject->UpdateTransformForGOBranch();
        }

        selectedGameObjects.erase(pairResult.first);
    }
}

void Scene::ClearObjectSelection()
{
    for (auto& pairGameObject : selectedGameObjects)
    {
        GameObject* currentGameObject        = GetGameObjectByUID(pairGameObject.first);
        GameObject* selectedGameObjectParent = GetGameObjectByUID(pairGameObject.second);

        multiSelectParent->RemoveGameObject(pairGameObject.first);
        currentGameObject->SetParent(pairGameObject.second);
        selectedGameObjectParent->AddGameObject(pairGameObject.first);

        currentGameObject->UpdateLocalTransform(selectedGameObjectParent->GetGlobalTransform());
        currentGameObject->UpdateTransformForGOBranch();
    }

    selectedGameObjects.clear();
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

UID Scene::GetMultiselectUID() const
{
    return multiSelectParent->GetUID();
}

void Scene::SetMultiselectPosition(const float3& newPosition)
{
    const float4x4 localMat = float4x4::FromTRS(newPosition, float4x4::identity, float3::one);
    multiSelectParent->SetLocalTransform(localMat);
}

void Scene::CreateStaticSpatialDataStruct()
{
    // PARAMETRIZED IN FUTURE
    float3 octreeCenter = float3::zero;
    float octreeLength  = 200;
    int nodeCapacity    = 10;
    sceneOctree         = new Octree(octreeCenter, octreeLength, nodeCapacity);

    for (const auto& objectIterator : gameObjectsContainer)
    {
        AABB objectBB = objectIterator.second->GetGlobalAABB();

        if (!objectIterator.second->IsStatic()) continue;
        if (objectIterator.second->GetUID() == gameObjectRootUID) continue;
        if (!objectBB.IsFinite() || objectBB.IsDegenerate()) continue;

        sceneOctree->InsertElement(objectIterator.second);
    }
}

void Scene::CreateDynamicSpatialDataStruct()
{
    // PARAMETRIZED IN FUTURE
    float3 center    = float3::zero;
    float length     = 200;
    int nodeCapacity = 5;
    dynamicTree      = new Quadtree(center, length, nodeCapacity);

    for (const auto& objectIterator : gameObjectsContainer)
    {
        AABB objectBB = objectIterator.second->GetGlobalAABB();

        if (objectIterator.second->IsStatic()) continue;
        if (objectIterator.second->GetUID() == gameObjectRootUID) continue;
        if (!objectBB.IsFinite() || objectBB.IsDegenerate()) continue;

        dynamicTree->InsertElement(objectIterator.second);
    }
}

void Scene::UpdateStaticSpatialStructure()
{
    staticModified = false;

    delete sceneOctree;

    CreateStaticSpatialDataStruct();
}

void Scene::UpdateDynamicSpatialStructure()
{
    dynamicModified = false;

    delete dynamicTree;

    CreateDynamicSpatialDataStruct();
}

void Scene::CheckObjectsToRender(std::vector<GameObject*>& outRenderGameObjects) const
{
#ifdef OPTICK
    OPTICK_CATEGORY("Scene::CheckObjectsToRender", Optick::Category::GameLogic)
#endif
    std::vector<GameObject*> queriedObjects;
    FrustumPlanes frustumPlanes = App->GetCameraModule()->GetFrustrumPlanes();
    if (App->GetSceneModule()->GetInPlayMode() && App->GetSceneModule()->GetScene()->GetMainCamera() != nullptr)
        frustumPlanes = App->GetSceneModule()->GetScene()->GetMainCamera()->GetFrustrumPlanes();

    sceneOctree->QueryElements<FrustumPlanes>(frustumPlanes, queriedObjects);

    dynamicTree->QueryElements<FrustumPlanes>(frustumPlanes, queriedObjects);

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
        GLOG("Load model %llu", modelUID);

        ResourceModel* newModel               = (ResourceModel*)App->GetResourcesModule()->RequestResource(modelUID);
        const Model& model                    = newModel->GetModelData();
        const std::vector<int>& rootNodesIdx  = model.GetRootNodesIdx();
        const std::vector<NodeData>& allNodes = model.GetNodes();

        std::vector<GameObject*> gameObjectsArray;
        gameObjectsArray.resize(allNodes.size());
        std::vector<UID> gameObjectsUID;
        std::vector<GameObject*> rootGameObjects;
        
        GLOG("Model Animation UID: %llu", newModel->GetAnimationUID());

        const auto& animUIDs = newModel->GetAllAnimationUIDs();
        GLOG("Total Animation UIDs %zu ", animUIDs.size());

        for (UID uid : animUIDs)
        {
            GLOG("Animation UID in list: %llu ", uid);
        }
        for (unsigned int i = 0; i < allNodes.size(); ++i)
        {
            gameObjectsUID.push_back(GenerateUID());
        }

        for (int rootNodeIdx : rootNodesIdx)
        {
            const NodeData& rootNode = allNodes[rootNodeIdx];

            std::vector<NodeParent> nodesToVisit;
            nodesToVisit.push_back({rootNodeIdx, rootNode.parentIndex});

            while (!nodesToVisit.empty())
            {
                NodeParent currentNode = nodesToVisit.back();
                nodesToVisit.pop_back();

                const int currentNodeIndex      = currentNode.nodeID;
                const int currentParentIndex    = currentNode.parentID;

                const NodeData& currentNodeData = allNodes[currentNodeIndex];

                if (currentParentIndex == -1)
                {
                    GameObject* rootObject = new GameObject(
                        GetGameObjectRootUID(), App->GetLibraryModule()->GetResourceName(modelUID),
                        gameObjectsUID[currentNodeIndex]
                    );
                    rootObject->SetLocalTransform(currentNodeData.transform);
                    // Add the gameObject to the rootObject
                    GetGameObjectByUID(GetGameObjectRootUID())->AddGameObject(rootObject->GetUID());
                    AddGameObject(rootObject->GetUID(), rootObject);
                    rootGameObjects.push_back(rootObject);
                    gameObjectsArray[currentNodeIndex] = rootObject;
                }
                else
                {
                    GameObject* gameObject = new GameObject(
                        gameObjectsUID[currentParentIndex], currentNodeData.name, gameObjectsUID[currentNodeIndex]
                    );
                    gameObject->SetLocalTransform(currentNodeData.transform);
                    GetGameObjectByUID(gameObject->GetParent())->AddGameObject(gameObject->GetUID());
                    AddGameObject(gameObject->GetUID(), gameObject);

                    gameObjectsArray[currentNodeIndex] = gameObject;
                }

                for (auto it = currentNodeData.children.rbegin(); it != currentNodeData.children.rend(); ++it)
                {
                    nodesToVisit.push_back({*it, currentNodeIndex});
                }
            }
        }

        // Iterate again to add the meshes and skins.
        // Can't be done in the same loop because the bones have to be already created
        for (int rootNodeIdx : rootNodesIdx)
        {
            const NodeData& rootNode = allNodes[rootNodeIdx];

            std::vector<NodeParent> nodesToVisit;
            nodesToVisit.push_back({rootNodeIdx, rootNode.parentIndex});

            while (!nodesToVisit.empty())
            {
                NodeParent currentNode = nodesToVisit.back();
                nodesToVisit.pop_back();

                const int currentNodeIndex      = currentNode.nodeID;
                const int currentParentIndex    = currentNode.parentID;

                const NodeData& currentNodeData = allNodes[currentNodeIndex];

                if (currentNodeData.meshes.size() > 0)
                {
                    GameObject* currentGameObject = gameObjectsArray[currentNodeIndex];
                    GLOG("Node %s has %d meshes", currentNodeData.name.c_str(), currentNodeData.meshes.size());

                    unsigned meshNum = 1;

                    for (const auto& mesh : currentNodeData.meshes)
                    {
                        GameObject* meshObject = nullptr;
                        if (currentNodeData.meshes.size() > 1)
                        {
                            meshObject = new GameObject(
                                currentGameObject->GetUID(),
                                currentGameObject->GetName() + " Mesh " + std::to_string(meshNum)
                            );
                            ++meshNum;
                        }
                        else
                        {
                            meshObject = currentGameObject;
                        }

                        if (meshObject->CreateComponent(COMPONENT_MESH))
                        {
                            if (currentNodeData.meshes.size() > 1)
                            {
                                currentGameObject->AddGameObject(meshObject->GetUID());
                                AddGameObject(meshObject->GetUID(), meshObject);
                            }

                            MeshComponent* meshComponent = meshObject->GetMeshComponent();
                            meshComponent->SetModelUID(modelUID);
                            meshComponent->AddMesh(mesh.first);
                            meshComponent->AddMaterial(mesh.second);

                            // Add skin to meshComponent
                            if (currentNodeData.skinIndex != -1)
                            {
                                GLOG(
                                    "Node %s has skin index: %d", currentNodeData.name.c_str(),
                                    currentNodeData.skinIndex
                                );
                                Skin skin = model.GetSkin(currentNodeData.skinIndex);

                                std::vector<GameObject*> bones;
                                std::vector<UID> bonesIds;
                                for (int index : skin.bonesIndices)
                                {
                                    bonesIds.push_back(gameObjectsArray[index]->GetUID());
                                    bones.push_back(gameObjectsArray[index]);
                                }
                                meshComponent->SetBones(bones, bonesIds);
                                meshComponent->SetBindMatrices(skin.inverseBindMatrices);
                                meshComponent->SetSkinIndex(currentNodeData.skinIndex);
                            }
                        }
                    }
                }

                for (auto it = currentNodeData.children.rbegin(); it != currentNodeData.children.rend(); ++it)
                {
                    nodesToVisit.push_back({*it, currentNodeIndex});
                }
            }
        }
        for (GameObject* rootGameObject : rootGameObjects)
        {
            if (!animUIDs.empty())
            {
                rootGameObject->CreateComponent(COMPONENT_ANIMATION);
                AnimationComponent* animComponent = rootGameObject->GetAnimationComponent();

                GLOG("Model has %zu animations", animUIDs.size());
                for (UID uid : animUIDs)
                {
                    GLOG("Setting aimation resource with UID %llu ", uid);
                    animComponent->SetAnimationResource(uid);

                    GLOG("Animation UID: %d", uid);
                }

            }
            else
            {
                GLOG("No animations found for this model");
            }
            rootGameObject->UpdateTransformForGOBranch();
        }
    }
}

void Scene::LoadPrefab(const UID prefabUID, const ResourcePrefab* prefab, const float4x4& transform)
{
    if (prefabUID != INVALID_UID)
    {
        std::map<UID, UID> remappingTable; // Reference UID | New GameObject UID

        const ResourcePrefab* resourcePrefab =
            prefab == nullptr ? (const ResourcePrefab*)App->GetResourcesModule()->RequestResource(prefabUID) : prefab;
        const std::vector<GameObject*>& referenceObjects = resourcePrefab->GetGameObjectsVector();
        const std::vector<int>& parentIndices            = resourcePrefab->GetParentIndices();

        std::vector<GameObject*> newObjects;
        newObjects.push_back(new GameObject(GetGameObjectRootUID(), referenceObjects[0]));

        // If new, always appear at origin. If overriden, stay in place
        if (prefab != nullptr)
        {
            newObjects[0]->SetLocalTransform(transform);
        }
        else
        {
            // This probably won't be needed when gltfDefaults are there, but for now it is
            float4x4 newTrans = newObjects[0]->GetLocalTransform();
            newTrans.SetTranslatePart(float3(0, 0, 0));
            newObjects[0]->SetLocalTransform(newTrans);
        }

        // First instantiate all gameObjects and components
        newObjects[0]->SetPrefabUID(prefabUID);
        GetGameObjectByUID(GetGameObjectRootUID())->AddGameObject(newObjects[0]->GetUID());
        AddGameObject(newObjects[0]->GetUID(), newObjects[0]);
        remappingTable.insert({referenceObjects[0]->GetUID(), newObjects[0]->GetUID()});

        for (int i = 1; i < referenceObjects.size(); ++i)
        {
            UID parentUID = newObjects[parentIndices[i]]->GetUID();
            newObjects.push_back(new GameObject(parentUID, referenceObjects[i]));
            newObjects[parentIndices[i]]->AddGameObject(newObjects[i]->GetUID());
            AddGameObject(newObjects[i]->GetUID(), newObjects[i]);
            remappingTable.insert({referenceObjects[i]->GetUID(), newObjects[i]->GetUID()});
        }

        // Then do a second loop to update all components UIDs reference (ex. skinning)
        for (int i = 0; i < newObjects.size(); ++i)
        {
            MeshComponent* mesh = referenceObjects[i]->GetMeshComponent();
            if (mesh != nullptr && mesh->GetBones().size() > 0)
            {
                // Remap the bones references
                const std::vector<UID>& bones = mesh->GetBones();
                std::vector<UID> newBonesUIDs;
                std::vector<GameObject*> newBonesObjects;

                for (const UID bone : bones)
                {
                    const UID uid = remappingTable.find(bone)->second;
                    newBonesUIDs.push_back(uid);
                    newBonesObjects.push_back(GetGameObjectByUID(uid));
                }

                // This should never be nullptr
                MeshComponent* newMesh = newObjects[i]->GetMeshComponent();
                newMesh->SetBones(newBonesObjects, newBonesUIDs);
            }
        }

        if (prefab == nullptr) App->GetResourcesModule()->ReleaseResource(resourcePrefab);
        newObjects[0]->UpdateTransformForGOBranch();

        // Get all scene lights, because if the prefab has lights when creating them they won't be added to the
        // scene, as the gameObject is still not part of the scene
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

    // Store uids and transforms. We need transforms so when we override the prefab, the objects
    // stay in place. UIDs to delete the duplicates
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