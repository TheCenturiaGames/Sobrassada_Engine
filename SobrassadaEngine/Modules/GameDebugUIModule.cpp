#include "GameDebugUIModule.h"

#include "Application.h"
#include "DebugDrawModule.h"
#include "InputModule.h"
#include "OpenGLModule.h"
#include "PhysicsModule.h"

#include "SDL.h"
#include "glew.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "imgui_internal.h"

GameDebugUIModule::GameDebugUIModule()
{
}

GameDebugUIModule::~GameDebugUIModule()
{
}

bool GameDebugUIModule::Init()
{
    return true;
}

update_status GameDebugUIModule::PreUpdate(float deltaTime)
{
#ifdef GAME

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

#endif

    return UPDATE_CONTINUE;
}

update_status GameDebugUIModule::Update(float deltaTime)
{
    return UPDATE_CONTINUE;
}

update_status GameDebugUIModule::RenderEditor(float deltaTime)
{
#ifdef GAME
    Draw();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#endif
    return UPDATE_CONTINUE;
}

update_status GameDebugUIModule::PostUpdate(float deltaTime)
{
    return UPDATE_CONTINUE;
}

bool GameDebugUIModule::ShutDown()
{
    return true;
}

void GameDebugUIModule::Draw()
{
    RenderOptions();
}

void GameDebugUIModule::RenderOptions()
{
    if (App->GetInputModule()->GetKeyboard()[SDL_SCANCODE_F9])
    {
        ImGui::OpenPopup("RenderOptions");
    }

    if (ImGui::BeginPopup("RenderOptions"))
    {
        int stringCount   = sizeof(DebugStrings) / sizeof(char*);
        float listBoxSize = (float)stringCount + 0.5f;
        if (ImGui::BeginListBox(
                "##RenderOptionsList", ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeightWithSpacing() * listBoxSize)
            ))
        {
            const auto& debugBitset = App->GetDebugDrawModule()->GetDebugOptionValues();
            for (int i = 0; i < stringCount; ++i)
            {
                bool currentBitValue = debugBitset[i];
                if (ImGui::Checkbox(DebugStrings[i], &currentBitValue))
                {
                    App->GetDebugDrawModule()->FlipDebugOptionValue(i);
                    if (i == (int)DebugOptions::RENDER_WIREFRAME)
                        App->GetOpenGLModule()->SetRenderWireframe(currentBitValue);
                    else if (i == (int)DebugOptions::RENDER_PHYSICS_WORLD)
                        App->GetPhysicsModule()->SetDebugOption(currentBitValue);
                }
            }

            ImGui::EndListBox();
        }

        ImGui::EndPopup();
    }
}