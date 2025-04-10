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
    if (!enabled) return;

    if (!App->GetSceneModule()->GetInPlayMode()) return;

    if (deltaTime <= 0.0f) return;

    if (!navMeshQuery)
    {
        ResourceNavMesh* navRes = App->GetResourcesModule()->GetNavMesh();

        if (navRes)
        {
            dtNavMeshQuery* tmpQuery = navRes->GetDetourNavMeshQuery();

            if (tmpQuery)
            {
                //TODO: Make the rest of the process
            }
        }
    }

    HandleInput(deltaTime);
}

void CharacterControllerComponent::Render(float deltaTime)
{
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

void CharacterControllerComponent::Move(const float3& direction, float deltaTime) const
{
    float finalSpeed           = std::min(speed, maxLinearSpeed);

    float3 movementOffset      = direction * finalSpeed * deltaTime;

    float4x4 localTr           = parent->GetLocalTransform();
    float4x4 translationMatrix = float4x4::FromTRS(movementOffset, float4x4::identity.RotatePart(), float3::one);

    localTr                    = localTr * translationMatrix;

    parent->SetLocalTransform(localTr);
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

        // TODO: Handle rotation of gameObject when changing directions (target direction will be used in this part)
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
