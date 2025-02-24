#include "EngineEditorModule.h"

#include "imgui.h"

void EngineEditorModule::RenderEditor(bool &engineEditorWindow)
{
    if (!ImGui::Begin("Basic Engine Editor", &engineEditorWindow))
    {
        ImGui::End();
        return;
    }
    ImGui::End();
}
