#include "PathfinderModule.h"

#include "Application.h"
#include "LibraryModule.h"
#include "SceneModule.h"
#include "ResourcesModule.h"
#include "ResourceNavmesh.h"

#include <DetourCommon.h>
#include <DetourCrowd.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>

PathfinderModule::PathfinderModule()
{
}

PathfinderModule::~PathfinderModule()
{
    dtFreeCrowd(crowd);
    dtFreeNavMeshQuery(navQuery);
    delete navmesh;
}


update_status PathfinderModule::Update(float deltaTime)
{

    if (!App->GetSceneModule()->GetInPlayMode()) return UPDATE_CONTINUE;

    if (deltaTime <= 0.0f) return UPDATE_CONTINUE;

    if (crowd->getAgentCount() > 0)
    {
        crowd->update(deltaTime, nullptr);
    }

    return UPDATE_CONTINUE;
}
/*
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
*/

// All ai agent components will call this to add themselves to crowd
int PathfinderModule::CreateAgent(float3 position, float radius, float height, float speed)
{
    if (!crowd) return -1;

    dtCrowdAgentParams params;
    memset(&params, 0, sizeof(params));
    params.radius                = radius;
    params.height                = height;
    params.maxSpeed              = speed;
    params.maxAcceleration       = 8.0f;
    params.collisionQueryRange   = radius * 12.0f;
    params.pathOptimizationRange = radius * 30.0f;
    params.updateFlags           = DT_CROWD_ANTICIPATE_TURNS | DT_CROWD_OBSTACLE_AVOIDANCE | DT_CROWD_SEPARATION;
    params.obstacleAvoidanceType = 3;
    params.separationWeight      = 2.0f;

    float pos[3]                 = {position.x, position.y, position.z};
    int agentId                  = crowd->addAgent(pos, &params);
    return agentId;
}

void PathfinderModule::RemoveAgent(int agentId)
{
    if (!crowd) return;
    if (agentId < 0 || agentId >= crowd->getAgentCount()) return;

    const dtCrowdAgent* agent = crowd->getAgent(agentId);
    if (!agent || !agent->active) return;

    crowd->removeAgent(agentId);
}

void PathfinderModule::InitQuerySystem()
{
    if (crowd == nullptr) crowd = dtAllocCrowd(); 

    navmesh  = App->GetResourcesModule()->GetNavMesh();
    navQuery = navmesh->GetDetourNavMeshQuery();

    if (crowd != nullptr && navmesh != nullptr)
    {
        crowd->init(maxAgents, maxAgentRadius, navmesh->GetDetourNavMesh());
    }
    else
    {
        GLOG("Failed to initialize Pathfinder crowd system.");
    }
}
