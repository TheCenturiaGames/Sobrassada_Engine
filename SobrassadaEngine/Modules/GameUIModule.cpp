#include "GameUIModule.h"

#include "Application.h"
#include "TextManager.h"
#include "CameraModule.h"
#include "InputModule.h"
#include "Scene/Components/Standalone/UI/CanvasComponent.h"

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

    for (CanvasComponent* canvas : canvases)
    {
        canvas->UpdateChildren();
        canvas->UpdateMousePosition(inputs->GetMousePosition());
    }

    if (inputs->GetMouseButtonDown(1))
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