#pragma once

#include <Globals.h>

class ResourceNavmesh;
class dtNavMesh;

namespace NavmeshImporter
{
    UID SaveNavmesh(const char* path, const dtNavMesh& navmesh);

    ResourceNavmesh* LoadNavmesh(UID navmeshUID);
}; // namespace NavmeshImporter
