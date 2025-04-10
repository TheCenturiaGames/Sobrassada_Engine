#include "ResourcePrefab.h"
#include "GameObject.h"

#include "GameObject.h"
#include "PrefabImporter.h"
#include <FileSystem.h>
#include <Application.h>
#include "LibraryModule.h"


ResourcePrefab::ResourcePrefab(UID uid, const std::string& name) : Resource(uid, name, ResourceType::Prefab)
{
}

ResourcePrefab::~ResourcePrefab()
{
    for (auto& object : gameObjects)
    {
        // Why this doesn't call the gameObject destructor????
        delete object;
    }
    gameObjects.clear();
    parentIndices.clear();
}

void ResourcePrefab::LoadData(const std::vector<GameObject*>& objects, const std::vector<int>& indices)
{
    gameObjects.clear();
    parentIndices = indices;

    // Clonar tots els GameObject (són còpies independents de les que provenen de JSON)
    for (GameObject* original : objects)
    {
        GameObject* clone = new GameObject(*original); // Copia els components
        gameObjects.push_back(clone);
    }

    // Reconstruir la jerarquia pare-fill usant parentIndices
    for (size_t i = 0; i < gameObjects.size(); ++i)
    {
        int parentIdx = parentIndices[i];

        if (parentIdx >= 0 && parentIdx < (int)gameObjects.size())
        {
            GameObject* parent = gameObjects[parentIdx];
            GameObject* child  = gameObjects[i];

            if (parent->GetUID() == child->GetUID())
            {
                continue;
            }

            child->SetParent(parent->GetUID());
            parent->AddGameObject(child->GetUID());
        }
        else if (parentIdx >= (int)gameObjects.size())
        {
            GLOG("WARNING: Invalid parent index %d for object %zu", parentIdx, i);
        }
    }

    for (GameObject* go : gameObjects)
    {
        go->SetPrefabUID(GetUID());
    }
}



GameObject* ResourcePrefab::FindGameObject(UID uid) const
{
    for (GameObject* gameobject : gameObjects)
    {
        if (gameobject->GetUID() == uid) return gameobject;
    }
    return nullptr;
}

bool ResourcePrefab::SaveToDisk() const
{
    if (gameObjects.empty()) return false;

    const GameObject* root = gameObjects[0];
    if (!root) return false;

    PrefabImporter::SavePrefab(root, true);
    return true;
}


bool ResourcePrefab::SaveClonedPrefab(const std::vector<GameObject*>& clonedObjects) const
{
    if (clonedObjects.empty()) return false;

    for (GameObject* go : clonedObjects)
    {
        go->SetPrefabUID(GetUID());
    }

    return PrefabImporter::SavePrefab(clonedObjects[0], true);
}

void ResourcePrefab::ReloadFromDisk()
{
    std::string path  = App->GetLibraryModule()->GetResourcePath(GetUID());

    char* buffer      = nullptr;
    unsigned int size = FileSystem::Load(path.c_str(), &buffer, false);

    if (size == 0 || buffer == nullptr)
    {
        GLOG("Failed to reload prefab from disk: %s", path.c_str());
        return;
    }

    rapidjson::Document doc;
    doc.Parse(buffer);
    delete[] buffer;

    if (!doc.HasMember("Prefab") || !doc["Prefab"].HasMember("GameObjects")) return;

    const rapidjson::Value& gameObjectsJSON = doc["Prefab"]["GameObjects"];

    for (GameObject* go : gameObjects)
    {
        delete go;
    }
    gameObjects.clear();

    for (rapidjson::SizeType i = 0; i < gameObjectsJSON.Size(); ++i)
    {
        const rapidjson::Value& goData = gameObjectsJSON[i];
        GameObject* newGO              = new GameObject(goData); 
        gameObjects.push_back(newGO);
    }
}


