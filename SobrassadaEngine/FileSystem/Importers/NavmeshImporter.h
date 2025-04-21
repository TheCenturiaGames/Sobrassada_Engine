#pragma once

#include "Globals.h"

class NavMeshConfig;
class ResourceNavMesh;
class dtNavMesh;

namespace NavmeshImporter
{
    UID SaveNavmesh(const char* name, const ResourceNavMesh* resource, const NavMeshConfig& config);
    ResourceNavMesh* LoadNavmesh(UID navmeshUID);
    
}; // namespace NavmeshImporter
