#pragma once

#include "Math/float4x4.h"
#include "Script.h"

class CharacterControllerComponent;
class CameraComponent;

class GodMode : public Script
{
  public:
    GodMode(GameObject* parent) : Script(parent) {}
    bool Init() override;
    void Update(float deltaTime) override;

  private:
    CharacterControllerComponent* characterController = nullptr;
    CameraComponent* godCamera                        = nullptr;
    float4x4 originalTransform;
    bool freeCamera = false;
};