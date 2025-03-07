#include "EngineEditorBase.h"

#include "imgui.h"
EngineEditorBase::EngineEditorBase(const std::string& editorName, const UID uid) : name(editorName), uid(uid)
{
}
EngineEditorBase::~EngineEditorBase()
{

}

bool EngineEditorBase::RenderEditor()
{
    bool stillOpen = true;
    if (ImGui::Begin(name.c_str(), &stillOpen))
    {
        ImGui::End();
        return stillOpen;
    }
    ImGui::End();
    return stillOpen;
}
