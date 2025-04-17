#include "NavmeshImporter.h"

#include "Application.h"
#include "DetourNavMesh.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "MetaNavmesh.h"
#include "NavMeshConfig.h"
#include "ResourceNavmesh.h"

struct NavMeshSetHeader
{
    int magic;
    int version;
    dtNavMeshParams params;
};


UID NavmeshImporter::SaveNavmesh(const char* name, ResourceNavMesh* resource, const NavMeshConfig& config)
{

    const int NAVMESHSET_MAGIC = 'S' << 24 | 'O' << 16 | 'B' << 8 | 'R';
    const int NAVMESHSET_VERSION = 1;

    dtNavMesh* navmesh = resource->GetDetourNavMesh();
    if (!navmesh) return 0;

    const dtNavMeshParams* params = navmesh->getParams();

    // Get raw navmesh data (we assume 1 tile, single data block)
    const dtMeshTile* tile = navmesh->getTileAt(0, 0, 0);

    if (!tile || !tile->data || tile->dataSize == 0) return 0;

    // Fill header
    NavMeshSetHeader header;
    header.magic = NAVMESHSET_MAGIC;
    header.version = NAVMESHSET_VERSION;
    memcpy(&header.params, params, sizeof(dtNavMeshParams));

    // Allocate buffer
    const size_t totalSize = sizeof(NavMeshSetHeader) + tile->dataSize;

    char* fileBuffer = new char[totalSize];
    char* cursor = fileBuffer;

    memcpy(cursor, &header, sizeof(NavMeshSetHeader));
    cursor += sizeof(NavMeshSetHeader);
    memcpy(cursor, tile->data, tile->dataSize);

    // Generate UID and file path
    UID navmeshUID = GenerateUID();
    navmeshUID = App->GetLibraryModule()->AssignFiletypeUID(navmeshUID, FileType::Navmesh);

    std::string metaNavPath = NAVMESHES_PATH + std::string(name) + META_EXTENSION;
    std::string navPath = NAVMESHES_PATH + std::string(name) + NAVMESH_EXTENSION;

    // Save metadata 
    MetaNavmesh meta(navmeshUID, metaNavPath, config);
    meta.Save(name, metaNavPath);

    // Write binary navmesh
    unsigned int bytesWritten = FileSystem::Save(navPath.c_str(), fileBuffer, (unsigned int)totalSize);

    delete[] fileBuffer;

    App->GetLibraryModule()->AddResource(navPath, navmeshUID);
    App->GetLibraryModule()->AddName(name, navmeshUID);

    GLOG("%s saved navmesh binary.", name);

    return navmeshUID;
}


ResourceNavMesh* NavmeshImporter::LoadNavmesh(UID navmeshUID)
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