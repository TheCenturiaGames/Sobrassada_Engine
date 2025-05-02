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
    ImGui::SetCurrentContext(AppEngine->GetEditorUIModule()->GetImGuiContext());
    AppEngine->GetEditorUIModule()->DrawScriptInspector(
        [this]()
        {
            ImGui::InputFloat("Speed", &speed, 0.1f, 1.0f); 

            ImGui::InputFloat("Max Rise Distance", &maxRiseDistance, 0.1f, 1.0f);

            ImGui::Text("Initial Y: %.2f", initialY);
        }
    );
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
        newTransform.TransformPos(0.0f, clampedRise, 0.0f);
        parent->SetLocalTransform(newTransform);
        parent->UpdateTransformForGOBranch();
    }
}