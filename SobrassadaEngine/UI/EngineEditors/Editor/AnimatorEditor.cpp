#include "AnimatorEditor.h"
#include "imgui.h"

AnimatorEditor::AnimatorEditor(const std::string& editorName, const UID uid)
    : EngineEditorBase(editorName, uid)
{
}

AnimatorEditor::~AnimatorEditor()
{
}

bool AnimatorEditor::RenderEditor()
{
    bool stillOpen                 = true;

    static bool playing            = false;
    static float currentTime       = 0.0f;
    static float animationDuration = 5.0f;
    if (ImGui::Begin(name.c_str(), &stillOpen))
    {
        ImGui::Text("Additional Animation Settings");

        if (ImGui::Button("Play")) playing = true;
        ImGui::SameLine();
        if (ImGui::Button("Pause")) playing = false;
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            playing     = false;
            currentTime = 0.0f;
        }

        ImGui::SliderFloat("TimeLine", &currentTime, 0.0f, animationDuration, "%.2f sec");

        if (playing)
        {
            currentTime += ImGui::GetIO().DeltaTime;
            if (currentTime > animationDuration) currentTime = 0.0f;
        }

        ImGui::End();
    }

    return stillOpen;
}
