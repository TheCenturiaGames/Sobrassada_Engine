#include "AIAgentComponent.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "PathfinderModule.h"
#include "ResourceNavmesh.h"
#include "SceneModule.h"
#include "Standalone/CharacterControllerComponent.h"

#include "DetourCrowd.h"

AIAgentComponent::AIAgentComponent(UID uid, GameObject* parent) : Component(uid, parent, "AI Agent", COMPONENT_AIAGENT)
{
    speed  = 3.5f;
    radius = 0.6f;
    height = 2.0f;

    RecreateAgent();
}

AIAgentComponent::AIAgentComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("Speed")) speed = initialState["Speed"].GetFloat();
    if (initialState.HasMember("Radius")) radius = initialState["Radius"].GetFloat();
    if (initialState.HasMember("Height")) height = initialState["Height"].GetFloat();

    RecreateAgent();
}

AIAgentComponent::~AIAgentComponent()
{
    if (agentId != -1)
    {
        App->GetPathfinderModule()->RemoveAgent(agentId);
        agentId = -1;
    }
}
// Updates agent position evey frame
void AIAgentComponent::Update(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;

    if (!App->GetSceneModule()->GetInPlayMode()) return;

    if (agentId == -1) return;

    const dtCrowdAgent* ag = App->GetPathfinderModule()->GetCrowd()->getAgent(agentId);
    if (ag && ag->active)
    {
        float3 newPos(ag->npos[0], ag->npos[1], ag->npos[2]);
        float4x4 transform = parent->GetLocalTransform();
        transform.SetTranslatePart(newPos);
        parent->SetLocalTransform(transform); // Change parent position
    }
}

void AIAgentComponent::Render(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;
}

void AIAgentComponent::RenderDebug(float deltaTime)
{

}

void AIAgentComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::SeparatorText("AIAgent Component");

        if (ImGui::DragFloat("Speed", &speed, 0.1f, 0.1f, 200.f, "%.2f")) RecreateAgent();
        if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.1f, 200.f, "%.2f")) RecreateAgent();
        if (ImGui::DragFloat("Height", &height, 0.1f, 0.1f, 200.f, "%.2f")) RecreateAgent();
    }
}

void AIAgentComponent::Clone(const Component* other)
{

    if (other->GetType() == ComponentType::COMPONENT_AIAGENT)
    {
        const AIAgentComponent* otherAIAgent = static_cast<const AIAgentComponent*>(other);
        speed                                = otherAIAgent->speed;
        radius                               = otherAIAgent->radius;
        height                               = otherAIAgent->height;
        agentId                              = -1;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

// OPTIONAL TODO save agents as a resource in the future.
void AIAgentComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("Speed", speed, allocator);
    targetState.AddMember("Radius", radius, allocator);
    targetState.AddMember("Height", height, allocator);
}

// finds closest navmesh walkable triangle.
bool AIAgentComponent::SetPathNavigation(const math::float3& destination) const
{
    if (agentId == -1) return false;

    PathfinderModule* pathfinder = App->GetPathfinderModule();
    dtNavMeshQuery* navQuery     = pathfinder->GetNavQuery();
    if (!navQuery) return false;

    // Prepare for finding the nearest poly
    dtQueryFilter filter;
    float extents[3] = {2.0f, 4.0f, 2.0f}; // bounding box for the search area
    float nearestPoint[3];
    dtPolyRef targetRef;

    dtStatus status = navQuery->findNearestPoly(destination.ptr(), extents, &filter, &targetRef, nearestPoint);
    if (dtStatusFailed(status) || targetRef == 0)
    {
        GLOG("Failed to find valid target poly for movement.");
        return false;
    }

    // Request move to destination
    bool result = pathfinder->GetCrowd()->requestMoveTarget(agentId, targetRef, destination.ptr());
    if (!result)
    {
        GLOG("Crowd agent failed to request movement.");
        return false;
    }
    return true;
}

void AIAgentComponent::AddToCrowd()
{
    if (agentId != -1)
    {
        GLOG("Failed to load AI agent. Duplicate AI agent.");
        return;
    }

    agentId = App->GetPathfinderModule()->CreateAgent(parent->GetPosition(), radius, height, speed);

    if (agentId != -1)
    {
        App->GetPathfinderModule()->AddAIAgentComponent(agentId, this);
    }
}

void AIAgentComponent::RecreateAgent()
{
    if (agentId != -1)
    {
        App->GetPathfinderModule()->RemoveAgent(agentId);
        agentId = -1;
    }

    AddToCrowd();
}
