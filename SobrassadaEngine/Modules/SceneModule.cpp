#include "SceneModule.h"

#include "Application.h"
#include "ResourcesModule.h"
#include "CameraModule.h"
#include "ComponentUtils.h"
#include "EditorUIModule.h"
#include "FileSystem.h"
#include "FrustumPlanes.h"
#include "GameObject.h"
#include "InputModule.h"
#include "LibraryModule.h"
#include "Octree.h"
#include "RaycastController.h"
#include "Root/RootComponent.h"
#include "Scene/Components/Standalone/MeshComponent.h"

#include "glew.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "imgui_internal.h"
// guizmo after imgui include
#include "./Libs/ImGuizmo/ImGuizmo.h"
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <Algorithm/Random/LCG.h>
#include <tiny_gltf.h>

SceneModule::SceneModule()
{
}

SceneModule::~SceneModule()
{
    delete loadedScene;
    loadedScene = nullptr;
}

bool SceneModule::Init()
{
    return true;
}

update_status SceneModule::PreUpdate(float deltaTime)
{
    return UPDATE_CONTINUE;
}

update_status SceneModule::Update(float deltaTime)
{
    return UPDATE_CONTINUE;
}

update_status SceneModule::Render(float deltaTime)
{
    if (loadedScene != nullptr)
    {
        return loadedScene->Render(deltaTime);
    }

    return UPDATE_CONTINUE;
}

update_status SceneModule::RenderEditor(float deltaTime)
{
    if (loadedScene != nullptr)
    {
        return loadedScene->RenderEditor(deltaTime);
    }
    return UPDATE_CONTINUE;
}

update_status SceneModule::PostUpdate(float deltaTime)
{
    // CAST RAY WHEN LEFT CLICK IS RELEASED
    if (loadedScene != nullptr && loadedScene->GetDoInputs() && !ImGuizmo::IsUsingAny())
    {
        const KeyState* mouseButtons = App->GetInputModule()->GetMouseButtons();
        const KeyState* keyboard = App->GetInputModule()->GetKeyboard();
        if (mouseButtons[SDL_BUTTON_LEFT - 1] == KeyState::KEY_DOWN && !keyboard[SDL_SCANCODE_LALT])
        {
            GameObject* selectedObject = RaycastController::GetRayIntersection<Octree>(
                App->GetCameraModule()->CastCameraRay(), loadedScene->GetOctree()
            );

            if (selectedObject != nullptr)
            {
                loadedScene->SetSelectedGameObject(selectedObject->GetUID());
                selectedObject->GetRootComponent()->SetSelectedComponent(selectedObject->GetRootComponent()->GetUID());
            }
        }
    }
    if (loadedScene != nullptr && loadedScene->GetStopPlaying()) SwitchPlayMode(false);

    return UPDATE_CONTINUE;
}

bool SceneModule::ShutDown()
{
    GLOG("Destroying octree")
    return true;
}

void SceneModule::CreateScene()
{
    CloseScene();

    GameObject* sceneGameObject = new GameObject("Scene");

    loadedScene                 = new Scene(GenerateUID(), DEFAULT_SCENE_NAME, sceneGameObject->GetUID());

    std::unordered_map<UID, GameObject*> loadedGameObjects;
    loadedGameObjects.insert({sceneGameObject->GetUID(), sceneGameObject});

    loadedScene->LoadComponents(std::map<UID, Component*>());
    loadedScene->LoadGameObjects(loadedGameObjects);

    sceneGameObject->CreateRootComponent();
}

void SceneModule::LoadScene(
    UID sceneUID, const char* sceneName, UID rootGameObject, const std::map<UID, Component*>& loadedGameComponents
)
{
    loadedScene = new Scene(sceneUID, sceneName, rootGameObject);
    loadedScene->LoadComponents(loadedGameComponents);
}

void SceneModule::LoadGameObjects(const std::unordered_map<UID, GameObject*>& loadedGameObjects)
{
    loadedScene->LoadGameObjects(loadedGameObjects);
}

void SceneModule::CloseScene()
{
    delete loadedScene;
    loadedScene = nullptr;
    if (App->GetResourcesModule() != nullptr) App->GetResourcesModule()->UnloadAllResources();
}

void SceneModule::SwitchPlayMode(bool play)
{
    if (play == isPlayMode || loadedScene == nullptr) return;

    if (isPlayMode)
    {
        std::string tmpScene = std::to_string(loadedScene->GetSceneUID()) + SCENE_EXTENSION;
        App->GetLibraryModule()->LoadScene(tmpScene.c_str(), true);
        FileSystem::Delete((SCENES_PATH + tmpScene).c_str());
        isPlayMode = false;
        loadedScene->SetStopPlaying(false);
    }
    else
    {
        App->GetLibraryModule()->SaveScene("", SaveMode::SavePlayMode);
        isPlayMode = true;
    }
}
