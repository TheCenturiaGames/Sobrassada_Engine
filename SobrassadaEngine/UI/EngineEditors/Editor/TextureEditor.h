#pragma once

#include "EngineEditorBase.h"

class TextureEditor : public EngineEditorBase
{
  public:
    TextureEditor(const std::string& editorName, UID uid);
    ~TextureEditor() override;

  private:
    bool RenderEditor() override;
    void Viewport();

    UID selectedTextureUID  = 0;
    bool showViewportWindow = false;
    bool showR              = true;
    bool showG              = true;
    bool showB              = true;
};
