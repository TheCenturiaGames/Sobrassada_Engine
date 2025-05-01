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
    fields.push_back({"Smoothness Velocity", InspectorField::FieldType::Float, &smoothnessVelocity, 0, 100});
    fields.push_back({"Enable Mouse Offset", InspectorField::FieldType::Bool, &mouseOffsetEnabled});
    fields.push_back({"Mouse Offset Intensity", InspectorField::FieldType::Float, &mouseOffsetIntensity, 0, 1});
    fields.push_back({"Look Ahead Intensity", InspectorField::FieldType::Float, &lookAheadIntensity, 0, 10});
    fields.push_back({"Look Ahead Smoothness", InspectorField::FieldType::Float, &lookAheadSmoothness, 0, 100});
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

    float3 desiredPosition       = target->GetGlobalTransform().TranslatePart();
    const float3 currentPosition = parent->GetGlobalTransform().TranslatePart();

    if (mouseOffsetEnabled)
    {
        currentLookAhead           = 0;
        const float3 mouseWorldPos = AppEngine->GetSceneModule()->GetScene()->GetMainCamera()->ScreenPointToXZ(
            parent->GetGlobalTransform().TranslatePart().y
        );
        desiredPosition += mouseWorldPos * 0.5f * mouseOffsetIntensity;
    }
    else if (lookAheadIntensity > 0)
    {
        // TODO: Get player speed and increase currentLookAhead only if its moving
        currentLookAhead             = Lerp(currentLookAhead, lookAheadIntensity, lookAheadSmoothness * deltaTime);
        const float3 targetRotation  = target->GetGlobalTransform().Col3(2);
        const float3 targetDir       = float3(targetRotation.x, 0, targetRotation.z).Normalized();
        desiredPosition             += targetDir * currentLookAhead;
    }
    finalPosition = Lerp(currentPosition, desiredPosition, smoothnessVelocity * deltaTime);

    parent->SetLocalPosition(finalPosition);
}