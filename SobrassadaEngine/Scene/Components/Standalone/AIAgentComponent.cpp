#include "AIAgentComponent.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "EngineTimer.h"
#include "GameObject.h"
#include "PathfinderModule.h"
#include "ResourceNavmesh.h"
#include "SceneModule.h"
#include "Standalone/CharacterControllerComponent.h"

#include "DetourCrowd.h"

AIAgentComponent::AIAgentComponent(UID uid, GameObject* parent) : Component(uid, parent, "AI Agent", COMPONENT_AIAGENT)
{
    speed           = 3.5f;
    radius          = 0.6f;
    height          = 2.0f;
    maxAngularSpeed = 90 / RAD_DEGREE_CONV;
    isRadians       = true;

    RecreateAgent();
}

AIAgentComponent::AIAgentComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("Speed")) speed = initialState["Speed"].GetFloat();
    if (initialState.HasMember("Radius")) radius = initialState["Radius"].GetFloat();
    if (initialState.HasMember("Height")) height = initialState["Height"].GetFloat();
    if (initialState.HasMember("MaxAngularSpeed"))
    {
        maxAngularSpeed = initialState["MaxAngularSpeed"].GetFloat();
    }
    if (initialState.HasMember("isRadians"))
    {
        isRadians = initialState["isRadians"].GetBool();
    }

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

    dtCrowd* crowd = App->GetPathfinderModule()->GetCrowd();

    if (!crowd) return;

    if (agentId == -1 || agentId >= crowd->getAgentCount() || crowd->getAgent(agentId) == nullptr) RecreateAgent();

    if (agentId == -1) return;

    const dtCrowdAgent* ag = crowd->getAgent(agentId);

    if (!ag || !ag->active) return;

    float3 newPos;

    if (isPaused)
    {
        newPos               = frozenPosition;

        dtCrowdAgent* editAg = crowd->getEditableAgent(agentId);

        if (editAg)
        {
            editAg->npos[0] = frozenPosition.x;
            editAg->npos[1] = frozenPosition.y;
            editAg->npos[2] = frozenPosition.z;

            editAg->vel[0] = editAg->vel[1] = editAg->vel[2] = 0.f;
            editAg->nvel[0] = editAg->nvel[1] = editAg->nvel[2] = 0.f;
            editAg->dvel[0] = editAg->dvel[1] = editAg->dvel[2] = 0.f;
        }
    }
    else newPos = float3(ag->npos[0], ag->npos[1], ag->npos[2]);

    float4x4 transform = parent->GetLocalTransform();
    transform.SetTranslatePart(newPos);
    parent->SetLocalTransform(transform); // Change parent position
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

        float dragStep = isRadians ? 1.0f / RAD_DEGREE_CONV : 1.0f;
        float minVal   = 0.0f;
        float maxVal   = isRadians ? 360.0f / RAD_DEGREE_CONV : 360.0f;

        ImGui::DragFloat(
            "Max Angular Speed##maxAngSpeed", &maxAngularSpeed, dragStep, minVal, maxVal, "%.3f",
            ImGuiSliderFlags_AlwaysClamp
        );

        if (maxAngularSpeed > maxVal) maxAngularSpeed = maxVal;

        bool prevUseRad = isRadians;

        ImGui::SameLine();
        ImGui::Checkbox("Radians##maxAngCheck", &isRadians);

        if (isRadians != prevUseRad)
        {
            if (isRadians)
            {
                maxAngularSpeed /= RAD_DEGREE_CONV;
            }
            else
            {
                maxAngularSpeed *= RAD_DEGREE_CONV;
            }
        }
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
        maxAngularSpeed                      = otherAIAgent->maxAngularSpeed;

        isRadians                            = otherAIAgent->isRadians;
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
    targetState.AddMember("MaxAngularSpeed", maxAngularSpeed, allocator);
    targetState.AddMember("isRadians", isRadians, allocator);
}

// finds closest navmesh walkable triangle.
bool AIAgentComponent::SetPathNavigation(const math::float3& destination)
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


void AIAgentComponent::PauseMovement()
{
    if (isPaused || agentId == -1) return;

    dtCrowd* crowd   = App->GetPathfinderModule()->GetCrowd();
    dtCrowdAgent* ag = crowd ? crowd->getEditableAgent(agentId) : nullptr;

    if (!ag) return;

    restoredSpeed              = ag->params.maxSpeed;
    restoredAccel              = ag->params.maxAcceleration;
    restoreAngular             = maxAngularSpeed;

    ag->params.maxSpeed        = 0.0f;
    ag->params.maxAcceleration = 0.0f;
    speed                      = 0.0f;
    maxAngularSpeed            = 0.0f;

    crowd->resetMoveTarget(agentId);

    frozenPosition = parent->GetGlobalTransform().TranslatePart();

    isPaused       = true;
}

void AIAgentComponent::ResumeMovement()
{
    if (!isPaused || agentId == -1) return;

    dtCrowdAgent* ag = App->GetPathfinderModule()->GetCrowd()->getEditableAgent(agentId);
    if (!ag) return;

    ag->params.maxSpeed        = restoredSpeed;
    ag->params.maxAcceleration = restoredAccel;
    speed                      = restoredSpeed;
    maxAngularSpeed            = restoreAngular;

    isPaused                   = false;
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

void AIAgentComponent::LookAtMovement(const float3& targetPos, float deltaTime)
{
    float3 selfPos = parent->GetGlobalTransform().TranslatePart();
    float3 desired = targetPos - selfPos;
    desired.y      = 0.0f;

    if (desired.LengthSq() < 0.0001f) return;
    desired.Normalize();

    float3 forward = parent->GetGlobalTransform().WorldZ();
    forward.y      = 0.0f;
    forward.Normalize();

    float angle   = atan2(forward.Cross(desired).y, forward.Dot(desired));

    float maxStep = maxAngularSpeed * deltaTime;
    angle         = std::clamp(angle, -maxStep, maxStep);

    if (fabs(angle) < 0.0001f) return;

    float4x4 rotY      = float4x4::FromEulerXYZ(0.0f, angle, 0.0f);
    float4x4 newGlobal = parent->GetGlobalTransform() * rotY;

    float4x4 newlocal  = parent->GetParentGlobalTransform().Transposed() * newGlobal;

    parent->SetLocalTransform(newlocal);
    parent->UpdateTransformForGOBranch();
}
