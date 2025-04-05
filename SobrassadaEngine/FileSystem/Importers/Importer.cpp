#include "Importer.h"

#include "Application.h"
#include "LibraryModule.h"
#include "MaterialImporter.h"
#include "MeshImporter.h"
#include "ModelImporter.h"
#include "PrefabManager.h"
#include "Resource.h"
#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "ResourceModel.h"
#include "ResourcePrefab.h"
#include "ResourceTexture.h"
#include "TextureImporter.h"

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
    case ResourceType::Model:
        return ModelImporter::LoadModel(uid);
    case ResourceType::Prefab:
        return PrefabManager::LoadPrefab(uid);

    default:
        GLOG("Unknown resource type: %d", type)
        return nullptr;
    }

    return nullptr;
}