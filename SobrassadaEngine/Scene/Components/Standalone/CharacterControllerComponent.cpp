#include "CharacterControllerComponent.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "InputModule.h"
#include "SceneModule.h"
#include "DetourNavMeshQuery.h"
#include "ResourcesModule.h"
#include "ResourceNavMesh.h"

#include "Math/float3.h"
#include "Math/float4x4.h"
#include <algorithm>
#include <cmath>

CharacterControllerComponent::CharacterControllerComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Character Controller", COMPONENT_CHARACTER_CONTROLLER)
{
    speed           = 1;
    maxLinearSpeed  = 10;
    maxAngularSpeed = 90 / RAD_DEGREE_CONV;
    isRadians       = true;
    targetDirection.Set(0.0f, 0.0f, 1.0f);
}

CharacterControllerComponent::CharacterControllerComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("TargetDirectionX"))
    {
        targetDirection.x = initialState["TargetDirectionX"].GetFloat();
    }
    if (initialState.HasMember("TargetDirectionY"))
    {
        targetDirection.y = initialState["TargetDirectionY"].GetFloat();
    }
    if (initialState.HasMember("TargetDirectionZ"))
    {
        targetDirection.z = initialState["TargetDirectionZ"].GetFloat();
    }
    if (initialState.HasMember("Speed"))
    {
        speed = initialState["Speed"].GetFloat();
    }
    if (initialState.HasMember("MaxLinearSpeed"))
    {
        maxLinearSpeed = initialState["MaxLinearSpeed"].GetFloat();
    }
    if (initialState.HasMember("MaxAngularSpeed"))
    {
        maxAngularSpeed = initialState["MaxAngularSpeed"].GetFloat();
    }
    if (initialState.HasMember("isRadians"))
    {
        isRadians = initialState["isRadians"].GetBool();
    }
}

CharacterControllerComponent::~CharacterControllerComponent()
{
}

void CharacterControllerComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator)
    const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("TargetDirectionX", targetDirection.x, allocator);
    targetState.AddMember("TargetDirectionY", targetDirection.y, allocator);
    targetState.AddMember("TargetDirectionZ", targetDirection.z, allocator);
    targetState.AddMember("Speed", speed, allocator);
    targetState.AddMember("MaxLinearSpeed", maxLinearSpeed, allocator);
    targetState.AddMember("MaxAngularSpeed", maxAngularSpeed, allocator);
    targetState.AddMember("isRadians", isRadians, allocator);
}

void CharacterControllerComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_CHARACTER_CONTROLLER)
    {
        const CharacterControllerComponent* otherCharacter = static_cast<const CharacterControllerComponent*>(other);
        enabled                                            = otherCharacter->enabled;

        speed                                              = otherCharacter->speed;
        maxLinearSpeed                                     = otherCharacter->maxLinearSpeed;
        maxAngularSpeed                                    = otherCharacter->maxAngularSpeed;

        isRadians                                          = otherCharacter->isRadians;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void CharacterControllerComponent::Update(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;
    if (!enabled) return;

    if (!App->GetSceneModule()->GetInPlayMode()) return;

    if (deltaTime <= 0.0f) return;

    ResourceNavMesh* navRes = App->GetResourcesModule()->GetNavMesh();
    if (!navRes) return;

    dtNavMesh* dtNav         = navRes->GetDetourNavMesh();
    dtNavMeshQuery* tmpQuery = navRes->GetDetourNavMeshQuery();
    if (!tmpQuery || !dtNav) return;

    if (!navMeshQuery)
    {
        navMeshQuery = tmpQuery;

        if (currentPolyRef == 0)
        {                    
            float3 startPos = parent->GetGlobalTransform().TranslatePart();

            dtQueryFilter filter;
            filter.setIncludeFlags(SAMPLE_POLYFLAGS_WALK);
            filter.setExcludeFlags(0);

            float extents[3] = {2.0f, 4.0f, 2.0f};
            float nearestPoint[3];
            dtPolyRef targetRef = 0;

            dtStatus status =
                navMeshQuery->findNearestPoly(startPos.ptr(), extents, &filter, &targetRef, nearestPoint);

            if (dtStatusFailed(status) || targetRef == 0)
            {
                GLOG("Failed to find valid target poly for movement.");
                return;
            }

            currentPolyRef = targetRef;
        }
    }
    
    if (!navMeshQuery || currentPolyRef == 0) return;
    
    verticalSpeed += gravity * deltaTime; 
    verticalSpeed      = std::max(verticalSpeed, maxFallSpeed); // Clamp fall speed

    float4x4 globalTr = parent->GetGlobalTransform();
    float3 currentPos = globalTr.TranslatePart();

    currentPos.y      += (verticalSpeed * deltaTime);

    AdjustHeightToNavMesh(currentPos);

    globalTr.SetTranslatePart(currentPos);
    float4x4 finalLocal = parent->GetParentGlobalTransform().Transposed() * globalTr;

    parent->SetLocalTransform(finalLocal);
    parent->UpdateTransformForGOBranch();

    HandleInput(deltaTime);

}

void CharacterControllerComponent::Render(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;
}

void CharacterControllerComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        float availableWidth = ImGui::GetContentRegionAvail().x;

        ImGui::Separator();
        ImGui::Text("Character Controller");

        ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, maxLinearSpeed, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::DragFloat("Max Linear Speed", &maxLinearSpeed, 0.1f, 0.0f, 100.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

        if (speed > maxLinearSpeed) speed = maxLinearSpeed;

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

void CharacterControllerComponent::AdjustHeightToNavMesh(float3& currentPos)
{
    if (!navMeshQuery || currentPolyRef == 0) return;

    dtQueryFilter filter;
    filter.setIncludeFlags(SAMPLE_POLYFLAGS_WALK);
    filter.setExcludeFlags(0);

    float halfExt[3] = {0.5f, 1.0f, 0.5f};
    float nearest[3];
    dtPolyRef newRef = 0;

    dtStatus st      = navMeshQuery->findNearestPoly(currentPos.ptr(), halfExt, &filter, &newRef, nearest);
    if (!dtStatusSucceed(st) || newRef == 0) return;

    bool posOverPoly = false;
    float closest[3];
    dtStatus st2 = navMeshQuery->closestPointOnPoly(newRef, currentPos.ptr(), closest, &posOverPoly);
    if (!dtStatusSucceed(st2) || !posOverPoly) return;

    currentPolyRef   = newRef; 

    float polyHeight = 0.0f;
    dtStatus stH     = navMeshQuery->getPolyHeight(newRef, closest, &polyHeight);
    if (dtStatusSucceed(stH))
    {
        float distToFloor         = polyHeight - currentPos.y;
        const float maxStepHeight = 0.5f;
        if (distToFloor >= 0.0f && distToFloor <= maxStepHeight)
        {
            currentPos.y  = polyHeight;
            verticalSpeed = 0.0f;
        }
    }
}

void CharacterControllerComponent::Move(const float3& direction, float deltaTime)
{
    if (!navMeshQuery || currentPolyRef == 0) return;

    float4x4 globalTr  = parent->GetGlobalTransform();
    float3 currentPos  = globalTr.TranslatePart();

    float finalSpeed   = std::min(speed, maxLinearSpeed);
    float3 offsetXZ    = direction * finalSpeed * deltaTime;

    float3 desiredPos  = currentPos;
    desiredPos.x      += offsetXZ.x;
    desiredPos.z      += offsetXZ.z;

    dtQueryFilter filter;
    filter.setIncludeFlags(SAMPLE_POLYFLAGS_WALK);
    filter.setExcludeFlags(0);

    float halfExt[3] = {0.5f, 1.0f, 0.5f};
    float nearest[3] = {};
    dtPolyRef newRef = 0;

    dtStatus status  = navMeshQuery->findNearestPoly(desiredPos.ptr(), halfExt, &filter, &newRef, nearest);

    if (!dtStatusSucceed(status) || newRef == 0) return;

    float closest[3] = {};
    bool posOverPoly = false;

    status           = navMeshQuery->closestPointOnPoly(newRef, desiredPos.ptr(), closest, &posOverPoly);

    if (!dtStatusSucceed(status) || !posOverPoly) return;

    currentPolyRef = newRef;

    desiredPos.x   = nearest[0];
    desiredPos.z   = nearest[2];

    globalTr.SetTranslatePart(desiredPos);
    float4x4 finalLocal = parent->GetParentGlobalTransform().Transposed() * globalTr;

    parent->SetLocalTransform(finalLocal);
    parent->UpdateTransformForGOBranch();
}

void CharacterControllerComponent::Rotate(float rotationDirection, float deltaTime)
{
    float angleDeg = 0.0f;

    if (isRadians)
    {
        angleDeg = maxAngularSpeed * rotationDirection * deltaTime;
    }
    else
    {
        angleDeg = (maxAngularSpeed * rotationDirection * deltaTime) / RAD_DEGREE_CONV;
    }

    float4x4 rotationMatrix = float4x4::FromEulerXYZ(0.0f, angleDeg, 0.0f);

    float4x4 localTr        = parent->GetLocalTransform();

    localTr                 = localTr * rotationMatrix;

    parent->SetLocalTransform(localTr);
    parent->UpdateTransformForGOBranch();
}

void CharacterControllerComponent::HandleInput(float deltaTime)
{
    const KeyState* keyboard = App->GetInputModule()->GetKeyboard();

    float3 direction(0.0f, 0.0f, 0.0f);

    if (keyboard[SDL_SCANCODE_W] == KEY_REPEAT) direction.z -= 1.0f;
    if (keyboard[SDL_SCANCODE_S] == KEY_REPEAT) direction.z += 1.0f;
    if (keyboard[SDL_SCANCODE_A] == KEY_REPEAT) direction.x -= 1.0f;
    if (keyboard[SDL_SCANCODE_D] == KEY_REPEAT) direction.x += 1.0f;

    float rotationDir = 0.0f;

    if (keyboard[SDL_SCANCODE_Q] == KEY_REPEAT) rotationDir += 1.0f;
    if (keyboard[SDL_SCANCODE_E] == KEY_REPEAT) rotationDir -= 1.0f;

    if (direction.LengthSq() > 0.0001f)
    {
        direction.Normalize();
        targetDirection = direction;

        Move(direction, deltaTime);
    }
    
    if (fabs(rotationDir) > 0.0001f)
    {
        Rotate(rotationDir, deltaTime);
    }
    // else
    //{
    //     //TODO: StateMachine IDLE
    // }
}
