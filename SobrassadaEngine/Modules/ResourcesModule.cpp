#include "ResourcesModule.h"
#include "Application.h"
#include "Importer.h"
#include "LibraryModule.h"
#include "MeshImporter.h"
#include "ResourceManagement/Resources/ResourceMaterial.h"
#include "ResourceManagement/Resources/ResourceMesh.h"
#include "ResourceManagement/Resources/ResourceTexture.h"
#include "Scene/Components/Standalone/MeshComponent.h"
#include "SceneModule.h"
#include "ShaderModule.h"

#include <Algorithm/Random/LCG.h> // TODO: LCG remove includes

ResourcesModule::ResourcesModule() : myNavmesh(15345456565, "defaultName")
{
}

ResourcesModule::~ResourcesModule()
{
}

bool ResourcesModule::Init()
{
    return true;
}

bool ResourcesModule::ShutDown()
{
    for (auto resource : resources)
    {
        delete resource.second;
    }
    resources.clear();
    return true;
}

Resource* ResourcesModule::RequestResource(UID uid)
{
    // Find if the resource is already loaded
    std::map<UID, Resource*>::iterator it = resources.find(uid);

    if (it != resources.end())
    {
        it->second->AddReference();
        return it->second;
    }

    // Find the library file (if exists) and load the custom file format
    return CreateNewResource(uid);
}

void ResourcesModule::ReleaseResource(const Resource* resource)
{
    if (resource != nullptr)
    {
        std::map<UID, Resource*>::iterator it = resources.find(resource->GetUID());

        if (it != resources.end())
        {
            it->second->RemoveReference();
            if (it->second->GetReferenceCount() <= 0)
            {
                delete it->second;
                resources.erase(it);
            }
        }
    }
}

Resource* ResourcesModule::CreateNewResource(UID uid)
{
    Resource* loadedResource = Importer::Load(uid);
    if (loadedResource != nullptr)
    {
        resources.insert(std::pair(uid, loadedResource));
        loadedResource->AddReference();
        return loadedResource;
    }
    return nullptr;
}

void ResourcesModule::UnloadAllResources()
{
    for (auto resource : resources)
    {
        delete resource.second;
    }
    resources.clear();
}

void ResourcesModule::CreateNavMesh()
{

    std::vector<std::pair<const ResourceMesh*, const float4x4&>> meshes;
    float minPos[3]                                         = {FLT_MAX, FLT_MAX, FLT_MAX};
    float maxPos[3]                                         = {FLT_MIN, FLT_MIN, FLT_MIN};

    const std::unordered_map<UID, GameObject*>& gameObjects = App->GetSceneModule()->GetScene()->GetAllGameObjects();

    if (!gameObjects.empty())
    {
        for (const auto& pair : gameObjects)
        {
            GameObject* gameObject = pair.second;
            if (gameObject)
            {
                const MeshComponent* meshComponent = gameObject->GetMeshComponent();
                const float4x4 globalMatrix        = gameObject->GetGlobalTransform();

                if (meshComponent)
                {
                    const ResourceMesh* resourceMesh = meshComponent->GetResourceMesh();
                    AABB aabb                        = gameObject->GetGlobalAABB();

                    minPos[0]                        = std::min(minPos[0], aabb.minPoint.x);
                    minPos[1]                        = std::min(minPos[1], aabb.minPoint.y);
                    minPos[2]                        = std::min(minPos[2], aabb.minPoint.z);

                    maxPos[0]                        = std::max(maxPos[0], aabb.maxPoint.x);
                    maxPos[1]                        = std::max(maxPos[1], aabb.maxPoint.y);
                    maxPos[2]                        = std::max(maxPos[2], aabb.maxPoint.z);

                    meshes.push_back({resourceMesh, globalMatrix});
                }
            }
        }
    }
    myNavmesh.BuildNavMesh(meshes, minPos, maxPos);
}
