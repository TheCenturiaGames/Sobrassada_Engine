#include "PathfinderModule.h"

#include "Application.h"
#include "SceneModule.h"

#include <DetourCrowd.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
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
    navQuery->findNearestPoly(startPos, {2, 4, 2}, &filter, &startRef, closestStart);
    navQuery->findNearestPoly(endPos, {2, 4, 2}, &filter, &endRef, closestEnd);

    if (!startRef || !endRef) return path; // No valid path

    dtPolyRef pathPolys[256];
    int pathCount = 0;
    navQuery->findPath(startRef, endRef, closestStart, closestEnd, &filter, pathPolys, &pathCount, 256);

    if (pathCount > 0)
    {
        float smoothPos[3];
        dtPolyRef smoothPathPolys[256];
        memcpy(smoothPathPolys, pathPolys, sizeof(dtPolyRef) * pathCount);
        int smoothCount = 0;

        dtVcopy(smoothPos, closestStart);
        path.push_back(float3(smoothPos[0], smoothPos[1], smoothPos[2]));

        for (int i = 0; i < pathCount && smoothCount < 256; i++)
        {
            float3 nextPoint;
            dtPolyRef polyRef = smoothPathPolys[i];
            dtStraightPath straightPath[256];
            int straightPathCount = 0;

            navQuery->findStraightPath(
                smoothPos, closestEnd, smoothPathPolys, pathCount, (float*)&straightPath, nullptr, nullptr,
                &straightPathCount, 256
            );

            for (int j = 0; j < straightPathCount; j++)
            {
                path.push_back(float3(straightPath[j].pos[0], straightPath[j].pos[1], straightPath[j].pos[2]));
            }
        }
    }
    return path;
}
