#include "PrefabManager.h"

#include "Application.h"
#include "FileSystem.h"
#include "ResourceManagement/Resources/ResourcePrefab.h"
#include "Scene/GameObjects/GameObject.h"
#include "SceneModule.h"

#include "prettywriter.h"
#include "stringbuffer.h"
#include <filesystem>
#include <queue>

namespace PrefabManager
{
    UID SavePrefab(const GameObject* gameObject)
    {
        // Create doc JSON
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value prefab(rapidjson::kObjectType);

        // Scene values
        UID uid                 = GenerateUID();
        const std::string& name = gameObject->GetName();

        // Create structure
        prefab.AddMember("UID", uid, allocator);
        prefab.AddMember("Name", rapidjson::Value(name.c_str(), allocator), allocator);

        // Serialize GameObjects
        rapidjson::Value gameObjectsJSON(rapidjson::kArrayType);
        std::queue<const GameObject*> queue; // Traverse all gameObjects using a queue to avoid recursiveness
        queue.push(gameObject);

        while (queue.size() > 0)
        {
            const GameObject* currentGameObject = queue.front();

            // DO whatever
            rapidjson::Value goJSON(rapidjson::kObjectType);
            currentGameObject->Save(goJSON, allocator);
            gameObjectsJSON.PushBack(goJSON, allocator);

            for (UID child : currentGameObject->GetChildren())
            {
                queue.push(App->GetSceneModule()->GetGameObjectByUUID(child));
            }

            queue.pop();
        }

        // Add gameObjects to scene
        prefab.AddMember("GameObjects", gameObjectsJSON, allocator);

        // Serialize Components
        // rapidjson::Value componentsJSON(rapidjson::kArrayType);
        //
        // for (auto it = components->begin(); it != components->end(); ++it)
        //{
        //    if (it->second != nullptr)
        //    {
        //        rapidjson::Value componentJSON(rapidjson::kObjectType);
        //
        //        it->second->Save(componentJSON, allocator);
        //
        //        componentsJSON.PushBack(componentJSON, allocator);
        //    }
        //}
        //
        //// Add components to scene
        // scene.AddMember("Components", componentsJSON, allocator);

        doc.AddMember("Prefab", prefab, allocator);

        // Save file like JSON
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::string prefabFilePath = std::filesystem::current_path().string() + DELIMITER + PREFABS_PATH + name + PREFAB_EXTENSION;

        unsigned int bytesWritten = (unsigned int
        )FileSystem::Save(prefabFilePath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);
        if (bytesWritten == 0)
        {
            GLOG("Failed to save prefab file: %s", prefabFilePath);
            return CONSTANT_EMPTY_UID;
        }

        return uid;
    }

    ResourcePrefab* LoadPrefab(UID prefabUID)
    {
        return nullptr;
    }
} // namespace PrefabManager
