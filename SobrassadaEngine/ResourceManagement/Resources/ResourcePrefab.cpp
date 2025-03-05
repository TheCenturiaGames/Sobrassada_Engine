#include "ResourcePrefab.h"

ResourcePrefab::ResourcePrefab(UID uid, const std::string& name) : Resource(uid, name, ResourceType::Prefab)
{
    rootGameObject = nullptr;
}

ResourcePrefab::~ResourcePrefab()
{
}