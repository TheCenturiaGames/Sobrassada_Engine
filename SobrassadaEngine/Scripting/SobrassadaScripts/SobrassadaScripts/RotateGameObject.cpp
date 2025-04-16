#include "pch.h"
#include "RotateGameObject.h"
#include "GameObject.h"
#include "Math/float4x4.h"
#include "CameraModule.h"
#include "Application.h"

bool RotateGameObject::Init()
{
    GLOG("Initiating RotationGameObject");
    return true;
}

void RotateGameObject::Update(float deltaTime)
{
    float4x4 newTransform = parent->GetLocalTransform();
    newTransform = newTransform * float4x4::RotateX(0.5f * deltaTime);
    parent->SetLocalTransform(newTransform);
    parent->UpdateTransformForGOBranch();
    GLOG("%f", AppEngine->GetCameraModule()->GetCameraPosition().y);
}
