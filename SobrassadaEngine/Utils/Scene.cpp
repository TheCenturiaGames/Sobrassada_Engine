#include "Scene.h"

#include "Application.h"
#include "CameraModule.h"
#include "Component.h"
#include "EditorUIModule.h"
#include "Framebuffer.h"
#include "GameObject.h"
#include "GameTimer.h"
#include "InputModule.h"
#include "LibraryModule.h"
#include "Octree.h"
#include "OpenGLModule.h"
#include "ResourceManagement/Resources/Resource.h"
#include "ResourceManagement/Resources/ResourceModel.h"
#include "ResourcesModule.h"
#include "Scene/Components/ComponentUtils.h"
#include "Scene/Components/Standalone/MeshComponent.h"
#include "SceneModule.h"

#include "imgui.h"
#include "imgui_internal.h"
// guizmo after imgui include
#include "./Libs/ImGuizmo/ImGuizmo.h"
#include "Importer.h"
#include "SDL_mouse.h"

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
    memcpy(sceneName, initName, strlen(initName));
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

    lightsConfig->InitSkybox();
    lightsConfig->InitLightBuffers();

    UpdateSpatialDataStruct();
}

void Scene::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
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

    if (App->GetSceneModule()->IsInPlayMode())
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
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ImGui::SetWindowFocus();

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

    // TODO: change when filesystem defined
    gameObjectsContainer.erase(gameObjectUID);

    delete gameObject;
}

const std::unordered_map<UID, Component*> Scene::GetAllComponents() const
{
    std::unordered_map<UID, Component*> collectedComponents;
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

    sceneOctree->QueryElements<FrustumPlanes>(frustumPlanes, queriedObjects);

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

void Scene::LoadModel(const UID modelUID)
{
    if (modelUID != CONSTANT_EMPTY_UID)
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
                GameObject* gameObject = new GameObject(gameObjectsArray[nodes[i].parentIndex]->GetUID(), nodes[i].name);
                //gameObject->SetLocalTransform(nodes[0].transform);

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
