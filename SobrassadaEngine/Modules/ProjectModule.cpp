#include "ProjectModule.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "FileSystem.h"
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
        static char newProjectPath[255];
        static char newProjectName[255];
        if (ImGui::Begin("Project manager"))
        {
            ImGui::BeginTabBar("##ProjectLoaderBar", ImGuiTabBarFlags_None);
            {
                if (ImGui::BeginTabItem("Create new project"))
                {
                    
                    ImGui::InputText("Path", newProjectPath, 255);
                    ImGui::SameLine();
                    if (ImGui::Button("Select path"))
                    {
                        projectSelectPathFileSelectDialogOpen = true;
                    }
                    ImGui::InputText("Name", newProjectName, 255);
                    ImGui::Text("Project path: %s\\%s", newProjectPath, newProjectName);
                    
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Load project"))
                {
                    static char selectedProjectPath[255];
                    ImGui::Text("Project to load");
                    ImGui::SameLine();
                    if (ImGui::Button("Select project"))
                    {
                        projectLoadFileSelectDialogOpen = true;
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

            if (projectLoadFileSelectDialogOpen)
            {
                const std::string resultingPath = App->GetEditorUIModule()->RenderFileDialog(projectLoadFileSelectDialogOpen, "Select project", true);
                if (!resultingPath.empty())
                {
                    memcpy(newProjectPath, resultingPath.c_str(), 255);
                    memcpy(newProjectName, FileSystem::GetFileNameWithoutExtension(newProjectPath).c_str(), 255);
                }
            } else if (projectSelectPathFileSelectDialogOpen)
            {
                const std::string resultingPath = App->GetEditorUIModule()->RenderFileDialog(projectSelectPathFileSelectDialogOpen, "Select save location", true);
                if (!resultingPath.empty())
                {
                    memcpy(newProjectPath, resultingPath.c_str(), 255);
                }
            }
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