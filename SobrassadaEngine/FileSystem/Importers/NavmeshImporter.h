#pragma once

#include <Globals.h>

class ResourceNavmesh;

namespace NavmeshImporter
{
    UID ImportNavmesh(
        const std::string& name, const char* filePath, const std::string& targetFilePath, UID sourceUID = INVALID_UID
    );

    ResourceNavmesh* LoadNavmesh(UID navmeshUID);
}; // namespace NavmeshImporter
