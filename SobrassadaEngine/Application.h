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
class ScriptModule;

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

    //I added the SOBRASADA_ENGINE_API to the modules so we can use any info inside the game from the modules
    SOBRASADA_ENGINE_API WindowModule* GetWindowModule() { return windowModule; }
    SOBRASADA_ENGINE_API OpenGLModule* GetOpenGLModule() { return openGLModule; }
    SOBRASADA_ENGINE_API ResourcesModule* GetResourcesModule() { return resourcesModule; }

    SOBRASADA_ENGINE_API InputModule* GetInputModule() { return inputModule; }
    SOBRASADA_ENGINE_API ShaderModule* GetShaderModule() { return shaderModule; }
    SOBRASADA_ENGINE_API LibraryModule* GetLibraryModule() { return libraryModule; }
    SOBRASADA_ENGINE_API EditorUIModule* GetEditorUIModule() { return editorUIModule; };
    SOBRASADA_ENGINE_API ProjectModule* GetProjectModule() { return projectModule; };
    SOBRASADA_ENGINE_API SceneModule* GetSceneModule() { return sceneModule; }
    SOBRASADA_ENGINE_API CameraModule* GetCameraModule() { return cameraModule; }
    SOBRASADA_ENGINE_API DebugDrawModule* GetDebugDrawModule() { return debugDraw; }
    SOBRASADA_ENGINE_API UserInterfaceModule* GetUserInterfaceModule() { return UIModule; }
    SOBRASADA_ENGINE_API ScriptModule* GetScriptModule() { return scriptModule; }

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
    ScriptModule* scriptModule       = nullptr;

    EngineTimer* engineTimer         = nullptr;
    GameTimer* gameTimer             = nullptr;

    EngineConfig* engineConfig       = nullptr;
};

extern Application* App;