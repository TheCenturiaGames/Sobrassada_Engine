#pragma once

#include "Module.h"
#include "Scene.h"

#include <document.h>

class SceneModule : public Module
{
  public:
    SceneModule();
    ~SceneModule() override;

    bool Init() override;
    update_status PreUpdate(float deltaTime) override;
    update_status Update(float deltaTime) override;
    update_status Render(float deltaTime) override;
    update_status RenderEditor(float deltaTime) override;
    update_status PostUpdate(float deltaTime) override;
    bool ShutDown() override;

    void CreateScene();
    void LoadScene(const rapidjson::Value& initialState, bool forceReload = false);
    void CloseScene();

    void SwitchPlayMode(bool play);

    void ReenerateStaticTree() const { loadedScene->UpdateStaticSpatialStructure(); }
    void ReenerateDynamicTree() const { loadedScene->UpdateDynamicSpatialStructure(); }

    bool GetDoInputsScene() const { return loadedScene->GetDoInputs() && !inPlayMode; }
    bool GetDoMouseInputsScene() const { return loadedScene->GetDoMouseInputs() && !inPlayMode; }
    bool GetDoInputsGame() const { return loadedScene->GetDoInputs() && inPlayMode; }

    bool IsSceneLoaded() const { return loadedScene != nullptr; }

    Scene* GetScene() const { return loadedScene; }

    bool GetInPlayMode() const { return inPlayMode; }

  private:
    Scene* loadedScene = nullptr;
    bool inPlayMode    = false;
};
