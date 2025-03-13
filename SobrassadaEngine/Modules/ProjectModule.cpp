#include "ProjectModule.h"

#include "Application.h"
#include "Config/EngineConfig.h"
#include "EditorUIModule.h"
#include "FileSystem.h"
#include "ImGui.h"
#include "WindowModule.h"

#include <filesystem>

bool ProjectModule::Init()
{
    if (!App->GetEngineConfig()->GetStartupProjectPath().empty())
    {
        loadedProjectAbsolutePath = App->GetEngineConfig()->GetStartupProjectPath();
        if (FileSystem::Exists(loadedProjectAbsolutePath.c_str()))
        {
            loadedProjectName = loadedProjectAbsolutePath;
            FileSystem::RemoveDelimiterIfPresent(loadedProjectName);
            loadedProjectName = FileSystem::GetFileNameWithoutExtension(loadedProjectName);
            projectLoaded     = true;
        }
        else
        {
            loadedProjectAbsolutePath = "";
            App->GetEngineConfig()->SetStartupProjectPath("");
        }
    }
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
                        showOpenProjectFileDialog = true;
                    }
                    ImGui::InputText("Name", newProjectName, 255);
                    ImGui::Text("Project path: %s\\%s", newProjectPath, newProjectName);

                    if (ImGui::Button("Create"))
                    {
                        CreateNewProject(newProjectPath, newProjectName);
                    }
                    ImGui::SameLine();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Load project"))
                {

                    ImGui::InputText("Path", newProjectPath, 255);
                    ImGui::SameLine();
                    if (ImGui::Button("Select path"))
                    {
                        showCreateProjectFileDialog = true;
                    }

                    if (ImGui::BeginListBox("Previous projects"))
                    {
                        for (const auto& path : App->GetEngineConfig()->GetProjectPaths())
                        {
                            if (ImGui::Selectable(path.c_str(), false)) memcpy(newProjectPath, path.c_str(), 255);
                        }
                        ImGui::EndListBox();
                    }

                    if (ImGui::Button("Clear previous projects"))
                    {
                        App->GetEngineConfig()->ClearPreviouslyLoadedProjectPaths();
                    }
                    // TODO Add list view for displaying previously loaded projects

                    if (ImGui::Button("Load"))
                    {
                        LoadProject(newProjectPath);
                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::End();

            if (showCreateProjectFileDialog)
            {
                const std::string resultingPath =
                    App->GetEditorUIModule()->RenderFileDialog(showCreateProjectFileDialog, "Select project", true);
                if (!resultingPath.empty())
                {
                    memcpy(newProjectPath, resultingPath.c_str(), 255);
                    memcpy(newProjectName, FileSystem::GetFileNameWithoutExtension(newProjectPath).c_str(), 255);
                }
            }
            else if (showOpenProjectFileDialog)
            {
                const std::string resultingPath =
                    App->GetEditorUIModule()->RenderFileDialog(showOpenProjectFileDialog, "Select save location", true);
                if (!resultingPath.empty())
                {
                    memcpy(newProjectPath, resultingPath.c_str(), 255);
                }
            }
            return UPDATE_CONTINUE;
        }
    }

    return projectReloadRequested ? UPDATE_RESTART : UPDATE_CONTINUE;
}

void ProjectModule::CloseCurrentProject()
{
    CloseProject();
}

void ProjectModule::CreateNewProject(const std::string& projectPath, const std::string& projectName)
{
    projectLoaded     = true;
    loadedProjectName = std::string(projectName);

    std::string path  = projectPath;
    FileSystem::AddDelimiterIfNotPresent(path);
    loadedProjectAbsolutePath = path + projectName + DELIMITER;
    FileSystem::CreateDirectories(loadedProjectAbsolutePath.c_str());
    App->GetWindowModule()->UpdateProjectNameInWindowTitle(loadedProjectName);
    App->GetEngineConfig()->SetStartupProjectPath(loadedProjectAbsolutePath);
    projectReloadRequested = true;
}

void ProjectModule::LoadProject(const std::string& projectPath)
{
    projectLoaded     = true;
    loadedProjectName = FileSystem::GetFileNameWithoutExtension(projectPath);

    std::string path  = projectPath;
    FileSystem::AddDelimiterIfNotPresent(path);
    loadedProjectAbsolutePath = std::string(path);
    App->GetWindowModule()->UpdateProjectNameInWindowTitle(loadedProjectName);
    App->GetEngineConfig()->SetStartupProjectPath(loadedProjectAbsolutePath);
    projectReloadRequested = true;
}

void ProjectModule::CloseProject()
{
    // TODO Save all assets
    App->GetEngineConfig()->SetStartupProjectPath("");
    projectReloadRequested = true;
}