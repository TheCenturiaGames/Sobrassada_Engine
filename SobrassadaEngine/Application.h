#pragma once

#include "Globals.h"
#include "Module.h"

#include <list>
#include <tuple>

class EngineConfig;
class WindowModule;
class OpenGLModule;
class ResourcesModule;
class InputModule;
class ShaderModule;
class LibraryModule;
class EditorUIModule;
class ProjectModule;
class SceneModule;
class CameraModule;
class DebugDrawModule;
class UserInterfaceModule;

class EngineTimer;
class GameTimer;

template <typename... Modules> class Application
{
  public:
    Application();
    ~Application();

    bool Init();
    update_status Update();
    bool ShutDown();

    WindowModule* GetWindowModule() { return windowModule; }
    OpenGLModule* GetOpenGLModule() { return openGLModule; }
    ResourcesModule* GetResourcesModule() { return resourcesModule; }

    InputModule* GetInputModule() { return inputModule; }
    ShaderModule* GetShaderModule() { return shaderModule; }
    LibraryModule* GetLibraryModule() { return libraryModule; }
    EditorUIModule* GetEditorUIModule() { return editorUIModule; };
    ProjectModule* GetProjectModule() { return projectModule; };
    SceneModule* GetSceneModule() { return sceneModule; }
    CameraModule* GetCameraModule() { return cameraModule; }
    DebugDrawModule* GetDebugDrawModule() { return debugDraw; }
    UserInterfaceModule* GetUserInterfaceModule() { return UIModule; }

    EngineTimer* GetEngineTimer() { return engineTimer; }
    GameTimer* GetGameTimer() { return gameTimer; }

    EngineConfig* GetEngineConfig() const { return engineConfig; }

    template <typename ModuleType> ModuleType* GetModule() { return std::get<ModuleType*>(engineModules); }

  private:
    std::list<Module*> modules;

    WindowModule* windowModule       = nullptr;
    OpenGLModule* openGLModule       = nullptr;
    ResourcesModule* resourcesModule = nullptr;
    InputModule* inputModule         = nullptr;
    ShaderModule* shaderModule       = nullptr;
    LibraryModule* libraryModule     = nullptr;
    EditorUIModule* editorUIModule   = nullptr;
    ProjectModule* projectModule     = nullptr;
    SceneModule* sceneModule         = nullptr;
    CameraModule* cameraModule       = nullptr;
    DebugDrawModule* debugDraw       = nullptr;
    UserInterfaceModule* UIModule    = nullptr;

    EngineTimer* engineTimer         = nullptr;
    GameTimer* gameTimer             = nullptr;

    EngineConfig* engineConfig       = nullptr;

    std::tuple<Modules...> engineModules;
};

extern Application<
    WindowModule, OpenGLModule, ResourcesModule, InputModule, ShaderModule, LibraryModule, EditorUIModule,
    ProjectModule, SceneModule, CameraModule, DebugDrawModule, UserInterfaceModule, EngineTimer, GameTimer,
    EngineConfig>* App;
