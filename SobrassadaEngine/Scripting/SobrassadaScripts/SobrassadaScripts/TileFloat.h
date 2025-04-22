#pragma once
#include "Script.h"

class TileFloat : public Script
{
  public:
    TileFloat(GameObject* parent);
    bool Init() override;
    void Update(float deltaTime) override;

  private:
    float speed           = 0.5f;
    float maxRiseDistance = 2.0f;
    float initialY        = 0.0f;
};