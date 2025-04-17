#pragma once
#include "Script.h"

class ButtonScript : public Script
{
  public:
    ButtonScript(GameObject* parent) : Script(parent) {}
    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override {};
    void TestDispatcher();
};
