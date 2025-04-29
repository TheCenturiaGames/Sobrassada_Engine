#include "pch.h"

#include "CameraMovement.h"

#include "Application.h"
#include "CameraComponent.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "InputModule.h"
#include "Interpolation.h"
#include "Scene.h"
#include "SceneModule.h"

#include "Math/MathFunc.h"

CameraMovement::CameraMovement(GameObject* parent) : Script(parent)
{
    fields.push_back({"Target", InspectorField::FieldType::InputText, &targetName});
    fields.push_back({"Smoothness", InspectorField::FieldType::Float, &smoothVelocity, 0, 100});
    fields.push_back({"Offset Intensity", InspectorField::FieldType::Float, &mouseOffsetIntensity, 0, 1});
}

bool CameraMovement::Init()
{
    const GameObject* targetObj = AppEngine->GetSceneModule()->GetScene()->GetGameObjectByName(targetName);
    if (targetObj) target = targetObj;

    return true;
}

void CameraMovement::Update(float deltaTime)
{
    FollowTarget(deltaTime);
}

void CameraMovement::FollowTarget(float deltaTime)
{
    if (!target) return;

    const float2 mousePos           = AppEngine->GetInputModule()->GetMousePosition();

    const float4x4& parentTransform = parent->GetGlobalTransform();
    const float2 localPos = mousePos - float2(parentTransform.TranslatePart().x, parentTransform.TranslatePart().y);

    const float3 worldPos = AppEngine->GetSceneModule()->GetScene()->GetMainCamera()->ScreenPointToXZ(
        parent->GetGlobalTransform().TranslatePart().y
    );
    GLOG("Mouse position: %f, %f", mousePos.x, mousePos.y);
    GLOG("Local position: %f, %f", localPos.x, localPos.y);
    GLOG("World position: %f, %f, %f", worldPos.x, worldPos.y, worldPos.z);

    const float3 clampPos        = worldPos.Clamp(-20, 20); 
    GLOG("Clamped position: %f, %f, %f", clampPos.x, clampPos.y, clampPos.z);

    const float3 desiredPosition = target->GetGlobalTransform().TranslatePart() + clampPos * 0.5f  * mouseOffsetIntensity;
    const float3 currentPosition = parent->GetGlobalTransform().TranslatePart();

    const float3 finalPosition   = Lerp(currentPosition, desiredPosition, (smoothVelocity * deltaTime));

    parent->SetLocalPosition(finalPosition);
}