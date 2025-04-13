#include "GameUIModule.h"

#include "Application.h"
#include "CameraModule.h"
#include "Framebuffer.h"
#include "InputModule.h"
#include "OpenGLModule.h"
#include "Scene.h"
#include "Scene/Components/Standalone/UI/CanvasComponent.h"
#include "SceneModule.h"
#include "TextManager.h"

#include "MathGeoLib.h"
#include "glew.h"

GameUIModule::GameUIModule()
{
}

GameUIModule::~GameUIModule()
{
}

bool GameUIModule::Init()
{
    return true;
}

update_status GameUIModule::Update(float deltaTime)
{
    InputModule* inputs = App->GetInputModule();

    if (canvases.size() > 0)
    {
        float2 mousePosition;

#if defined(GAME) || defined(GAMEDEBUG)
        // In game, just get the mouse position
        mousePosition = inputs->GetMousePosition();
#else
        // Get the mouse position depending on the window size and position
        auto& windowPosition = App->GetSceneModule()->GetScene()->GetWindowPosition();
        auto& mousePos       = App->GetSceneModule()->GetScene()->GetMousePosition();
        auto& windowSize     = App->GetSceneModule()->GetScene()->GetWindowSize();

        mousePosition.x      = std::get<0>(mousePos) - std::get<0>(windowPosition);
        mousePosition.y      = std::get<1>(windowSize) - (std::get<1>(mousePos) - std::get<1>(windowPosition));
#endif
        for (CanvasComponent* canvas : canvases)
        {
            canvas->UpdateChildren();
            canvas->UpdateMousePosition(mousePosition);
        }
    }

    if (inputs->GetMouseButtonDown(1) == KEY_DOWN)
    {
        GLOG("Mouse button pressed");
        for (CanvasComponent* canvas : canvases)
        {
            canvas->OnMouseButtonPressed();
        }
    }

    return UPDATE_CONTINUE;
}

update_status GameUIModule::Render(float deltaTime)
{
    for (CanvasComponent* canvas : canvases)
    {
        canvas->RenderUI();
    }

    return UPDATE_CONTINUE;
}

bool GameUIModule::ShutDown()
{
    return true;
}

void GameUIModule::OnWindowResize(const unsigned int width, const unsigned int height)
{
    for (CanvasComponent* canvas : canvases)
    {
        canvas->OnWindowResize(width, height);
    }
}

void GameUIModule::RemoveCanvas(CanvasComponent* canvasToRemove)
{
    const auto& iterator = std::find(canvases.begin(), canvases.end(), canvasToRemove);
    if (iterator != canvases.end())
    {
        canvases.erase(iterator);
    }
}