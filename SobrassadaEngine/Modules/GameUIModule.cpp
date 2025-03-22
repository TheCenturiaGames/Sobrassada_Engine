#include "GameUIModule.h"

#include "Application.h"
#include "ProjectModule.h"

GameUIModule::GameUIModule()
{
}

GameUIModule::~GameUIModule()
{
}

bool GameUIModule::Init()
{
    if (App->GetProjectModule()->IsProjectLoaded())
    {
        FT_Library library;
        FT_Face face;

        if (FT_Init_FreeType(&library))
        {
            GLOG("Error: Could not initialize FreeType");
        }

        if (FT_New_Face(library, "./EngineDefaults/Shader/Font/arial.ttf", 0, &face))
        {
            GLOG("Error: Could not load the font");
        }

        GLOG("Font loaded successfully: %s %s", face->family_name, face->style_name);

        FT_Done_Face(face);
        FT_Done_FreeType(library);
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
    return UPDATE_CONTINUE;
}

bool GameUIModule::ShutDown()
{
    return true;
}