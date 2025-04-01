#pragma once

#include <Globals.h>

class ResourceNavmesh;
class dtNavMesh;

namespace NavmeshImporter
{
    void SaveNavmesh(const char* path, const dtNavMesh& navmesh);

    ResourceNavmesh* LoadNavmesh(UID navmeshUID);
}; // namespace NavmeshImporter
