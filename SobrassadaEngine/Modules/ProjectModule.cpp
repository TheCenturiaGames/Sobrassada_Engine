#include "ProjectModule.h"

#include "Application.h"
#include "ImGui.h"
#include "WindowModule.h"

#include <filesystem>

bool ProjectModule::Init()
{
    //loadedProjectName = "TemplateProject";
    //loadedProjectAbsolutePath = std::filesystem::current_path().string() + DELIMITER + DEFAULT_PROJECTS_PATH + DELIMITER + loadedProjectName;
    //loadedProjectName = "TemplateProject";
    //loadedProjectAbsolutePath = std::filesystem::current_path().string() + DELIMITER;
    
    //loadedProjectAbsolutePath = "C:\\Users\\lukas\\Documents\\Unreal Projects\\" + loadedProjectName + DELIMITER;
    // TODO Load config file with startup project and list of previously selected projects
    App->GetWindowModule()->UpdateProjectNameInWindowTitle(loadedProjectName);
    return Module::Init();
}

update_status ProjectModule::RenderEditor(float deltaTime)
{
    if (!projectLoaded)
    {
        if (ImGui::Begin("Project manager"))
        {
            ImGui::BeginTabBar("##ProjectLoaderBar", ImGuiTabBarFlags_None);
            {
                if (ImGui::BeginTabItem("Create new project"))
                {
                    static char newProjectPath[255];
                    static char newProjectName[255];
                    ImGui::InputText("Path", newProjectPath, 255);
                    // TODO  Button for file dialog
                    ImGui::SameLine();
                    ImGui::InputText("Name", newProjectPath, 255);
            
                    if (ImGui::Button("Select project"))
                    {
                        //todo add file dialog
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Load project"))
                {
                    static char selectedProjectPath[255];
                    ImGui::Text("Project to load");
                    ImGui::SameLine();
                    if (ImGui::Button("Select project"))
                    {
                        //todo add file dialog
                    }
                    ImGui::Text(selectedProjectPath);
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
    
            if (ImGui::Button("Load"))
            {
                // TODO Set project path and name
                projectLoaded = true;
                App->GetWindowModule()->UpdateProjectNameInWindowTitle(loadedProjectName);
            }
        
            ImGui::End();
            return UPDATE_CONTINUE;
        }
    }
    
    return UPDATE_CONTINUE;
}

void ProjectModule::LoadNewProject()
{
    // TODO Save old project
    projectLoaded = false;
    loadedProjectName = "No project loaded";
    loadedProjectAbsolutePath = "";
    App->GetWindowModule()->UpdateProjectNameInWindowTitle(loadedProjectName);
}