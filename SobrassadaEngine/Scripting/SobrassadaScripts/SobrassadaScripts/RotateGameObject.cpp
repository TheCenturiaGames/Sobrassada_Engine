#include "pch.h"
#include "RotateGameObject.h"
#include "GameObject.h"
#include "Math/float4x4.h"

bool RotateGameObject::Init()
{
    GLOG("Initiating RotationGameObject");
    return true;
}

void RotateGameObject::Update(float deltaTime)
{
    float4x4 matrix = parent->GetGlobalTransform();
    //matrix = matrix.RotateX(0.01f);
}
