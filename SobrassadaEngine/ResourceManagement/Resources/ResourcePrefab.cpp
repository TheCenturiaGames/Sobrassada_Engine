#include "ResourcePrefab.h"
#include "GameObject.h"

#include "GameObject.h"


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
}



GameObject* ResourcePrefab::FindGameObject(UID uid) const
{
    for (GameObject* gameobject : gameObjects)
    {
        if (gameobject->GetUID() == uid) return gameobject;
    }
    return nullptr;
}
