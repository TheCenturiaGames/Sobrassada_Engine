#include "pch.h"

#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "ImGui.h"
#include "Math/float4x4.h"
#include "RotateGameObject.h"

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

void RotateGameObject::Inspector()
{
    // Using ImGui in the dll cause problems, so we need to call ImGui outside the dll
    if (fields.empty())
    {
        fields.push_back({InspectorField::FieldType::Text, (void*)"Test"});
        fields.push_back({"Speed", InspectorField::FieldType::Float, &speed, 0.0f, 2.0f});
        fields.push_back({"Prueba vec2", InspectorField::FieldType::Vec2, &prueba});
        fields.push_back({"Color", InspectorField::FieldType::Color, &color});
    }

    AppEngine->GetEditorUIModule()->DrawScriptInspector(fields);
}
