#include "AIAgentComponent.h"

#include "Application.h"
#include "SceneModule.h"
#include "PathfinderModule.h"

#include <DetourCrowd.h>


AIAgentComponent::AIAgentComponent(UID uid, GameObject* parent, float3 position, float moveSpeed)
    : Component(uid, parent, "AI Agent Controller", COMPONENT_AIAGENT)
{
    agentPosition = position;
}

AIAgentComponent::AIAgentComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("AgentPositionX")) agentPosition.x = initialState["AgentPositionX"].GetFloat();
    if (initialState.HasMember("AgentPositionY")) agentPosition.y = initialState["AgentPositionY"].GetFloat();
    if (initialState.HasMember("AgentPositionZ")) agentPosition.z = initialState["AgentPositionZ"].GetFloat();

    if (initialState.HasMember("Speed")) speed = initialState["Speed"].GetFloat();
    if (initialState.HasMember("Radius")) radius = initialState["Radius"].GetFloat();
    if (initialState.HasMember("Height")) height = initialState["Height"].GetFloat();
    if (initialState.HasMember("AutoAdd")) autoAdd = initialState["AutoAdd"].GetBool();

    if (autoAdd) AddToCrowd();
}

AIAgentComponent::~AIAgentComponent()
{
}

void AIAgentComponent::Update(float deltaTime)
{
    if (!enabled) return;

    if (!App->GetSceneModule()->GetInPlayMode()) return;

    if (deltaTime <= 0.0f) return;

}

// OPTIONAL TODO save agents as a resource in the future.
void AIAgentComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("AgentPositionX", agentPosition.x, allocator);
    targetState.AddMember("AgentPositionY", agentPosition.y, allocator);
    targetState.AddMember("AgentPositionZ", agentPosition.z, allocator);
    targetState.AddMember("Speed", speed, allocator);
    targetState.AddMember("Radius", radius, allocator);
    targetState.AddMember("Height", height, allocator);
    targetState.AddMember("AutoAdd", autoAdd, allocator);
}

void AIAgentComponent::setPath(const float3& destination)
{
}

void AIAgentComponent::AddToCrowd()
{
    if (agentId != -1)
    {
        GLOG("Failed to load AI agent. Duplicate AI agent.");
        return;
    }
     
    agentId = App->GetPathfinderModule()->CreateAgent(agentPosition, radius, height, speed);

}

void AIAgentComponent::RecreateAgent()
{
    if (agentId != -1)
    {
        App->GetPathfinderModule()->RemoveAgent(agentId); // Step 1: Remove old agent
        agentId = -1;
    }

    AddToCrowd();
}
