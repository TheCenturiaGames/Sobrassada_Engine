#pragma once
#include "Script.h"
#include "Math/float4x4.h"

class CharacterControllerComponent;
class CameraComponent;

class GodMode : public Script
{
  public:
    GodMode(GameObject* parent) : Script(parent) {}
    bool Init() override;
    void Update(float deltaTime) override;

  private:
    CharacterControllerComponent* characterController;
    CameraComponent* godCamera;
    float4x4 originalTransform;
    bool freeCamera = false;
};