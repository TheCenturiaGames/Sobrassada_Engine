#include "AIAgentComponent.h"

#include "Application.h"
#include "PathfinderModule.h"
#include "SceneModule.h"
#include "EditorUIModule.h"


#include <DetourCrowd.h>

AIAgentComponent::AIAgentComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "AI Agent Controller", COMPONENT_AIAGENT)
{
    float4x4 transformMatrix = parent->GetGlobalTransform();
    agentPosition            = float3(transformMatrix[0][3], transformMatrix[1][3], transformMatrix[2][3]);
    speed                    = 3.5f;
    radius                   = 0.6f;
    height                   = 2.0f;
    agentId                  = -1;
    autoAdd                  = false;
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

void AIAgentComponent::Render(float deltaTime)
{
}

void AIAgentComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::DragFloat("Speed", &speed, 0.1f, 0.1f, 200.f, "%.2f");
        ImGui::DragFloat("Radius", &radius, 0.1f, 0.1f, 200.f, "%.2f");
        ImGui::DragFloat("Height", &height, 0.1f, 0.1f, 200.f, "%.2f");
        ImGui::Checkbox("AutoAdd", &autoAdd);
        
        if (ImGui::Button("Create Agent")) RecreateAgent();
    }
}

void AIAgentComponent::Clone(const Component* other)
{

    if (other->GetType() == ComponentType::COMPONENT_AIAGENT)
    {
        const AIAgentComponent* otherAIAgent = static_cast<const AIAgentComponent*>(other);
        agentPosition                                       = otherAIAgent->agentPosition;
        speed                                              = otherAIAgent->speed;
        radius                                             = otherAIAgent->radius;
        height                                             = otherAIAgent->height;
        agentId                                            = -1;
        autoAdd                                            = false;
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
