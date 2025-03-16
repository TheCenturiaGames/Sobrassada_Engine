#pragma once

#include "Libs/RecastDetour/Detour/Include/DetourNavMeshBuilder.h"
#include "Libs/RecastDetour/Detour/Include/DetourNavMeshQuery.h"
#include "Libs/RecastDetour/Recast/Include/Recast.h"
#include "Resource.h"

class ResourceMesh;

enum SamplePartitionType
{
    SAMPLE_PARTITION_WATERSHED,
    SAMPLE_PARTITION_MONOTONE,
    SAMPLE_PARTITION_LAYERS
};

enum SamplePolyAreas
{
    SAMPLE_POLYAREA_GROUND,
    SAMPLE_POLYAREA_WATER,
    SAMPLE_POLYAREA_ROAD,
    SAMPLE_POLYAREA_DOOR,
    SAMPLE_POLYAREA_GRASS,
    SAMPLE_POLYAREA_JUMP
};

enum SamplePolyFlags
{
    SAMPLE_POLYFLAGS_WALK     = 0x01,  // Ability to walk (ground, grass, road)
    SAMPLE_POLYFLAGS_SWIM     = 0x02,  // Ability to swim (water).
    SAMPLE_POLYFLAGS_DOOR     = 0x04,  // Ability to move through doors.
    SAMPLE_POLYFLAGS_JUMP     = 0x08,  // Ability to jump.
    SAMPLE_POLYFLAGS_DISABLED = 0x10,  // Disabled polygon
    SAMPLE_POLYFLAGS_ALL      = 0xffff // All abilities.
};

class ResourceNavMesh : public Resource
{

  public:
    ResourceNavMesh(UID uid, const std::string& name);
    ~ResourceNavMesh() override;

    bool BuildNavMesh(
        const std::vector<std::pair<const ResourceMesh*, const float4x4&>>& meshes, float3& minPoint, float3& maxPoint
    );
    void Render();

    void SetHeightfieldOptions(const float3 bmin, const float3 bmax, float cellSize, float cellHeight);
    void SetWalkableOptions(
        const float walkableSlopeAngle, const float walkabkeClimb, const float walkableHeight,
        const float walkableRadius
    );
    void
    SetFilterOptions(bool m_filterLowHangingObstacles, bool m_filterLedgeSpans, bool m_filterWalkableLowHeightSpans);
    void SetPartitionOptions(int partitionType, int minRegionArea, int mergeRegionArea);
    void SetContourOptions(float maxSimplificationError, int maxEdgeLen, int maxVertsPerPoly);
    void SetPolyMeshOptions(int maxVertsPerPoly, float detailSampleDist, float detailSampleMaxError);

    dtNavMesh* GetDetourNavMesh() { return navMesh; }
    dtNavMeshQuery* GetDetourNavMeshQuery() { return navQuery; }

  private:
    rcConfig config;

    rcContext* context;
    rcHeightfield* heightfield;
    rcCompactHeightfield* compactHeightfield;
    rcContourSet* contourSet;
    rcPolyMesh* polymesh;
    rcPolyMeshDetail* polymeshDetail;
    dtNavMesh* navMesh;
    dtNavMeshQuery* navQuery;

    bool m_filterLowHangingObstacles;
    bool m_filterLedgeSpans;
    bool m_filterWalkableLowHeightSpans;

    float m_agentMaxClimb;
    float m_agentHeight;
    float m_agentRadius;

    unsigned char* triAreas;

    int partitionType;
};