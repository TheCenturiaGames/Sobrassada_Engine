#pragma once

#include "Globals.h"

class NavMeshConfig;
class ResourceNavmesh;
class dtNavMesh;

namespace NavmeshImporter
{
    UID SaveNavmesh(const char* name, const unsigned char* navData, const int dataSize, const NavMeshConfig& config);
    ResourceNavmesh* LoadNavmesh(UID navmeshUID);
    
}; // namespace NavmeshImporter
