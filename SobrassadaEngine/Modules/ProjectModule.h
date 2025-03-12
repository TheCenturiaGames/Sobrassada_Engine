#pragma once
#include "Module.h"

#include <string>

class ProjectModule : public Module
{
public:

    ProjectModule() = default;
    ~ProjectModule() override = default;

    bool Init() override;
    update_status RenderEditor(float deltaTime) override;

    const std::string& GetLoadedProjectPath() const { return loadedProjectAbsolutePath; }
    const std::string& GetLoadedProjectName() const { return loadedProjectName; }

    void ShowProjectDialog();

    bool IsProjectLoaded() const { return projectLoaded; }
    
private:

    void CreateNewProject(const std::string& projectPath, const std::string& projectName);
    void LoadProject(const std::string& projectPath);
    void CloseProject();
    
private:

    bool projectLoaded = false;
    bool showCreateProjectFileDialog = false;
    bool showOpenProjectFileDialog = false;

    std::string loadedProjectName = "No project loaded";
    std::string loadedProjectAbsolutePath;
    std::vector<char*> previouslyLoadedProjects;
};
