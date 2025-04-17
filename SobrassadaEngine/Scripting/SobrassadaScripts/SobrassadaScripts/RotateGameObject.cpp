#include "pch.h"
#include "RotateGameObject.h"
#include "GameObject.h"
#include "Math/float4x4.h"
#include "CameraModule.h"
#include "Application.h"
#include "ImGui.h"
#include "EditorUIModule.h"

bool RotateGameObject::Init()
{
    GLOG("Initiating RotationGameObject");
    return true;
}

void RotateGameObject::Update(float deltaTime)
{
    float4x4 newTransform = parent->GetLocalTransform();
    newTransform = newTransform * float4x4::RotateX(speed * deltaTime);
    parent->SetLocalTransform(newTransform);
    parent->UpdateTransformForGOBranch();
    GLOG("%f", AppEngine->GetCameraModule()->GetCameraPosition().y);
}

void RotateGameObject::Inspector()
{   
    // Probably I will change this
    /*
    AppEngine->GetEditorUIModule()->DrawScriptInspector([this]() {
        ImGui::SetCurrentContext(AppEngine->GetEditorUIModule()->GetImGuiContext());
        ImGui::Text("Hello");
        ImGui::SliderFloat("Speed", &speed, 0.0f, 1.0f);
    });
    */
}
