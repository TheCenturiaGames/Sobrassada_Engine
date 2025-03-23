#include "GameUIModule.h"

#include "Application.h"
#include "ProjectModule.h"
#include "TextManager.h"

GameUIModule::GameUIModule()
{
}

GameUIModule::~GameUIModule()
{
}

bool GameUIModule::Init()
{
    currentFont = new TextManager::FontData();
    if (App->GetProjectModule()->IsProjectLoaded())
    {
        currentFont->Init("./EngineDefaults/Shader/Font/Arial.ttf", 64);        
    }
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
    RenderText("Hello World", float2(1, 0));
    return UPDATE_CONTINUE;
}

bool GameUIModule::ShutDown()
{
    currentFont->Clean();
    return true;
}

void GameUIModule::RenderText(const std::string text, const float2& position) const
{
    TextManager::RenderText(*currentFont, text, float3(position, 0), float3(1, 1, 1));
}