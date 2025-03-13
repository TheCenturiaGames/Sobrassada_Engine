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



//CharacterControllerComponent::CharacterControllerComponent(
//    UID uid, UID uidParent, UID uidRoot, const char* initName, const ComponentType characterControllertype,
//    const float4x4& parentGlobalTransform
//)
//    : Component(uid, uidParent, uidRoot, initName, type, parentGlobalTransform)
//{
//}



CharacterControllerComponent::CharacterControllerComponent(UID uid, UID uidParent)
    : Component(uid, uidParent, "Character Controller", COMPONENT_CHARACTER_CONTROLLER)
{
    speed = 0;
    maxLinearSpeed = 10;
    maxAngularSpeed = 120;
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
}

CharacterControllerComponent::~CharacterControllerComponent()
{
}

void CharacterControllerComponent::Update()
{
    if (!enabled) return;

    if (!App->GetSceneModule()->IsInPlayMode()) return;

    float deltaTime = App->GetSceneModule()->GetCurrentDeltaTime();

    if (deltaTime <= 0.0f) return;

    HandleInput(deltaTime);

}

void CharacterControllerComponent::Render()
{
}

void CharacterControllerComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::Separator();
        ImGui::Text("Character Controller");

        ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Max Linear Speed", &maxLinearSpeed, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Max Angular Speed", &maxAngularSpeed, 1.0f, 0.0f, 360.0f);
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

void CharacterControllerComponent::HandleInput(float deltaTime)
{
    const KeyState* keyboard = App->GetInputModule()->GetKeyboard();

    float3 direction(0.0f, 0.0f, 0.0f);

    if (keyboard[SDL_SCANCODE_W] == KEY_REPEAT) direction.z += 1.0f;
    if (keyboard[SDL_SCANCODE_S] == KEY_REPEAT) direction.z -= 1.0f;

    if (direction.LengthSq() > 0.0001f)
    {
        direction.Normalize();
        Move(direction, deltaTime);
    }
    //else
    //{
    //    //TODO: StateMachine IDLE
    //}

}


