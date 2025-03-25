#pragma once
#include "EngineEditorBase.h"
class AnimatorEditor : public EngineEditorBase
{
  public:
    AnimatorEditor(const std::string& editorName, const UID uid);
    ~AnimatorEditor();
    UID GetUID() const { return uid; }
    bool RenderEditor() override;

 
};