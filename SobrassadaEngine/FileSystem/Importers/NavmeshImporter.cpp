#include "NavmeshImporter.h"

#include "Application.h"
#include "DetourNavMesh.h"
#include "FileSystem.h"
#include "LibraryModule.h"

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

UID NavmeshImporter::SaveNavmesh(const char* name, const dtNavMesh& navmesh)
{
    unsigned int size     = 0;
    unsigned int byteSize = 0;

    NavMeshSetHeader header;

    header.magic    = NAVMESHSET_MAGIC;
    header.version  = NAVMESHSET_VERSION;
    header.numTiles = 0;

    for (int i = 0; i < navmesh.getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navmesh.getTile(i);
        if (!tile || !tile->header || !tile->dataSize) continue;
        header.numTiles++;
    }
    memcpy(&header.params, navmesh.getParams(), sizeof(dtNavMeshParams));
    size += sizeof(NavMeshSetHeader);

    for (int i = 0; i < navmesh.getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navmesh.getTile(i);
        if (!tile || !tile->header || !tile->dataSize) continue;

        NavMeshTileHeader tileHeader;
        tileHeader.tileRef   = navmesh.getTileRef(tile);
        tileHeader.dataSize  = tile->dataSize;

        size                += sizeof(tile->dataSize);
        size                += tile->dataSize;
    }

    char* fileBuffer   = new char[size];
    char* cursor       = fileBuffer;

    unsigned int bytes = sizeof(NavMeshSetHeader);
    memcpy(cursor, &header, bytes);
    cursor       += bytes;
    byteSize     += bytes;
    int maxTiles  = navmesh.getMaxTiles();

    for (int i = 0; i < maxTiles; ++i)
    {
        const dtMeshTile* tile = navmesh.getTile(i);
        if (!tile || !tile->header || !tile->dataSize) continue;

        NavMeshTileHeader tileHeader;
        tileHeader.tileRef  = navmesh.getTileRef(tile);
        tileHeader.dataSize = tile->dataSize;
        bytes               = sizeof(tile->dataSize);
        memcpy(cursor, &tileHeader.dataSize, bytes);
        cursor   += bytes;
        byteSize += bytes;
        bytes     = tile->dataSize;
        memcpy(cursor, tile->data, bytes);
        if (i + 1 < maxTiles)
        {
            cursor   += bytes;
            byteSize += bytes;
        }
    }

    UID navmeshUID            = GenerateUID();
    navmeshUID                = App->GetLibraryModule()->AssignFiletypeUID(navmeshUID, FileType::Navmesh);

    std::string navpath       = NAVMESHES_PATH + std::string(name) + NAVMESH_EXTENSION; 

    unsigned int bytesWritten = (unsigned int)FileSystem::Save(navpath.c_str(), fileBuffer, size, true);

    delete[] fileBuffer;

    // added navmesh to resources
    App->GetLibraryModule()->AddResource(navpath, navmeshUID);

    GLOG("%s saved as binary", name);

    FileSystem::Save(navpath.c_str(), fileBuffer, size);

    return navmeshUID;

    delete[] fileBuffer;
}

ResourceNavmesh* NavmeshImporter::LoadNavmesh(UID navmeshUID)
{
    return nullptr;
}