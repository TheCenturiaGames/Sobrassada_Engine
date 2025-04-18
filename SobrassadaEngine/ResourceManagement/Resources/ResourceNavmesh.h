#pragma once

#include "Resource.h"

class ResourceMesh;
class dtNavMesh;
class dtNavMeshQuery;
struct rcConfig;
class rcContext;
struct rcHeightfield;
struct rcCompactHeightfield;
struct rcContourSet;
struct rcPolyMesh;
struct rcPolyMeshDetail;
class ResourceMesh;

struct Vertex;

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
        const std::vector<std::pair<const ResourceMesh*, const float4x4&>>& meshes, const float minPoint[3],
        const float maxPoint[3]
    );

    void RenderNavmeshEditor();
    void CreateDetourData();

    dtNavMesh* GetDetourNavMesh() const { return navMesh; }
    dtNavMeshQuery* GetDetourNavMeshQuery() const { return navQuery; }


  private:
    rcConfig* config                         = nullptr;
    rcHeightfield* heightfield               = nullptr;
    rcCompactHeightfield* compactHeightfield = nullptr;
    rcContourSet* contourSet                 = nullptr;
    rcPolyMesh* polymesh                     = nullptr;
    rcPolyMeshDetail* polymeshDetail         = nullptr;
    dtNavMesh* navMesh                       = nullptr;
    dtNavMeshQuery* navQuery                 = nullptr;

    bool m_filterLowHangingObstacles;
    bool m_filterLedgeSpans;
    bool m_filterWalkableLowHeightSpans;

    float m_agentMaxClimb;
    float m_agentHeight;
    float m_agentRadius;

    int partitionType;
};