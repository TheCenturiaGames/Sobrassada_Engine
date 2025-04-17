#include "NavmeshImporter.h"

#include "Application.h"
#include "DetourNavMesh.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "MetaNavmesh.h"
#include "NavMeshConfig.h"

struct NavMeshSetHeader
{
    int magic;
    int version;
    int numTiles;
    dtNavMeshParams params;
};

struct NavMeshTileHeader
{
    dtTileRef tileRef;
    int dataSize;
};

const int NAVMESHSET_MAGIC   = 'S' << 24 | 'O' << 16 | 'B' << 8 | 'R';
const int NAVMESHSET_VERSION = 1;

UID NavmeshImporter::SaveNavmesh(const char* name, const unsigned char* navData, const int dataSize, const NavMeshConfig& config)
{
    if (!navData || dataSize <= 0)
    {
        GLOG("Invalid navmesh data.");
        return 0;
    }

    // Save raw navmesh data
    UID navmeshUID = GenerateUID();
    navmeshUID = App->GetLibraryModule()->AssignFiletypeUID(navmeshUID, FileType::Navmesh);

    std::string navpath = NAVMESHES_PATH + std::string(name) + NAVMESH_EXTENSION;
    FileSystem::Save(navpath.c_str(), navData, dataSize, true);

    // Save config to meta
    MetaNavmesh meta(navmeshUID, navpath, config);
    meta.Save(name, navpath);

    App->GetLibraryModule()->AddResource(navpath, navmeshUID);
    App->GetLibraryModule()->AddName(name, navmeshUID);

    GLOG("%s saved as single-tile navmesh", name);
    return navmeshUID;
}

ResourceNavmesh* NavmeshImporter::LoadNavmesh(UID navmeshUID)
{
    return nullptr;
}
/*
ResourceNavMesh* NavmeshImporter::LoadNavmesh(UID navmeshUID)
{

    if (navmeshPath.empty())
    {
        GLOG("Failed to find navmesh path for UID %llu", navmeshUID);
        return nullptr;
    }

    char* fileBuffer = nullptr;
    unsigned int size = FileSystem::Load(navmeshPath.c_str(), &fileBuffer);
    if (size == 0 || fileBuffer == nullptr)
    {
        GLOG("Failed to load navmesh file: %s", navmeshPath.c_str());
        return nullptr;
    }

    char* cursor = fileBuffer;

    // Read and validate header
    NavMeshSetHeader header;
    memcpy(&header, cursor, sizeof(NavMeshSetHeader));
    cursor += sizeof(NavMeshSetHeader);

    if (header.magic != NAVMESHSET_MAGIC || header.version != NAVMESHSET_VERSION)
    {
        GLOG("Invalid navmesh file header: %s", navmeshPath.c_str());
        delete[] fileBuffer;
        return nullptr;
    }

    dtNavMesh* navMesh = dtAllocNavMesh();
    if (!navMesh || dtStatusFailed(navMesh->init(&header.params)))
    {
        GLOG("Failed to init Detour navmesh");
        delete[] fileBuffer;
        return nullptr;
    }

    // Load tiles
    for (int i = 0; i < header.numTiles; ++i)
    {
        int dataSize = 0;
        memcpy(&dataSize, cursor, sizeof(int));
        cursor += sizeof(int);

        if (dataSize == 0) continue;

        unsigned char* tileData = (unsigned char*)dtAlloc(dataSize, DT_ALLOC_PERM);
        if (!tileData)
        {
            GLOG("Failed to allocate tile data");
            continue;
        }

        memcpy(tileData, cursor, dataSize);
        cursor += dataSize;

        dtTileRef tileRef = 0;
        navMesh->addTile(tileData, dataSize, DT_TILE_FREE_DATA, 0, &tileRef);
    }

    delete[] fileBuffer;

    // Allocate navmesh query
    dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
    if (!navQuery || dtStatusFailed(navQuery->init(navMesh, 2048)))
    {
        GLOG("Failed to create navmesh query");
        dtFreeNavMesh(navMesh);
        return nullptr;
    }

    // Create the resource object and set its navmesh
    ResourceNavMesh* resource = new ResourceNavMesh();
    resource->SetNavMesh(navMesh);
    resource->SetNavMeshQuery(navQuery);
    resource->SetUID(navmeshUID);
    resource->SetName(navmeshPath);

    return resource;
}*/