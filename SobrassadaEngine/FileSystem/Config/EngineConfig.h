#pragma once
#include "ConfigFile.h"

#include <string>
#include <unordered_set>

class EngineConfig : public ConfigFile
{
  public:
    EngineConfig();
    ~EngineConfig() override;

    void SetStartupProjectPath(const std::string& newStartupProjectPath);

    void Load() override;
    void Save() const override;

    const std::string& GetStartupProjectPath() const { return startupProjectPath; }
    const std::unordered_set<std::string>& GetProjectPaths() const { return previouslyLoadedProjectPaths; }

    void ClearPreviouslyLoadedProjectPaths();

  private:
    std::string startupProjectPath;
    std::unordered_set<std::string> previouslyLoadedProjectPaths;
};
