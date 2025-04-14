#pragma once
#include "Script.h"

class ToggleMenu : public Script
{
  public:
    ToggleMenu(GameObject* parent) : Script(parent) {}
    bool Init() override;
    void Update(float deltaTime) override;
};
