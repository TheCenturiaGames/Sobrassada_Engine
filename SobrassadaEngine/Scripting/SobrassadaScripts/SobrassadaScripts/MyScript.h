#pragma once
#include "Script.h"

class MyScript : public Script
{
  public:
    bool Init() override;
    void Update(float deltaTime) override;
};
