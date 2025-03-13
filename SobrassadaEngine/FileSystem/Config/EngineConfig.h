#pragma once
#include "ConfigFile.h"

#include <string>
#include <vector>

class EngineConfig : public ConfigFile
{
public:
    EngineConfig();
    ~EngineConfig() override;

    void SetStartupProjectPath(const std::string& newStartupProjectPath);
    
    void Load() override;
    void Save() const override;

    const std::string& GetStartupProjectPath() const { return startupProjectPath; }
    const std::vector<std::string>& GetProjectPaths() const { return previouslyLoadedProjectPaths; }

    void ClearPreviouslyLoadedProjectPaths();

private:

    std::string startupProjectPath;
    std::vector<std::string> previouslyLoadedProjectPaths;
};
