#include "ResourcesModule.h"

#include "Application.h"
#include "FileSystem/Importer.h"
#include "FileSystem/MeshImporter.h"
#include "LibraryModule.h"
#include "ResourceManagement/Resources/ResourceMaterial.h"
#include "ResourceManagement/Resources/ResourceMesh.h"
#include "ResourceManagement/Resources/ResourceTexture.h"
#include "SceneModule.h"
#include "ShaderModule.h"

#include <Algorithm/Random/LCG.h> // TODO: LCG remove includes

ResourcesModule::ResourcesModule()
{
}

ResourcesModule::~ResourcesModule()
{
    
}

bool ResourcesModule::Init()
{
    program = App->GetShaderModule()->GetProgram("./Test/VertexShader.glsl", "./Test/BRDFPhongFragmentShader.glsl");

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
