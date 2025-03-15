#include "CharacterControllerComponent.h"

#include "Application.h"
#include "SceneModule.h"
#include "GameObject.h"
#include "EditorUIModule.h"
#include "InputModule.h"

#include <Math/float4x4.h>
#include <Math/float3.h>
#include <cmath>
#include <algorithm>


CharacterControllerComponent::CharacterControllerComponent(UID uid, UID uidParent)
    : Component(uid, uidParent, "Character Controller", COMPONENT_CHARACTER_CONTROLLER)
{
    speed = 1;
    maxLinearSpeed = 10;
    maxAngularSpeed = 90/RAD_DEGREE_CONV;
    useRad          = true;
    targetDirection.Set(0.0f, 0.0f, 1.0f);
}

CharacterControllerComponent::CharacterControllerComponent(const rapidjson::Value& initialState) : Component(initialState)
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
        useRad = initialState["isRadians"].GetBool();
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
    targetState.AddMember("isRadians", useRad, allocator);
}

void CharacterControllerComponent::Update()
{
}

void CharacterControllerComponent::Render(float deltaTime)
{
    if (!enabled) return;

    if (!App->GetSceneModule()->GetInPlayMode()) return;

    if (deltaTime <= 0.0f) return;

    HandleInput(deltaTime);
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
        ImGui::DragFloat("Max Linear Speed", &maxLinearSpeed, 0.1f, 0.0f, 100.0f,"%.3f",ImGuiSliderFlags_AlwaysClamp);

        if (speed > maxLinearSpeed) speed = maxLinearSpeed;
        
        float dragStep = useRad ? 1.0f / RAD_DEGREE_CONV : 1.0f;
        float minVal   = 0.0f;
        float maxVal   = useRad ? 360.0f / RAD_DEGREE_CONV : 360.0f; 
        
        ImGui::DragFloat(
            "Max Angular Speed##maxAngSpeed", &maxAngularSpeed, dragStep, minVal, maxVal, "%.3f",
            ImGuiSliderFlags_AlwaysClamp
        );

        if (maxAngularSpeed > maxVal) maxAngularSpeed = maxVal;

        bool prevUseRad = useRad;

        ImGui::SameLine();
        ImGui::Checkbox("Radians##maxAngCheck", &useRad);

        if (useRad != prevUseRad)
        {
            if (useRad)
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

void CharacterControllerComponent::Move(const float3& direction, float deltaTime)
{
    float finalSpeed = std::min(speed, maxLinearSpeed);

    float3 movementOffset = direction * finalSpeed * deltaTime;

    GameObject* parentGO = GetParent();
    if (!parentGO) return;

    float4x4 localTr = parentGO->GetLocalTransform();
    float4x4 translationMatrix = float4x4::FromTRS(movementOffset, float4x4::identity.RotatePart(), float3::one);

    localTr = localTr * translationMatrix;

    parentGO->SetLocalTransform(localTr);
    parentGO->UpdateTransformForGOBranch();
}

void CharacterControllerComponent::Rotate(float rotationDir, float deltaTime)
{
    float angleDeg = 0.0f;
    
    if (useRad)
    {
        angleDeg = maxAngularSpeed * rotationDir * deltaTime;
    }
    else
    {
        angleDeg = (maxAngularSpeed * rotationDir * deltaTime) / RAD_DEGREE_CONV; 
    }

    float4x4 rotationMatrix = float4x4::FromEulerXYZ(0.0f, angleDeg, 0.0f);

    GameObject* parentGO    = GetParent();
    if (!parentGO) return;

    float4x4 localTr = parentGO->GetLocalTransform();
    localTr          = localTr * rotationMatrix;
    parentGO->SetLocalTransform(localTr);
    parentGO->UpdateTransformForGOBranch();

}

void CharacterControllerComponent::HandleInput(float deltaTime)
{
    const KeyState* keyboard = App->GetInputModule()->GetKeyboard();

    float3 direction(0.0f, 0.0f, 0.0f);

    if (keyboard[SDL_SCANCODE_W] == KEY_REPEAT) direction -= targetDirection;
    if (keyboard[SDL_SCANCODE_S] == KEY_REPEAT) direction += targetDirection;

    float rotationDir = 0.0f;

    if (keyboard[SDL_SCANCODE_A] == KEY_REPEAT) rotationDir += 1.0f;
    if (keyboard[SDL_SCANCODE_D] == KEY_REPEAT) rotationDir -= 1.0f;

    if (direction.LengthSq() > 0.0001f)
    {
        direction.Normalize();
        Move(direction, deltaTime);
    }

    if (fabs(rotationDir) > 0.0001f)
    {
        Rotate(rotationDir, deltaTime);
    }
    //else
    //{
    //    //TODO: StateMachine IDLE
    //}

}


