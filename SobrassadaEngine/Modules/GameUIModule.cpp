#include "GameUIModule.h"

#include "Application.h"
#include "TextManager.h"
#include "CameraModule.h"

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
    return UPDATE_CONTINUE;
}

update_status GameUIModule::Render(float deltaTime)
{
    for (CanvasComponent* canvas : canvas)
    {
        canvas->Render(deltaTime);
    }

    return UPDATE_CONTINUE;
}

bool GameUIModule::ShutDown()
{
    return true;
}