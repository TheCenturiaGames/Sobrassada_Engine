#pragma once
#include "Script.h"

class TileFloat : public Script
{

  public:
    TileFloat(GameObject* parent) : Script(parent) {}
    bool Init() override;
    bool Update();

  private:
    float speed = 0.5f;
};