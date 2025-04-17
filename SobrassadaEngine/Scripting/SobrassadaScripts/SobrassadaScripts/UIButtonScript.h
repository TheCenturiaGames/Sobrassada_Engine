#pragma once

#include "Script.h"
#include <string>

class UIButtonScript : public Script
{
  public:
    bool Init() override;
    void Update(float deltaTime) override;
    UIButtonScript(GameObject* parent) : Script(parent) {}

  private:
    void OnButtonClicked();

    std::string screenToActivate = "MainPanel";
};
