#pragma once
#include "Script.h"
class CharacterScript : public Script
{
  public:
    CharacterScript(GameObject* parent) : Script(parent) {}
    bool Init() override;
    void Update(float deltaTime) override;
};
