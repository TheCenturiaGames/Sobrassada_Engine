#pragma once

#include "EngineEditors/EngineEditorBase.h"

class PrefabEditor : public EngineEditorBase
{
  public:
    PrefabEditor(const std::string& editorName, UID uid);
    ~PrefabEditor() override;

  private:
    bool RenderEditor() override;
};