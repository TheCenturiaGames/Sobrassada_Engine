#include "pch.h"
#include "TileFloat.h"

#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "ImGui.h"
#include "Math/float4x4.h"



TileFloat::TileFloat(GameObject* parent) : Script(parent)
{
    // Using ImGui in the dll cause problems, so we need to call ImGui outside the dll
    fields.push_back({InspectorField::FieldType::Text, (void*)"Test"});
    fields.push_back({"Speed", InspectorField::FieldType::Float, &speed, 0.0f, 2.0f});

}

bool TileFloat::Init()
{
    initialY = parent->GetLocalTransform().TranslatePart().y;
    GLOG("Initiating TileFloatScript");
    return true;
}

void TileFloat::Update(float deltaTime)
{
    float currentY      = parent->GetLocalTransform().TranslatePart().y;
    float distanceRisen = currentY - initialY;

    if (distanceRisen < maxRiseDistance)
    {
        float riseStep    = speed * deltaTime;
        float clampedRise = (distanceRisen + riseStep > maxRiseDistance) ? (maxRiseDistance - distanceRisen) : riseStep;

        float4x4 newTransform = parent->GetLocalTransform();
        newTransform.TransformPos(0.0f, clampedRise, 0.0f);
        parent->SetLocalTransform(newTransform);
        parent->UpdateTransformForGOBranch();
    }
}