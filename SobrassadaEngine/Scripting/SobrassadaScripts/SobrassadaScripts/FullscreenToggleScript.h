#pragma once
#include "Script.h"

class FullscreenToggleScript : public Script
{
  public:
    FullscreenToggleScript(GameObject* parent) : Script(parent) {}
    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;

    void OnClick();
};