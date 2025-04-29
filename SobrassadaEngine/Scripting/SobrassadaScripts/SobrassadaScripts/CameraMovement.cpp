#include "pch.h"

#include "CameraMovement.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "Scene.h"
#include "SceneModule.h"
#include "Interpolation.h"

CameraMovement::CameraMovement(GameObject* parent) : Script(parent)
{
   fields.push_back({"Target", InspectorField::FieldType::InputText, &targetName});
   fields.push_back({"Smoothness", InspectorField::FieldType::Float, &smoothness, 0, 100});
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

    // TODO: Smoothing

    const float3 desiredPosition = target->GetGlobalTransform().TranslatePart();
    const float3 currentPosition = parent->GetGlobalTransform().TranslatePart();

    const float3 finalPosition    = Lerp(currentPosition, desiredPosition, (smoothness * deltaTime));

    parent->SetLocalPosition(finalPosition);
}