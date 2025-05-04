#include "pch.h"

#include "TileFloatScript.h"

#include "Application.h"
#include "CameraModule.h"
#include "CuChulainn.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "ImGui.h"
#include "LibraryModule.h"
#include "Math/float4x4.h"
#include "Standalone/CharacterControllerComponent.h"

void TileFloatScript::Inspector()
{
}

void TileFloatScript::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator)
{
    targetState.AddMember("Speed", speed, allocator);
}

void TileFloatScript::Load(const rapidjson::Value& initialState)
{
    if (initialState.HasMember("Speed") && initialState["Speed"].IsFloat())
    {
        speed = initialState["Speed"].GetFloat();
    }
}

bool TileFloatScript::Init()
{
    character->GetLastPosition();
    if (!character)
    {
        GLOG("CharacterController component not found for CuChulainn");
        return false;
    }
    initialY = parent->GetLocalTransform().TranslatePart().y;
    GLOG("Initiating TileFloatScript");
    return true;
}

void TileFloatScript::Update(float deltaTime)
{
    
    if (character == nullptr) return;
    float currentY = parent->GetLocalTransform().TranslatePart().y;
    if (currentY == maxRiseDistance) return;

    float distance = character->GetLastPosition().Distance(parent->GetPosition());
    if (distance <= 2.0f)
    {
        float currentY      = parent->GetLocalTransform().TranslatePart().y;
        float distanceRisen = currentY - initialY;

        if (distanceRisen < maxRiseDistance)
        {
            float riseStep = speed * deltaTime;
            float clampedRise =
                (distanceRisen + riseStep > maxRiseDistance) ? (maxRiseDistance - distanceRisen) : riseStep;

            float4x4 newTransform = parent->GetLocalTransform();
            float3 currentPos     = parent->GetLocalTransform().TranslatePart();
            newTransform.SetTranslatePart(currentPos.x, currentPos.y + clampedRise, currentPos.z);
            parent->SetLocalTransform(newTransform);
            parent->UpdateTransformForGOBranch();
        }
    }
}