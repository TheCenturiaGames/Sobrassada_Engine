#include "Importer.h"

#include "Application.h"
#include "LibraryModule.h"
#include "MaterialImporter.h"
#include "MeshImporter.h"
#include "TextureImporter.h"
#include "PrefabManager.h"

Resource* Importer::Load(UID uid)
{
    const ResourceType type = Resource::GetResourceTypeForUID(uid);

    switch (type)
    {
    case ResourceType::Unknown:
        return nullptr;
    case ResourceType::Texture:
        return TextureImporter::LoadTexture(uid);
    case ResourceType::Material:
        return MaterialImporter::LoadMaterial(uid);
    case ResourceType::Mesh:
        return MeshImporter::LoadMesh(uid);
    case ResourceType::Prefab:
        return PrefabManager::LoadPrefab(uid);

    default:
        GLOG("Unknown resource type: %d", type)
        return nullptr;
    }

    return nullptr;
}