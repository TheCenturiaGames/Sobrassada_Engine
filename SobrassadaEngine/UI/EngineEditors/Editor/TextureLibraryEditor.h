#pragma once

#include "EngineEditors/EngineEditorBase.h"

class TextureLibraryEditor : public EngineEditorBase
{
  public:
    TextureLibraryEditor(const std::string& editorName, UID uid);
    virtual ~TextureLibraryEditor();

    bool RenderEditor() override;
};
