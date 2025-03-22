#include "PrefabEditor.h"
#include "imgui.h"

PrefabEditor::PrefabEditor(const std::string& editorName, UID uid) : EngineEditorBase(editorName, uid)
{
}

PrefabEditor::~PrefabEditor()
{
}

bool PrefabEditor::RenderEditor()
{
    if (!EngineEditorBase::RenderEditor()) return false;

    ImGui::Begin(name.c_str());

    ImGui::End();
    return true;
}