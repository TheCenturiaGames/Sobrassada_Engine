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

    float4x4 transform = parent->GetLocalTransform();
    float3 pos         = transform.TranslatePart();
    pos.y              = 3.0f;
    transform.SetTranslatePart(pos);
    parent->SetLocalTransform(transform);
    parent->UpdateTransformForGOBranch();


    GLOG("Initiating TileFloatScript");
    return true;
}

void TileFloatScript::Update(float deltaTime)
{
    if (!character) return;

    const float distance = character->GetLastPosition().Distance(parent->GetPosition());
    if (distance > 10.0f) return;

    float3 currentPos   = parent->GetLocalTransform().TranslatePart();

    if (currentPos.y >= initialY) return;

    float riseStep         = speed * deltaTime * 5;
    float clampedRise      = min(riseStep, currentPos.y);

    currentPos.y          += clampedRise;

    float4x4 newTransform  = parent->GetLocalTransform();
    newTransform.SetTranslatePart(currentPos);
    parent->SetLocalTransform(newTransform);
    parent->UpdateTransformForGOBranch();
}