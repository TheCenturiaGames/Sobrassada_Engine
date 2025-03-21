#pragma once

#include "EngineEditors/EngineEditorBase.h"

class TextureEditor : public EngineEditorBase
{
  public:
    TextureEditor(const std::string& editorName, UID uid);
    virtual ~TextureEditor();

    bool RenderEditor() override;
    

  private:
      
    void ViewPort();
    UID selectedTextureUID = 0;
    bool showDetailWindow  = false;
    bool showR             = true;
    bool showG             = true;
    bool showB             = true;
    bool editorOpen        = false;
};
