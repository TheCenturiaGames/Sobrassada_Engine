#pragma once

#include "Globals.h"
#include "Module.h"

#include <list>

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

class Application
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
    PathfinderModule* GetPathfinderModule() { return pathModule; }

    EngineTimer* GetEngineTimer() { return engineTimer; }
    GameTimer* GetGameTimer() { return gameTimer; }

    EngineConfig* GetEngineConfig() const { return engineConfig; }

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
    PathfinderModule* pathModule     = nullptr;

    EngineTimer* engineTimer         = nullptr;
    GameTimer* gameTimer             = nullptr;

    EngineConfig* engineConfig       = nullptr;
};

extern Application* App;
