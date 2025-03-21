#pragma once

#include "Module.h"

class GameUIModule : Module
{
  public:
    GameUIModule();
    ~GameUIModule() override;

    bool Init() override;
    update_status Update(float deltaTime) override;
    update_status Render(float deltaTime) override;
    bool ShutDown() override;
};