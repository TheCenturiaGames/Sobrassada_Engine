#include "BatchManager.h"

#include "GeometryBatch.h"
#include <ResourceMesh.h>

BatchManager::BatchManager()
{
}

BatchManager ::~BatchManager()
{
    batches.clear();
    batches.shrink_to_fit();
}

GeometryBatch* BatchManager::RequestBatch(const ResourceMesh* mesh)
{
    if (batches.empty())
    {
        return CreateNewBatch(mesh);
    }

    for (GeometryBatch* it : batches)
    {
        if (it->GetMode() == mesh->GetMode())
        {
            return it;
        }
    }

    return CreateNewBatch(mesh);
}

GeometryBatch* BatchManager::CreateNewBatch(const ResourceMesh* mesh)
{
    GeometryBatch* newBatch = new GeometryBatch(mesh);
    batches.push_back(newBatch);
    return newBatch;
}
