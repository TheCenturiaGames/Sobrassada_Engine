#pragma once

#include "Module.h"

#include <ft2build.h>
#include <vector>
#include FT_FREETYPE_H

namespace TextManager
{
    struct FontData;
}

class CanvasComponent;

class GameUIModule : public Module
{
  public:
    GameUIModule();
    ~GameUIModule() override;

    bool Init() override;
    update_status Update(float deltaTime) override;
    update_status Render(float deltaTime) override;
    bool ShutDown() override;

    void OnWindowResize(const unsigned int width, const unsigned int height);
    void AddCanvas(CanvasComponent* newCanvas) { canvases.push_back(newCanvas); }
    void RemoveCanvas(CanvasComponent* canvasToRemove);

  private:
    std::vector<CanvasComponent*> canvases;
};