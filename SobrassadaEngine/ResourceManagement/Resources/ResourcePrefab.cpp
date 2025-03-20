#include "ResourcePrefab.h"

ResourcePrefab::ResourcePrefab(UID uid, const std::string& name) : Resource(uid, name, ResourceType::Prefab)
{
}

ResourcePrefab::~ResourcePrefab()
{
}

void ResourcePrefab::LoadData(const std::vector<GameObject*>& objects, const std::vector<int>& indices)
{
    for (int i = 0; i < objects.size(); ++i)
    {
        gameObjects.emplace_back(objects[i]);
        parentIndices.push_back(indices[i]);
    }
}