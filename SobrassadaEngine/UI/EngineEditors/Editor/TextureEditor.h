#pragma once

#include "EngineEditors/EngineEditorBase.h"

class TextureEditor : public EngineEditorBase
{
  public:
    TextureEditor(const std::string& editorName, UID uid);
    ~TextureEditor() override;

  private:
    bool RenderEditor() override;
    void Viewport();

    UID selectedTextureUID  = 0;
    bool showViewPortWindow = false;
    bool showR              = true;
    bool showG              = true;
    bool showB              = true;
};
