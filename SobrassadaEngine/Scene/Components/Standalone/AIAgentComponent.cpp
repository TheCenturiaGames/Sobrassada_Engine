#include "AIAgentComponent.h"
#include "Application.h"
#include "DetourCrowd.h"
#include "SceneModule.h"

AIAgentComponent::AIAgentComponent(UID uid, GameObject* parent, float3 position, float moveSpeed)
    : Component(uid, parent, "AI Agent Controller", COMPONENT_AIAGENT)
{
    agentPosition = position;
}

AIAgentComponent::AIAgentComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("AgentPositionX"))
    {
        agentPosition.x = initialState["AgentPositionX"].GetFloat();
    }
    if (initialState.HasMember("AgentPositionY"))
    {
        agentPosition.y = initialState["AgentPositionY"].GetFloat();
    }
    if (initialState.HasMember("AgentPositionZ"))
    {
        agentPosition.z = initialState["AgentPositionZ"].GetFloat();
    }
    if (initialState.HasMember("Speed"))
    {
        speed = initialState["Speed"].GetFloat();
    }
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

void AIAgentComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("AgentPositionX", agentPosition.x, allocator);
    targetState.AddMember("AgentPositionY", agentPosition.y, allocator);
    targetState.AddMember("AgentPositionZ", agentPosition.z, allocator);
    targetState.AddMember("Speed", speed, allocator);

}

void AIAgentComponent::setPath(const float3& destination)
{
}
