#include "pch.h"
#include "TileFloatScript.h"

#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "ImGui.h"
#include "Math/float4x4.h"


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
    initialY = parent->GetLocalTransform().TranslatePart().y;
    GLOG("Initiating TileFloatScript");
    return true;
}

void TileFloatScript::Update(float deltaTime)
{
    float currentY      = parent->GetLocalTransform().TranslatePart().y;
    float distanceRisen = currentY - initialY;

    if (distanceRisen < maxRiseDistance)
    {
        float riseStep    = speed * deltaTime;
        float clampedRise = (distanceRisen + riseStep > maxRiseDistance) ? (maxRiseDistance - distanceRisen) : riseStep;

        float4x4 newTransform = parent->GetLocalTransform();
        float3 currentPos     = parent->GetLocalTransform().TranslatePart();
        newTransform.SetTranslatePart(currentPos.x, currentPos.y + clampedRise, currentPos.z);
        parent->SetLocalTransform(newTransform);
        parent->UpdateTransformForGOBranch();
    }
}