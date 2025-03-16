#include "ProjectConfig.h"

#include "Application.h"
#include "FileSystem.h"
#include "ProjectModule.h"

#include <document.h>
#include <prettywriter.h>
#include <stringbuffer.h>

ProjectConfig::ProjectConfig()
{
    ProjectConfig::Load();
}

ProjectConfig::~ProjectConfig()
{
}

void ProjectConfig::SetStartupScene(const std::string& newStartupSceneName)
{
    startupSceneName = newStartupSceneName;
    Save();
}

void ProjectConfig::Load()
{
    rapidjson::Document doc;
    bool loaded = FileSystem::LoadJSON((App->GetProjectModule()->GetLoadedProjectPath() + "ProjectConfig.json").c_str() , doc);

    if (!loaded)
    {
        GLOG("Failed to load config file. Generating an empty one. ");
        Save();
        return;
    }

    if (doc.HasMember("StartupScenePath"))
        startupSceneName = doc["StartupScenePath"].GetString();
}

void ProjectConfig::Save() const
{
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    doc.AddMember("StartupScenePath", rapidjson::Value(startupSceneName.c_str(), allocator), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter writer(buffer);
    doc.Accept(writer);

    std::string savePath = App->GetProjectModule()->GetLoadedProjectPath() + "ProjectConfig.json";
    unsigned int bytesWritten =
        FileSystem::Save(savePath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);

    if (bytesWritten == 0) GLOG("Failed to save project config file: %s", savePath.c_str());
}