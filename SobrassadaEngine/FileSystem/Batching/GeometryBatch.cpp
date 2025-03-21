#include "GeometryBatch.h"

#include <ResourceMesh.h>
#include <Standalone/MeshComponent.h>

GeometryBatch::GeometryBatch(const MeshComponent* mesh, const ResourceMesh* resource) : mode(resource->GetMode())
{
    AddComponent(mesh);
    AddResource(resource);
}

GeometryBatch ::~GeometryBatch()
{
}