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
#include "./Libs/ImGuizmo/ImGuizmo.h"

Scene::Scene(UID sceneUID, const char* sceneName, UID rootGameObject)
    : sceneUID(sceneUID), sceneName(std::string(sceneName)), gameObjectRootUID(rootGameObject)
{
    selectedGameObjectUID = gameObjectRootUID;

    lightsConfig          = new LightsConfig();
}

Scene::~Scene()
{
    for (auto it = gameObjectsContainer.begin(); it != gameObjectsContainer.end(); ++it)
    {
        delete it->second;
    }
    gameObjectsContainer.clear();

    for (auto it = gameComponents.begin(); it != gameComponents.end(); ++it)
    {
        delete it->second;
    }
    gameComponents.clear();

    delete lightsConfig;
    delete sceneOctree;
    lightsConfig = nullptr;

    GLOG("%s scene closed", sceneName.c_str());
}

void Scene::LoadComponents(const std::map<UID, Component*>& loadedGameComponents)
{
    gameComponents.clear();
    gameComponents.insert(loadedGameComponents.begin(), loadedGameComponents.end());

    lightsConfig->InitSkybox();
    lightsConfig->InitLightBuffers();
}

void Scene::LoadGameObjects(const std::unordered_map<UID, GameObject*>& loadedGameObjects)
{
    gameObjectsContainer.clear();
    gameObjectsContainer.insert(loadedGameObjects.begin(), loadedGameObjects.end());

    GameObject* root = GetGameObjectByUID(gameObjectRootUID);
    if (root != nullptr)
    {
        GLOG("Init transform and AABB calculation");
        root->ComponentGlobalTransformUpdated();
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
    stop = false;

    RenderEditorControl();
    RenderScene();
    RenderGame();

    if (stop) return UPDATE_CONTINUE;

    RenderSelectedGameObjectUI();
    lightsConfig->EditorParams();

    return UPDATE_CONTINUE;
}

void Scene::RenderEditorControl()
{
    if (!ImGui::Begin("Editor Control"))
    {
        ImGui::End();
        return;
    }

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
        stop = true;
        App->GetSceneModule()->SwitchPlayMode(false);
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
    ImGui::End();
}

void Scene::RenderScene()
{
    if (!ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::End();
        return;
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
        App->GetCameraModule()->SetAspectRatio(aspectRatio);
        framebuffer->Resize((int)windowSize.x, (int)windowSize.y);
    }

    ImGui::End();
}

void Scene::RenderGame()
{
    if (!ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::End();
        return;
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
        App->GetCameraModule()->SetAspectRatio(aspectRatio);
        framebuffer->Resize((int)windowSize.x, (int)windowSize.y);
    }

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
        GameObject* newGameObject = new GameObject(selectedGameObjectUID, "new Game Object");
        UID newUUID               = newGameObject->GetUID();

        GetGameObjectByUID(selectedGameObjectUID)->AddGameObject(newUUID);

        // TODO: change when filesystem defined
        gameObjectsContainer.insert({newUUID, newGameObject});

        GetGameObjectByUID(newGameObject->GetParent())->ComponentGlobalTransformUpdated();
    }

    if (selectedGameObjectUID != gameObjectRootUID)
    {
        ImGui::SameLine();

        if (ImGui::Button("Delete GameObject"))
        {
            UID parentUID                = GetGameObjectByUID(selectedGameObjectUID)->GetParent();
            GameObject* parentGameObject = GetGameObjectByUID(parentUID);
            RemoveGameObjectHierarchy(selectedGameObjectUID);
            // parentGameObject->PassAABBUpdateToParent(); // TODO: check if it works
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

    // TODO: change when filesystem defined
    gameObjectsContainer.erase(gameObjectUID);

    delete gameObject;
}

void Scene::RemoveComponent(uint64_t componentUID)
{
    gameComponents.erase(componentUID);
}

AABBUpdatable* Scene::GetTargetForAABBUpdate(UID uuid)
{
    if (gameObjectsContainer.count(uuid))
    {
        return gameObjectsContainer[uuid];
    }

    if (gameComponents.count(uuid))
    {
        return gameComponents[uuid];
    }

    return nullptr;
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
        AABB objectBB = objectIterator.second->GetAABB();

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
        AABB objectOBB = gameObject->GetAABB();

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

Component* Scene::GetComponentByUID(uint64_t componentUID)
{
    if (gameComponents.count(componentUID))
    {
        return gameComponents[componentUID];
    }
    return nullptr;
}
