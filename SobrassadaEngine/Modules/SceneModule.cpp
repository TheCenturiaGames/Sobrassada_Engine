#include "SceneModule.h"

#include "ComponentUtils.h"
#include "EditorUIModule.h"
#include "LibraryModule.h"
#include "Octree.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "Application.h"

#include <filesystem>
#include <tiny_gltf.h>

SceneModule::SceneModule() : sceneLibraryPath(std::filesystem::current_path().string() + DELIMITER + SCENES_PATH)
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
    
    loadedScene = new Scene("New Scene");
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
                std::string(SCENES_PATH + std::string(loadedScene->GetSceneName()) + SCENE_EXTENSION).c_str(), true
            );
        }
    }
    else
    {
        if (loadedScene != nullptr)
        {
            bInPlayMode = App->GetLibraryModule()->SaveScene(sceneLibraryPath.c_str(), SaveMode::Save);
        }
    }
}
