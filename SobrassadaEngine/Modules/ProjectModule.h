#pragma once
#include "Module.h"

#include <string>
#include <vector>

class ProjectModule : public Module
{
  public:
    ProjectModule()           = default;
    ~ProjectModule() override = default;

    bool Init() override;
    update_status RenderEditor(float deltaTime) override;

    void CloseCurrentProject();

    const std::string& GetLoadedProjectPath() const { return loadedProjectAbsolutePath; }
    const std::string& GetLoadedProjectName() const { return loadedProjectName; }

    bool IsProjectLoaded() const { return projectLoaded; }

  private:
    void CreateNewProject(const std::string& projectPath, const std::string& projectName);
    void LoadProject(const std::string& projectPath);
    void CloseProject();

  private:
    char engineWorkingDirectory[255];

    bool projectReloadRequested      = false;
    bool projectLoaded               = false;
    bool showCreateProjectFileDialog = false;
    bool showOpenProjectFileDialog   = false;
    bool setAsStartupProject         = false;

    std::string loadedProjectName    = "No project loaded";
    std::string loadedProjectAbsolutePath;
    std::vector<char*> previouslyLoadedProjects;

    char newProjectPath[255];
    char newProjectName[255];
};
