#pragma once

#include "Globals.h"
#include "Module.h"

#include <list>

class WindowModule;
class OpenGLModule;
class ResourcesModule;
class InputModule;
class ShaderModule;
class LibraryModule;
class EditorUIModule;
class SceneModule;
class CameraModule;
class DebugDrawModule;

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
    SceneModule* GetSceneModule() { return sceneModule; }
    CameraModule* GetCameraModule() { return cameraModule; }
    DebugDrawModule* GetDebugDrawModule() { return debugDraw; }

    GameTimer* GetGameTimer() { return gameTimer; }

  private:
    std::list<Module*> modules;

    WindowModule* windowModule       = nullptr;
    OpenGLModule* openGLModule       = nullptr;
    ResourcesModule* resourcesModule = nullptr;
    InputModule* inputModule         = nullptr;
    ShaderModule* shaderModule       = nullptr;
    LibraryModule* libraryModule     = nullptr;
    EditorUIModule* editorUIModule   = nullptr;
    SceneModule* sceneModule         = nullptr;
    CameraModule* cameraModule       = nullptr;
    DebugDrawModule* debugDraw       = nullptr;

    EngineTimer* engineTimer         = nullptr;
    GameTimer* gameTimer             = nullptr;
};

extern Application* App;
