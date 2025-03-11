#include "SceneModule.h"

#include "ComponentUtils.h"
#include "EditorUIModule.h"
#include "LibraryModule.h"
#include "Octree.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "Application.h"
#include "InputModule.h"
#include "ProjectModule.h"
#include "RaycastController.h"

#include <filesystem>
#include <tiny_gltf.h>

SceneModule::SceneModule()
{
}

SceneModule::~SceneModule()
{
    
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
        const KeyState* keyboard     = App->GetInputModule()->GetKeyboard();
        if (mouseButtons[SDL_BUTTON_LEFT - 1] == KeyState::KEY_DOWN && !keyboard[SDL_SCANCODE_LALT])
        {
            GameObject* selectedObject = RaycastController::GetRayIntersection<Octree>(
                App->GetCameraModule()->CastCameraRay(), loadedScene->GetOctree()
            );

            if (selectedObject != nullptr)
            {
                loadedScene->SetSelectedGameObject(selectedObject->GetUID());
            }
        }
    }
    return UPDATE_CONTINUE;
}

bool SceneModule::ShutDown()
{
    CloseScene();
    GLOG("Destroying scene")
    return true;
}

void SceneModule::CreateScene()
{
    CloseScene();

    loadedScene = new Scene(DEFAULT_SCENE_NAME);
    loadedScene->Init();
}

void SceneModule::LoadScene(const rapidjson::Value& initialState, const bool forceReload)
{
    const UID extractedSceneUID = initialState["UID"].GetUint64();
    if (!forceReload && loadedScene != nullptr && loadedScene->GetSceneUID() == extractedSceneUID)
    {
        GLOG("Scene already loaded: %s", loadedScene->GetSceneName());
        return;
    }

    CloseScene();

    loadedScene = new Scene(initialState, extractedSceneUID);
    loadedScene->Init();
}

void SceneModule::CloseScene()
{
    // TODO Warning dialog before closing scene without saving
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
            bInPlayMode = !App->GetLibraryModule()->LoadScene(
                std::string(App->GetProjectModule()->GetLoadedProjectPath() + SCENES_PATH + std::string(loadedScene->GetSceneName()) + SCENE_EXTENSION).c_str(), true
            );
        }
    }
    else
    {
        if (loadedScene != nullptr)
        {
            bInPlayMode = App->GetLibraryModule()->SaveScene((App->GetProjectModule()->GetLoadedProjectPath() + SCENES_PATH).c_str(), SaveMode::Save);
        }
    }
}
