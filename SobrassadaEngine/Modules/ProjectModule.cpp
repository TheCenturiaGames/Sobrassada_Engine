#include "ProjectModule.h"

#include "Application.h"
#include "Config/EngineConfig.h"
#include "EditorUIModule.h"
#include "FileSystem.h"
#include "ImGui.h"
#include "WindowModule.h"

#include <direct.h>
#include <filesystem>

ProjectModule::~ProjectModule()
{
}

bool ProjectModule::Init()
{
    _getcwd(engineWorkingDirectory, IM_ARRAYSIZE(engineWorkingDirectory));
    if (!App->GetEngineConfig()->GetStartupProjectPath().empty())
    {
        loadedProjectAbsolutePath = App->GetEngineConfig()->GetStartupProjectPath();
        if (FileSystem::Exists(loadedProjectAbsolutePath.c_str()))
        {
            loadedProjectName = loadedProjectAbsolutePath;
            FileSystem::RemoveDelimiterIfPresent(loadedProjectName);
            loadedProjectName = FileSystem::GetFileNameWithoutExtension(loadedProjectName);
            projectConfig = new ProjectConfig();
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
    if (!projectLoaded && ImGui::Begin("Project manager"))
    {
        ImGui::BeginTabBar("##ProjectLoaderBar", ImGuiTabBarFlags_None);
        {
            if (ImGui::BeginTabItem("Create new project"))
            {
                ImGui::InputText("Path", newProjectPath, IM_ARRAYSIZE(newProjectPath));
                ImGui::SameLine();
                if (ImGui::Button("Select path"))
                {
                    showOpenProjectFileDialog = true;
                    App->GetEditorUIModule()->SetFileDialogCurrentPath(newProjectPath);
                }
                ImGui::InputText("Name", newProjectName, IM_ARRAYSIZE(newProjectName));
                if (strlen(newProjectName) == 0)
                {
                    ImGui::SameLine();

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                    ImGui::Text("Name for the project folder is required!");
                    ImGui::PopStyleColor();
                }
                if (FileSystem::IsAbsolute(newProjectPath))
                    ImGui::Text("Absolute project path: %s\\%s", newProjectPath, newProjectName);
                else
                {
                    if (strlen(newProjectPath) == 0)
                        ImGui::Text("Absolute project path: %s\\%s", engineWorkingDirectory, newProjectName);
                    else
                        ImGui::Text(
                            "Absolute project path: %s\\%s\\%s", engineWorkingDirectory, newProjectPath, newProjectName
                        );
                }

                if (strlen(newProjectName) != 0 && ImGui::Button("Create"))
                {
                    if (FileSystem::IsAbsolute(newProjectPath)) CreateNewProject(newProjectPath, newProjectName);
                    else
                        CreateNewProject(
                            std::string(engineWorkingDirectory) + DELIMITER + newProjectPath, newProjectName
                        );
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Load project"))
            {

                ImGui::InputText("Path", newProjectPath, IM_ARRAYSIZE(newProjectPath));
                ImGui::SameLine();
                if (ImGui::Button("Select path"))
                {
                    showCreateProjectFileDialog = true;
                    App->GetEditorUIModule()->SetFileDialogCurrentPath(newProjectPath);
                }

                if (ImGui::BeginListBox("Previous projects"))
                {
                    for (const auto& path : App->GetEngineConfig()->GetProjectPaths())
                    {
                        if (ImGui::Selectable(path.c_str(), false)) memcpy(newProjectPath, path.c_str(), path.size());
                    }
                    ImGui::EndListBox();
                }

                if (ImGui::Button("Clear previous projects"))
                {
                    App->GetEngineConfig()->ClearPreviouslyLoadedProjectPaths();
                }
                if (FileSystem::IsAbsolute(newProjectPath)) ImGui::Text("Absolute project path: %s", newProjectPath);
                else
                {
                    if (strlen(newProjectPath) == 0) ImGui::Text("Absolute project path: %s", engineWorkingDirectory);
                    else ImGui::Text("Absolute project path: %s\\%s", engineWorkingDirectory, newProjectPath);
                }

                if (ImGui::Button("Load"))
                {
                    if (FileSystem::IsAbsolute(newProjectPath)) LoadProject(newProjectPath);
                    else LoadProject(std::string(engineWorkingDirectory) + DELIMITER + newProjectPath);
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
                memcpy(newProjectPath, resultingPath.c_str(), resultingPath.size());
                std::string newProjectPathWithoutExt = FileSystem::GetFileNameWithoutExtension(newProjectPath);
                memcpy(newProjectName, newProjectPathWithoutExt.c_str(), newProjectPathWithoutExt.size());
            }
        }
        else if (showOpenProjectFileDialog)
        {
            const std::string resultingPath =
                App->GetEditorUIModule()->RenderFileDialog(showOpenProjectFileDialog, "Select save location", true);
            if (!resultingPath.empty())
            {
                memcpy(newProjectPath, resultingPath.c_str(), resultingPath.size());
            }
        }
        return UPDATE_CONTINUE;
    }

    return projectReloadRequested ? UPDATE_RESTART : UPDATE_CONTINUE;
}

bool ProjectModule::ShutDown()
{
    delete projectConfig;

    return true;
}

void ProjectModule::CloseCurrentProject()
{
    CloseProject();
}

void ProjectModule::SetAsStartupScene(const std::string& newSceneName) const
{
    if (projectConfig != nullptr)
    {
        projectConfig->SetStartupScene(newSceneName);
    }
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