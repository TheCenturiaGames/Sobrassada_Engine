#pragma once
#include "Script.h"

class ExitGameScript : public Script
{
  public:
    ExitGameScript(GameObject* parent) : Script(parent) {}
    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;

    void OnClick();
};
