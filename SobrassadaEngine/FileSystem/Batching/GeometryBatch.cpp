#include "GeometryBatch.h"

#include <ResourceMesh.h>

GeometryBatch::GeometryBatch(const ResourceMesh* mesh) : mode(mesh->GetMode())
{
}

GeometryBatch ::~GeometryBatch()
{
}