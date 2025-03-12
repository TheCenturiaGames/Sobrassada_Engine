#include "PrefabManager.h"

#include "Application.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "ResourceManagement/Resources/ResourcePrefab.h"
#include "Scene/GameObjects/GameObject.h"
#include "SceneModule.h"

#include "prettywriter.h"
#include "stringbuffer.h"
#include <filesystem>
#include <queue>

namespace PrefabManager
{
    UID SavePrefab(const GameObject* gameObject, bool override)
    {
        // Create doc JSON
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value prefab(rapidjson::kObjectType);

        // Scene values
        UID uid              = GenerateUID();
        std::string savePath = PREFABS_PATH + std::string("Prefab") + PREFAB_EXTENSION;
        UID finalPrefabUID =
            override ? gameObject->GetPrefabUID() : App->GetLibraryModule()->AssignFiletypeUID(uid, FileType::Prefab);
        savePath                = PREFABS_PATH + std::to_string(finalPrefabUID) + PREFAB_EXTENSION;
        const std::string& name = gameObject->GetName();

        // Create structure
        prefab.AddMember("UID", finalPrefabUID, allocator);
        prefab.AddMember("Name", rapidjson::Value(name.c_str(), allocator), allocator);

        // Serialize GameObjects
        rapidjson::Value gameObjectsJSON(rapidjson::kArrayType);
        std::queue<const GameObject*> queue; // Traverse all gameObjects using a queue to avoid recursiveness
        queue.push(gameObject);

        while (queue.size() > 0)
        {
            const GameObject* currentGameObject = queue.front();

            rapidjson::Value goJSON(rapidjson::kObjectType);
            currentGameObject->Save(goJSON, allocator);
            gameObjectsJSON.PushBack(goJSON, allocator);

            // TODO: Serialize the rest of components after merged with the branch where the gameObjects have an array
            // of their components, because right now is quite more complicated to get them
            // rapidjson::Value componentJSON(rapidjson::kObjectType);
            // currentGameObject->GetRootComponent()->Save(componentJSON, allocator);
            // goJSON.AddMember("RootComponent", componentJSON, allocator);

            for (UID child : currentGameObject->GetChildren())
            {
                queue.push(App->GetSceneModule()->GetGameObjectByUUID(child));
            }

            queue.pop();
        }

        // Add gameObjects to scene
        prefab.AddMember("GameObjects", gameObjectsJSON, allocator);

        doc.AddMember("Prefab", prefab, allocator);

        // Save file like JSON
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        unsigned int bytesWritten =
            (unsigned int)FileSystem::Save(savePath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);
        if (bytesWritten == 0)
        {
            GLOG("Failed to save prefab file: %s", savePath);
            return INVALID_UID;
        }

        // Add the prefab to the resources map
        App->GetLibraryModule()->AddPrefab(finalPrefabUID, name);
        App->GetLibraryModule()->AddResource(savePath, finalPrefabUID);

        return finalPrefabUID;
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
        if (prefab.HasMember("GameObjects") && prefab["GameObjects"].IsArray())
        {
            const rapidjson::Value& gameObjects = prefab["GameObjects"];
            for (rapidjson::SizeType i = 0; i < gameObjects.Size(); i++)
            {
                const rapidjson::Value& gameObject = gameObjects[i];
                GameObject* newObject              = new GameObject(gameObject);
                newObject->CreateRootComponent();

                // TODO: Deserialize the rest of components and add them to their corresponding gameObject,
                // once the serialization is also done

                loadedGameObjects.push_back(newObject);
            }
        }

        std::vector<Component*> loadedComponents;
        if (prefab.HasMember("Components") && prefab["Components"].IsArray())
        {
            const rapidjson::Value& components = prefab["Components"];
            for (rapidjson::SizeType i = 0; i < components.Size(); i++)
            {
                const rapidjson::Value& component = components[i];
            }
        }

        ResourcePrefab* resourcePrefab = new ResourcePrefab(uid, name);
        resourcePrefab->LoadData(loadedGameObjects);
        return resourcePrefab;
    }
} // namespace PrefabManager
