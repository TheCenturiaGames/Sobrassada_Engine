#include "SceneModule.h"

#include "CameraModule.h"
#include "ComponentUtils.h"
#include "EditorUIModule.h"
#include "FileSystem.h"
#include "FrustumPlanes.h"
#include "GameObject.h"
#include "LibraryModule.h"
#include "Octree.h"
#include "Root/RootComponent.h"
#include "Scene/Components/Standalone/MeshComponent.h"

#include "glew.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
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

    loadedScene                 = new Scene(GenerateUID(), "New Scene", sceneGameObject->GetUID());

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
