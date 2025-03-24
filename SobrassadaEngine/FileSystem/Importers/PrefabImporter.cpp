#include "PrefabImporter.h"

#include "Application.h"
#include "FileSystem.h"
#include "GameObject.h"
#include "LibraryModule.h"
#include "MetaPrefab.h"
#include "ResourcePrefab.h"

#include <fstream>
#include <sstream>

namespace PrefabImporter
{
    UID Import(const char* sourceFilePath, const std::string& targetFilePath, UID sourceUID)
    {
        const std::string fileName     = FileSystem::GetFileNameWithExtension(sourceFilePath);
        const std::string relativePath = PREFABS_ASSETS_PATH + fileName;
        const std::string copyPath     = targetFilePath + relativePath;

        if (!FileSystem::Exists(copyPath.c_str()))
        {
            FileSystem::Copy(sourceFilePath, copyPath.c_str());
        }

        // Copy the prefab to fle and ensurees it's a json file
        rapidjson::Document doc;
        bool loaded = FileSystem::LoadJSON(copyPath.c_str(), doc);
        if (!loaded)
        {
            GLOG("Failed to load prefab JSON: %s", sourceFilePath);
            return INVALID_UID;
        }
        if (!doc.HasMember("Prefab") || !doc["Prefab"].IsObject())
        {
            GLOG("Invalid prefab format: %s", sourceFilePath);
            return INVALID_UID;
        }
        rapidjson::Value& prefab = doc["Prefab"];

        // Takes the name of the file
        std::string name;
        if (prefab.HasMember("Name") && prefab["Name"].IsString()) name = prefab["Name"].GetString();
        else name = FileSystem::GetFileNameWithoutExtension(sourceFilePath);

        // Check a valid UID if not generates a new one and assign it
        UID finalPrefabUID;
        if (sourceUID == INVALID_UID)
        {
            UID uid                   = GenerateUID();
            finalPrefabUID            = App->GetLibraryModule()->AssignFiletypeUID(uid, FileType::Prefab);

            std::string metaAssetPath = PREFABS_ASSETS_PATH + name + PREFAB_EXTENSION;
            MetaPrefab meta(finalPrefabUID, metaAssetPath);
            meta.Save(name, metaAssetPath);
        }
        else
        {
            finalPrefabUID = sourceUID;
        }

        // Route to save the prefab
        std::string saveFilePath =
            targetFilePath + PREFABS_LIB_PATH + std::to_string(finalPrefabUID) + PREFAB_EXTENSION;

        // Read the file
        std::ifstream file(copyPath);
        if (!file.is_open())
        {
            GLOG("Failed to open file: %s", copyPath.c_str());
            return INVALID_UID;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        std::string fileContent = ss.str();
        file.close();

        //  Save the file
        unsigned int bytesWritten = FileSystem::Save(
            saveFilePath.c_str(), fileContent.c_str(), static_cast<unsigned int>(fileContent.size()), false
        );
        if (bytesWritten == 0)
        {
            GLOG("Failed to save prefab file: %s", saveFilePath.c_str());
            return INVALID_UID;
        }

        // Register to library
        App->GetLibraryModule()->AddPrefab(finalPrefabUID, name);
        App->GetLibraryModule()->AddName(name, finalPrefabUID);
        App->GetLibraryModule()->AddResource(saveFilePath, finalPrefabUID);

        GLOG("%s prefab imported successfully", name.c_str());
        return finalPrefabUID;
    }
} // namespace PrefabImporter