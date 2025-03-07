#include "Scene.h"

#include "Application.h"
#include "CameraModule.h"
#include "Component.h"
#include "Framebuffer.h"
#include "GameObject.h"
#include "GameTimer.h"
#include "LibraryModule.h"
#include "Octree.h"
#include "OpenGLModule.h"
#include "SceneModule.h"

#include "imgui.h"
#include "imgui_internal.h"
// guizmo after imgui include
#include "Importer.h"
#include "./Libs/ImGuizmo/ImGuizmo.h"

Scene::Scene(const char* sceneName)
    : sceneUID(GenerateUID())
{
    memcpy(this->sceneName, sceneName, strlen(sceneName));
    
    GameObject* sceneGameObject = new GameObject("SceneModule GameObject");
    selectedGameObjectUID = gameObjectRootUID = sceneGameObject->GetUID();
    
    gameObjectsContainer.insert({sceneGameObject->GetUID(), sceneGameObject});

    lightsConfig           = new LightsConfig();
}

Scene::Scene(const rapidjson::Value& initialState, UID loadedSceneUID) : sceneUID(loadedSceneUID)
{
    const char* initName = initialState["Name"].GetString();
    memcpy(sceneName, initName, strlen(initName));
    gameObjectRootUID      = initialState["RootGameObject"].GetUint64();
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
        lightsConfig           = new LightsConfig();
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
    sceneOctree = nullptr;

    GLOG("%s scene closed", sceneName);
}

void Scene::Init()
{
    GameObject* root = GetGameObjectByUID(gameObjectRootUID);
    if (root != nullptr)
    {
        root->UpdateTransformForGOBranch();
    }
    
    UpdateSpatialDataStruct();
}

const char* Scene::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    // Create structure
    targetState.AddMember("UID", sceneUID, allocator);
    targetState.AddMember("Name", rapidjson::Value(sceneName, allocator), allocator);
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

    } else GLOG("Light Config not found");

    return sceneName;
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

update_status Scene::Render(float deltaTime)
{
    lightsConfig->RenderSkybox();
    lightsConfig->RenderLights();

    std::vector<GameObject*> objectsToRender;
    CheckObjectsToRender(objectsToRender);

    for (auto& gameObject : objectsToRender)
    {
        if (gameObject != nullptr)
        {
            gameObject->Render();
        }
    }

    return UPDATE_CONTINUE;
}

update_status Scene::RenderEditor(float deltaTime)
{
    RenderScene();

    RenderSelectedGameObjectUI();

    lightsConfig->EditorParams();

    return UPDATE_CONTINUE;
}

void Scene::RenderScene()
{
    if (!ImGui::Begin(sceneName))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginChild("##SceneChildToolBar", ImVec2(0, 70)))
    {
        GameTimer* gameTimer = App->GetGameTimer();

        float timeScale      = gameTimer->GetTimeScale();

        if (ImGui::Button("Play"))
        {
            App->GetSceneModule()->SwitchPlayModeStateTo(true);
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
            App->GetSceneModule()->SwitchPlayModeStateTo(false);
            gameTimer->Reset();
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat("Time scale", &timeScale, 0, 4)) gameTimer->SetTimeScale(timeScale);

        if (App->GetSceneModule()->IsInPlayMode())
        {
            ImGui::SeparatorText("Playing");
            ImGui::Text("Frame count: %d", gameTimer->GetFrameCount());
            ImGui::SameLine();
            ImGui::Text("Game time: %.3f", gameTimer->GetTime() / 1000.0f);
            ImGui::SameLine();
            ImGui::Text("Delta time: %.3f", gameTimer->GetDeltaTime() / 1000.0f);
        }

        ImGui::EndChild();
    }

    if (ImGui::BeginChild(
            "##SceneChild", ImVec2(0.f, 0.f), NULL, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar
        ))
    {
        // right click focus window
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ImGui::SetWindowFocus();

        // do inputs only if window is focused
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
            ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
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

        ImGuizmo::SetRect(
            ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight()
        );

        ImVec2 windowSize = ImGui::GetWindowSize();
        if (framebuffer->GetTextureWidth() != windowSize.x || framebuffer->GetTextureHeight() != windowSize.y)
        {
            float aspectRatio = windowSize.y / windowSize.x;
            App->GetCameraModule()->SetAspectRatio(aspectRatio);
            framebuffer->Resize((int)windowSize.x, (int)windowSize.y);
        }

        ImGui::EndChild();
    }
    ImGui::End();
}

void Scene::RenderSelectedGameObjectUI()
{
    GameObject* selectedGameObject = GetSeletedGameObject();
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

void Scene::RemoveGameObjectHierarchy(UID gameObjectUUID)
{
    // TODO: Change when filesystem defined
    if (!gameObjectsContainer.count(gameObjectUUID) || gameObjectUUID == gameObjectRootUID) return;

    GameObject* gameObject = GetGameObjectByUID(gameObjectUUID);

    for (UID childUUID : gameObject->GetChildren())
    {
        RemoveGameObjectHierarchy(childUUID);
    }

    UID parentUUID = gameObject->GetParent();

    // TODO: change when filesystem defined
    if (gameObjectsContainer.count(parentUUID))
    {
        GameObject* parentGameObject = GetGameObjectByUID(parentUUID);
        parentGameObject->RemoveGameObject(gameObjectUUID);
        selectedGameObjectUID = parentUUID;
    }

    // TODO: change when filesystem defined
    gameObjectsContainer.erase(gameObjectUUID);

    delete gameObject;
}

const std::unordered_map<uint64_t, Component*> Scene::GetAllComponents() const
{
    std::unordered_map<uint64_t, Component*> collectedComponents;
    for (auto& pair : gameObjectsContainer)
    {
        if (pair.second != nullptr)
        {
            collectedComponents.insert(pair.second->GetComponents().begin(), pair.second->GetComponents().end());
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

    sceneOctree->QueryElements(frustumPlanes, queriedObjects);

    for (auto gameObject : queriedObjects)
    {
        AABB objectOBB = gameObject->GetGlobalAABB();

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
