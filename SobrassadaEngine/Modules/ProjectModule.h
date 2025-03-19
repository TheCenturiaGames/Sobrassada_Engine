#pragma once
#include "Module.h"
#include "Config/ProjectConfig.h"

#include <string>
#include <vector>

class ProjectModule : public Module
{
  public:
    ProjectModule()           = default;
    ~ProjectModule() override;

    bool Init() override;
    update_status RenderEditor(float deltaTime) override;
    bool ShutDown() override;
    
    void CloseCurrentProject();
    void SetAsStartupScene(const std::string& newScenePath) const;

    const std::string& GetLoadedProjectPath() const { return loadedProjectAbsolutePath; }
    const std::string& GetLoadedProjectName() const { return loadedProjectName; }

    const std::string& GetStartupSceneName() const { return projectConfig != nullptr ? projectConfig->GetStartupScene() : DEFAULT_EMPTY_STRING; }

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

    ProjectConfig* projectConfig = nullptr;

    std::string DEFAULT_EMPTY_STRING          = "";
};
