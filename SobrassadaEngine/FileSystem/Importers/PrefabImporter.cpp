#include "PrefabImporter.h"

#include "Application.h"
#include "FileSystem.h"
#include "GameObject.h"
#include "LibraryModule.h"
#include "MetaPrefab.h"
#include "ProjectModule.h"
#include "ResourcePrefab.h"
#include "SceneModule.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <filesystem>
#include <fstream>
#include <queue>
#include <sstream>

#include "SceneModule.h"

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

    UID SavePrefab(const GameObject* gameObject, bool override)
    {
        // Create doc JSON
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value prefab(rapidjson::kObjectType);

        // Scene values
        UID uid = GenerateUID();

        UID finalPrefabUID =
            override ? gameObject->GetPrefabUID() : App->GetLibraryModule()->AssignFiletypeUID(uid, FileType::Prefab);

        std::string name;
        if (override)
        {
            name = App->GetLibraryModule()->GetResourceName(finalPrefabUID);
        }
        else
        {
            name                     = gameObject->GetName();
            std::string originalName = name;
            int suffix               = 1;

            const auto& prefabMap    = App->GetLibraryModule()->GetPrefabMap();
            while (prefabMap.find(name) != prefabMap.end())
            {
                name = originalName + " (" + std::to_string(suffix++) + ")";
            }
        }

        // Paths
        std::string savePath = App->GetProjectModule()->GetLoadedProjectPath() + PREFABS_LIB_PATH +
                               std::to_string(finalPrefabUID) + PREFAB_EXTENSION;

        std::string assetPath =
            App->GetProjectModule()->GetLoadedProjectPath() + PREFABS_ASSETS_PATH + name + PREFAB_EXTENSION;

        // Create structure
        prefab.AddMember("UID", finalPrefabUID, allocator);
        prefab.AddMember("Name", rapidjson::Value(name.c_str(), allocator), allocator);

        // Serialize GameObjects
        rapidjson::Value gameObjectsJSON(rapidjson::kArrayType);
        std::queue<const GameObject*> queue;
        queue.push(gameObject);

        while (!queue.empty())
        {
            const GameObject* currentGameObject = queue.front();

            rapidjson::Value goJSON(rapidjson::kObjectType);
            currentGameObject->Save(goJSON, allocator);
            gameObjectsJSON.PushBack(goJSON, allocator);

            for (UID child : currentGameObject->GetChildren())
            {
                queue.push(App->GetSceneModule()->GetScene()->GetGameObjectByUID(child));
            }

            queue.pop();
        }

        prefab.AddMember("GameObjects", gameObjectsJSON, allocator);
        doc.AddMember("Prefab", prefab, allocator);

        // Save to string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        // Save asset file
        MetaPrefab meta(finalPrefabUID, assetPath);
        meta.Save(name, assetPath);

        unsigned int bytesWritten = (unsigned int
        )FileSystem::Save(assetPath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);
        if (bytesWritten == 0)
        {
            GLOG("Failed to save prefab file: %s", assetPath.c_str());
            return INVALID_UID;
        }

        // Save library file
        bytesWritten =
            (unsigned int)FileSystem::Save(savePath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);
        if (bytesWritten == 0)
        {
            GLOG("Failed to save prefab file: %s", savePath.c_str());
            return INVALID_UID;
        }

        // Register in library
        App->GetLibraryModule()->AddPrefab(finalPrefabUID, name);
        App->GetLibraryModule()->AddName(name, finalPrefabUID);
        App->GetLibraryModule()->AddResource(savePath, finalPrefabUID);

        return finalPrefabUID;
    }

    void CopyPrefab(
        const std::string& filePath, const std::string& targetFilePath, const std::string& name, const UID sourceUID
    )
    {
        std::string destination = targetFilePath + PREFABS_LIB_PATH + std::to_string(sourceUID) + PREFAB_EXTENSION;
        FileSystem::Copy(filePath.c_str(), destination.c_str());

        App->GetLibraryModule()->AddPrefab(sourceUID, name);
        App->GetLibraryModule()->AddName(name, sourceUID);
        App->GetLibraryModule()->AddResource(destination, sourceUID);
    }

    ResourcePrefab* LoadPrefab(UID prefabUID)
    {
        rapidjson::Document doc;
        std::string filepath = App->GetLibraryModule()->GetResourcePath(prefabUID);

        bool loaded          = FileSystem::LoadJSON(filepath.c_str(), doc);
        if (!loaded)
        {
            GLOG("Failed to load prefab file: %s", filepath.c_str());
            return nullptr;
        }
        if (!doc.HasMember("Prefab") || !doc["Prefab"].IsObject())
        {
            GLOG("Invalid prefab format: %s", filepath.c_str());
            return nullptr;
        }

        rapidjson::Value& prefab = doc["Prefab"];

        UID uid                  = prefab["UID"].GetUint64();
        std::string name         = prefab["Name"].GetString();

        std::vector<GameObject*> loadedGameObjects;
        std::vector<int> parentIndices;
        if (prefab.HasMember("GameObjects") && prefab["GameObjects"].IsArray())
        {
            const rapidjson::Value& gameObjects = prefab["GameObjects"];
            for (rapidjson::SizeType i = 0; i < gameObjects.Size(); i++)
            {
                const rapidjson::Value& gameObject = gameObjects[i];
                GameObject* newObject              = new GameObject(gameObject);

                int index                          = 0;
                for (const GameObject* obj : loadedGameObjects)
                {
                    if (obj->GetUID() == newObject->GetParent()) break;
                    ++index;
                }
                parentIndices.push_back(index
                ); // We will ignore the parent index of the root object, so it's fine this is also 0 for it
                loadedGameObjects.push_back(newObject);
            }
        }
        ResourcePrefab* resourcePrefab = new ResourcePrefab(uid, name);
        resourcePrefab->LoadData(loadedGameObjects, parentIndices);
        return resourcePrefab;
    }
} // namespace PrefabImporter