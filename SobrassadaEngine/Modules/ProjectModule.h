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

    void LoadNewProject();

    bool IsProjectLoaded() const { return projectLoaded; }

private:

    bool projectLoaded = false;
    bool projectLoadFileSelectDialogOpen = false;
    bool projectSelectPathFileSelectDialogOpen = false;

    std::string loadedProjectName = "No project loaded";
    std::string loadedProjectAbsolutePath;
    std::vector<char*> previouslyLoadedProjects;
};
