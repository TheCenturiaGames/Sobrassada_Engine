#pragma once

#include "Math/float2.h"
#include "Math/float3.h"
#include "Script.h"

class RotateGameObject : public Script
{
  public:
    RotateGameObject(GameObject* parent) : Script(parent) {}
    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;

  private:
    float speed   = 0.5f;
    float2 prueba = {0.0f, 0.0f};
    float3 color  = {1.0f, 0.0f, 0.0f};
};