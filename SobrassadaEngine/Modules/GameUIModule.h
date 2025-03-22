#pragma once

#include "Module.h"
#include "Scene/Components/Standalone/UI/CanvasComponent.h"

#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H

class GameUIModule : public Module
{
  public:
    GameUIModule();
    ~GameUIModule() override;

    bool Init() override;
    update_status Update(float deltaTime) override;
    update_status Render(float deltaTime) override;
    bool ShutDown() override;

    void AddCanvas(CanvasComponent* newCanvas) { canvas.push_back(newCanvas); }

  private:
    std::vector<CanvasComponent*> canvas;
};