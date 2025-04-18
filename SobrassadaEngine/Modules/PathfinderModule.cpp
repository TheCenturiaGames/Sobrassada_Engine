#include "PathfinderModule.h"

#include "Application.h"
#include "CameraModule.h"
#include "Components/Standalone/AIAgentComponent.h"
#include "LibraryModule.h"
#include "ResourceNavmesh.h"
#include "ResourcesModule.h"
#include "SceneModule.h"

#include "Geometry/Plane.h"
#include "ImGui.h"

#include "DetourCrowd.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

PathfinderModule::PathfinderModule()
{
}

bool PathfinderModule ::Init()
{
    if (!crowd) crowd = dtAllocCrowd();
    return true;
}

PathfinderModule::~PathfinderModule()
{
    dtFreeCrowd(crowd);
    // dtFreeNavMeshQuery(navQuery); TODO when saving navmesh, re-enable this
    if (navmesh) navmesh = nullptr;
}

update_status PathfinderModule::Update(float deltaTime)
{
    if (!App->GetSceneModule()->GetInPlayMode()) return UPDATE_CONTINUE;

    if (crowd->getAgentCount() > 0)
    {
        crowd->update(deltaTime, nullptr);
    }

    return UPDATE_CONTINUE;
}

// All ai agent components will call this to add themselves to crowd
int PathfinderModule::CreateAgent(const float3& position, const float radius, const float height, const float speed)
{

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

    return crowd->addAgent(position.ptr(), &params);
}

void PathfinderModule::RemoveAgent(int agentId)
{

    if (agentId < 0 || agentId >= crowd->getAgentCount()) return;

    const dtCrowdAgent* agent = crowd->getAgent(agentId);
    if (!agent || !agent->active) return;

    crowd->removeAgent(agentId);
    RemoveAIAgentComponent(agentId);
}

void PathfinderModule::InitQuerySystem()
{

    navmesh  = App->GetResourcesModule()->GetNavMesh();
    navQuery = navmesh->GetDetourNavMeshQuery();

    if (navmesh != nullptr)
    {
        crowd->init(maxAgents, maxAgentRadius, navmesh->GetDetourNavMesh());
    }
    else
    {
        GLOG("Failed to initialize Pathfinder crowd system.");
    }
}

// Currently called by clicking in the game, but any float3 will move the agents there. ONLY WORKS IN PLAY MODE
void PathfinderModule::HandleClickNavigation()
{
    if (!clickNavigationEnabled || !App->GetSceneModule()->GetInPlayMode()) return; // from UI

    LineSegment ray = App->GetCameraModule()->CastCameraRay();

    float3 hitPoint;

    RaycastToGround(ray, hitPoint);

    for (auto& pair : agentComponentMap)
    {
        AIAgentComponent* comp = pair.second;
        if (comp != nullptr) comp->setPath(hitPoint);
    }
}

bool PathfinderModule::RaycastToGround(const LineSegment& ray, float3& outHitPoint)
{
    Plane groundPlane(float3::unitY, 0.0f); // Ground at Y = 0
    float distance = 0.0f;

    if (ray.Intersects(groundPlane, &distance))
    {
        outHitPoint = ray.GetPoint(distance);
        return true;
    }

    return false;
}

void PathfinderModule::RenderCrowdEditor()
{
    ImGui::Checkbox("Navigation Enabled", &clickNavigationEnabled);
}

void PathfinderModule::AddAIAgentComponent(int agentId, AIAgentComponent* comp)
{
    agentComponentMap[agentId] = comp;
}

void PathfinderModule::RemoveAIAgentComponent(int agentId)
{
    agentComponentMap.erase(agentId);
}
AIAgentComponent* PathfinderModule::GetComponentFromAgentId(int agentId)
{
    auto it = agentComponentMap.find(agentId);
    return (it != agentComponentMap.end()) ? it->second : nullptr;
}
