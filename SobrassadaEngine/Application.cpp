#include "Application.h"

#include "CameraModule.h"
#include "DebugDrawModule.h"
#include "EditorUIModule.h"
#include "EngineTimer.h"
#include "Framebuffer.h"
#include "GameTimer.h"
#include "InputModule.h"
#include "LibraryModule.h"
#include "OpenGLModule.h"
#include "ResourcesModule.h"
#include "SceneModule.h"
#include "ShaderModule.h"
#include "WindowModule.h"

#ifdef _DEBUG
#include "optick.h"
#endif

Application::Application()
{
    modules.push_back(windowModule = new WindowModule());
    modules.push_back(openGLModule = new OpenGLModule());
    modules.push_back(libraryModule = new LibraryModule());
    modules.push_back(resourcesModule = new ResourcesModule());
    modules.push_back(inputModule = new InputModule());
    modules.push_back(shaderModule = new ShaderModule());
    modules.push_back(sceneModule = new SceneModule());
    modules.push_back(cameraModule = new CameraModule());
    modules.push_back(debugDraw = new DebugDrawModule());
    modules.push_back(editorUIModule = new EditorUIModule());

    engineTimer = new EngineTimer();
    engineTimer->Start();
    gameTimer = new GameTimer();
}

Application::~Application()
{
    for (std::list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
    {
        delete *it;
    }
}

bool Application::Init()
{
    bool returnStatus = true;

    for (std::list<Module*>::iterator it = modules.begin(); it != modules.end() && returnStatus; ++it)
        returnStatus = (*it)->Init();

    return returnStatus;
}

update_status Application::Update()
{
    const float deltaTime = engineTimer->Tick() / 1000.0f;
    gameTimer->Tick();

    update_status returnStatus = UPDATE_CONTINUE;

#ifdef _DEBUG
    OPTICK_CATEGORY("Application::PreUpdate", Optick::Category::GameLogic)
#endif
    for (std::list<Module*>::iterator it = modules.begin(); it != modules.end() && returnStatus == UPDATE_CONTINUE;
         ++it)
        returnStatus = (*it)->PreUpdate(deltaTime);

#ifdef _DEBUG
    OPTICK_CATEGORY("Application::Update", Optick::Category::GameLogic)
#endif
    for (std::list<Module*>::iterator it = modules.begin(); it != modules.end() && returnStatus == UPDATE_CONTINUE;
         ++it)
        returnStatus = (*it)->Update(deltaTime);

#ifdef _DEBUG
    OPTICK_CATEGORY("Application::Render", Optick::Category::Rendering)
#endif
    for (std::list<Module*>::iterator it = modules.begin(); it != modules.end() && returnStatus == UPDATE_CONTINUE;
         ++it)
        returnStatus = (*it)->Render(deltaTime);

#ifdef _DEBUG
    OPTICK_CATEGORY("Application::RenderEditor", Optick::Category::Rendering)
#endif
    // Unbinding frame buffer so ui gets rendered
    App->GetOpenGLModule()->GetFramebuffer()->Unbind();
    int i = 0;

    for (std::list<Module*>::iterator it = modules.begin(); it != modules.end() && returnStatus == UPDATE_CONTINUE;
        ++it)
        returnStatus = (*it)->RenderEditor(deltaTime);


#ifdef _DEBUG
    OPTICK_CATEGORY("Application::PostUpdate", Optick::Category::GameLogic)
#endif
    for (std::list<Module*>::iterator it = modules.begin(); it != modules.end() && returnStatus == UPDATE_CONTINUE;
         ++it)
    {
        returnStatus = (*it)->PostUpdate(deltaTime);
    }

    return returnStatus;
}

bool Application::ShutDown()
{
    bool returnStatus = true;

    for (std::list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend() && returnStatus; ++it)
        returnStatus = (*it)->ShutDown();

    delete engineTimer;
    delete gameTimer;

    return returnStatus;
}
