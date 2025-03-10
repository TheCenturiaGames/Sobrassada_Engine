#include "ProjectModule.h"

#include <filesystem>

bool ProjectModule::Init()
{
    //loadedProjectName = "TemplateProject";
    //loadedProjectAbsolutePath = std::filesystem::current_path().string() + DELIMITER + DEFAULT_PROJECTS_PATH + DELIMITER + loadedProjectName;
    loadedProjectName = "TemplateProject";
    loadedProjectAbsolutePath = std::filesystem::current_path().string() + DELIMITER;
    
    loadedProjectAbsolutePath = "C:\\Users\\lukas\\Documents\\Unreal Projects\\" + loadedProjectName + DELIMITER;
    // TODO Load config file with startup project and list of previously selected projects
    return Module::Init();
}

update_status ProjectModule::RenderEditor(float deltaTime)
{
    return Module::RenderEditor(deltaTime);
}