#include "pch.h"
#include "RotateGameObject.h"
#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "ImGui.h"
#include "Math/float4x4.h"

RotateGameObject::RotateGameObject(GameObject* parent) : Script(parent)
{
    // Using ImGui in the dll cause problems, so we need to call ImGui outside the dll
    fields.push_back({InspectorField::FieldType::Text, (void*)"Test"});
    fields.push_back({"Speed", InspectorField::FieldType::Float, &speed, 0.0f, 2.0f});
    fields.push_back({"Vec2 Test", InspectorField::FieldType::Vec2, &prueba});
    fields.push_back({"Color Test", InspectorField::FieldType::Color, &color});
}

bool RotateGameObject::Init()
{
    GLOG("Initiating RotationGameObject");
    return true;
}

void RotateGameObject::Update(float deltaTime)
{
    float4x4 newTransform = parent->GetLocalTransform();
    newTransform          = newTransform * float4x4::RotateX(speed * deltaTime);
    parent->SetLocalTransform(newTransform);
    parent->UpdateTransformForGOBranch();
    GLOG("%f", AppEngine->GetCameraModule()->GetCameraPosition().y);
}
