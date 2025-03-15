
#include "UI/EngineEditorBase.h"
#include "imgui.h"

EngineEditorBase::~EngineEditorBase()
{
}

bool EngineEditorBase::RenderEditor()
{
    bool stillOpen = true;

    if (!ImGui::Begin(name.c_str(), &stillOpen))
    {
        ImGui::End();
        return stillOpen;
    }
    ImVec2 currentSize = ImGui::GetWindowSize();
    if (currentSize.x < 150 || currentSize.y < 150)
    {
        ImGui::SetWindowSize(ImVec2(150, 150), ImGuiCond_Always);
    }
    ImGui::End();
    return stillOpen;
}
