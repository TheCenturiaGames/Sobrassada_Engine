#pragma once
#include "Script.h"

class RotateGameObject : public Script
{
  public:
    RotateGameObject(GameObject* parent) : Script(parent) {}
    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;

  private:
    float speed = 0.5f;
};