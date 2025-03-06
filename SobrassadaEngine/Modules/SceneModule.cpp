#include "SceneModule.h"

#include "CameraModule.h"
#include "ComponentUtils.h"
#include "EditorUIModule.h"
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
    CloseScene();
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

    GameObject* sceneGameObject = new GameObject("SceneModule GameObject");

    loadedScene                 = new Scene(GenerateUID(), "New Scene", sceneGameObject->GetUID());

    std::unordered_map<UID, GameObject*> loadedGameObjects;
    loadedGameObjects.insert({sceneGameObject->GetUID(), sceneGameObject});

    loadedScene->LoadComponents(std::map<UID, Component*>());
    loadedScene->LoadGameObjects(loadedGameObjects);

    sceneGameObject->CreateRootComponent();

    // TODO Filesystem: Save this new created level immediatelly
}

void SceneModule::LoadScene(
    UID sceneUID, const char* sceneName, UID rootGameObject, const std::map<UID, Component*>& loadedGameComponents
)
{
    CloseScene();
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
}

void SceneModule::SwitchPlayModeStateTo(bool wantedStatePlayMode)
{
    if (wantedStatePlayMode == bInPlayMode) return;

    if (bInPlayMode)
    {
        if (loadedScene != nullptr)
        {
            App->GetLibraryModule()->LoadScene(
                std::string(SCENES_PATH + std::string(loadedScene->GetSceneName()) + SCENE_EXTENSION).c_str(), true
            );
            bInPlayMode = false;
        }
    }
    else
    {
        if (loadedScene != nullptr)
        {
            loadedScene->Save();
            bInPlayMode = true;
        }
    }
}
