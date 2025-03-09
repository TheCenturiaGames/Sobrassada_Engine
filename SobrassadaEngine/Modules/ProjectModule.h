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

private:

    std::string loadedProjectName;
    std::string loadedProjectAbsolutePath;
    std::vector<char*> previouslyLoadedProjects;
};
