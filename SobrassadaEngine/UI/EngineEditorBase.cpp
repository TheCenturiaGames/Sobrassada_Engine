#include "EngineEditorBase.h"

#include "imgui.h"

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


