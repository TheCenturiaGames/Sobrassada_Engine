#include "BatchManager.h"

#include "GeometryBatch.h"
#include <ResourceMesh.h>
#include <Standalone/MeshComponent.h>

BatchManager::BatchManager()
{
}

BatchManager::~BatchManager()
{
    for (GeometryBatch* it : batches)
    {
        delete it;
    }
    batches.clear();
    batches.shrink_to_fit();
}

void BatchManager::LoadData()
{
    for (GeometryBatch* it : batches)
        it->LoadData();
}

void BatchManager::Render()
{
    for (GeometryBatch* it : batches)
        it->Render();
}

void BatchManager::ClearObjectsToRender()
{
    for (GeometryBatch* it : batches)
        it->ClearObjectsToRender();
}

GeometryBatch* BatchManager::RequestBatch(const MeshComponent* mesh)
{
    if (batches.empty())
    {
        return CreateNewBatch(mesh);
    }

    const ResourceMesh* resource = mesh->GetResourceMesh();

    for (GeometryBatch* it : batches)
    {
        if (it->GetMode() == resource->GetMode())
        {
            return it;
        }
    }

    return CreateNewBatch(mesh);
}

GeometryBatch* BatchManager::CreateNewBatch(const MeshComponent* mesh)
{
    GeometryBatch* newBatch = new GeometryBatch(mesh, mesh->GetResourceMesh());
    batches.push_back(newBatch);
    return newBatch;
}
