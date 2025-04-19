#pragma once

#include "Script.h"

class VSyncToggleScript : public Script
{
  public:
    VSyncToggleScript(GameObject* parent) : Script(parent) {}

    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;

    void OnClick();
};
