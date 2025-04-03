#include "PathfinderModule.h"

#include "Application.h"
#include "SceneModule.h"
#include "LibraryModule.h"


#include <DetourCrowd.h>
#include <DetourNavMesh.h>
#include <DetourCommon.h>
#include <DetourNavMeshQuery.h>



PathfinderModule::PathfinderModule()
{
}

PathfinderModule::~PathfinderModule()
{
    dtFreeNavMeshQuery(navQuery);
}

bool PathfinderModule::Init()
{

    crowd = dtAllocCrowd();
    return true;
}

update_status PathfinderModule::Update(float deltaTime)
{

    if (!App->GetSceneModule()->GetInPlayMode()) return;

    if (deltaTime <= 0.0f) return;

    if (crowd->getAgentCount() > 0)
    {
        crowd->update(deltaTime, nullptr);
    }

    return update_status();
}

std::vector<float3> PathfinderModule::FindPath(float3 start, float3 end)
{
    std::vector<float3> path;

    dtQueryFilter filter;
    float startPos[3] = {start.x, start.y, start.z};
    float endPos[3]   = {end.x, end.y, end.z};

    dtPolyRef startRef, endRef;
    float closestStart[3], closestEnd[3];

    float extent[3] = {2.0f, 4.0f, 2.0f};

    // Find closest navmesh polygons
    navQuery->findNearestPoly(startPos, extent, &filter, &startRef, closestStart);
    navQuery->findNearestPoly(endPos, extent, &filter, &endRef, closestEnd);

    if (!startRef || !endRef) return path; // No valid path

    // Find path through navmesh
    dtPolyRef pathPolys[256];
    int pathCount = 0;
    navQuery->findPath(startRef, endRef, closestStart, closestEnd, &filter, pathPolys, &pathCount, 256);

    if (pathCount == 0) return path; 

    // straight-line path
    float straightPath[256 * 3];          // positions (x,y,z)
    unsigned char straightPathFlags[256]; // flags for path points
    dtPolyRef straightPathRefs[256];      // polygon references
    int straightPathCount = 0;

    navQuery->findStraightPath(
        closestStart, closestEnd, pathPolys, pathCount, straightPath, straightPathFlags, straightPathRefs,
        &straightPathCount, 256
    );

    // Store waypoints
    for (int j = 0; j < straightPathCount; j++)
    {
        path.emplace_back(straightPath[j * 3], straightPath[j * 3 + 1], straightPath[j * 3 + 2]);
    }

    return path;
}

void PathfinderModule::QueryNavmesh(UID uid)
{
}
