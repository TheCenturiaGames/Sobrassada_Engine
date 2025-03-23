#pragma once

#include "Module.h"
#include "Scene/Components/Standalone/UI/CanvasComponent.h"

#include <ft2build.h>
#include <vector>
#include FT_FREETYPE_H

namespace TextManager
{
    struct FontData;
}

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
    void ResetCanvas() { canvas.clear(); }

  private:
    std::vector<CanvasComponent*> canvas;

    unsigned int uiWidgetProgram = 0;
};