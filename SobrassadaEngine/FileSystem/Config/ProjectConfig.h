#pragma once
#include "ConfigFile.h"

#include <string>

class ProjectConfig : public ConfigFile
{
public:
    ProjectConfig();
    ~ProjectConfig() override;

    void Load() override;
    void Save() const override;

    const std::string& GetStartupScene() const { return startupSceneName; }

    void SetStartupScene(const std::string& newStartupSceneName);

private:
    std::string startupSceneName = "";
};