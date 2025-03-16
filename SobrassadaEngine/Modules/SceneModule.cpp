#include "SceneModule.h"

#include "ComponentUtils.h"
#include "EditorUIModule.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "Octree.h"
#include "ProjectModule.h"

#include "Application.h"
#include "InputModule.h"
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
    if (App->GetProjectModule()->IsProjectLoaded() && !App->GetProjectModule()->GetStartupSceneName().empty())
    {
        App->GetLibraryModule()->LoadScene((App->GetProjectModule()->GetStartupSceneName() + SCENE_EXTENSION).c_str());
    }
    return true;
}

update_status SceneModule::PreUpdate(float deltaTime)
{
    return UPDATE_CONTINUE;
}

update_status SceneModule::Update(float deltaTime)
{
    if (loadedScene != nullptr)
    {
        return loadedScene->Update(deltaTime);
    }
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
    if (loadedScene != nullptr && GetDoInputsScene() && !ImGuizmo::IsUsingAny())
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
    if (loadedScene != nullptr && loadedScene->GetStopPlaying()) SwitchPlayMode(false);

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
    if (inPlayMode)
    {
        std::string tmpScene = App->GetProjectModule()->GetLoadedProjectPath() + SCENES_PLAY_PATH +
                               std::to_string(loadedScene->GetSceneUID()) + SCENE_EXTENSION;
        FileSystem::Delete(tmpScene.c_str());
        inPlayMode = false;
    }

    // TODO Warning dialog before closing scene without saving
    delete loadedScene;
    loadedScene = nullptr;
}

void SceneModule::SwitchPlayMode(bool play)
{
    if (play == inPlayMode || loadedScene == nullptr) return;

    if (inPlayMode)
    {
        std::string tmpScene = std::to_string(loadedScene->GetSceneUID()) + SCENE_EXTENSION;
        if (App->GetLibraryModule()->LoadScene(tmpScene.c_str(), true))
        {
            FileSystem::Delete((App->GetProjectModule()->GetLoadedProjectPath() + SCENES_PLAY_PATH + tmpScene).c_str());
            inPlayMode = false;
            loadedScene->SetStopPlaying(false);
        }
    }
    else
    {
        if (App->GetLibraryModule()->SaveScene("", SaveMode::SavePlayMode)) inPlayMode = true;
    }
}
