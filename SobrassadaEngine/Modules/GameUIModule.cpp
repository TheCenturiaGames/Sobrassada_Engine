#include "GameUIModule.h"

#include "Application.h"
#include "TextManager.h"
#include "CameraModule.h"
#include "InputModule.h"
#include "Scene/Components/Standalone/UI/CanvasComponent.h"
#include "SceneModule.h"
#include "Scene.h"
#include "OpenGLModule.h"
#include "Framebuffer.h"

#include "glew.h"
#include "MathGeoLib.h"

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

#ifndef GAME
        // Must get the framebuffer size of the window and multiply by the Lerped values
        auto& windowPosition = App->GetSceneModule()->GetScene()->GetWindowPosition();
        auto& windowSize     = App->GetSceneModule()->GetScene()->GetWindowSize();
        auto& mousePos       = App->GetSceneModule()->GetScene()->GetMousePosition();

        float windowMinX     = std::get<0>(windowPosition);
        float windowMaxX     = std::get<0>(windowPosition) + std::get<0>(windowSize);

        float windowMinY     = std::get<1>(windowPosition);
        float windowMaxY     = std::get<1>(windowPosition) + std::get<1>(windowSize);

        float percentageX    = (std::get<0>(mousePos) - windowMinX) / (windowMaxX - windowMinX);
        float percentageY    = (std::get<1>(mousePos) - windowMinY) / (windowMaxY - windowMinY);

        mousePosition.x      = Lerp(-1, 1, percentageX);
        mousePosition.y      = Lerp(1, -1, percentageY);
#else
        mousePos = inputs->GetMousePosition();
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