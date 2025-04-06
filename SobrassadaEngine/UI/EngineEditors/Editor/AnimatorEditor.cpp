#include "AnimatorEditor.h"
#include "Standalone/AnimationComponent.h"
#include "Application.h"
#include "SceneModule.h"
#include "Scene/Scene.h"
#include "ResourceManagement/Resources/ResourceAnimation.h"
#include "imgui.h"

AnimatorEditor::AnimatorEditor(const std::string& editorName, const UID uid) : EngineEditorBase(editorName, uid)
{
    
    selectedObject    = nullptr;
    selectedAnimation = nullptr;
}

AnimatorEditor::~AnimatorEditor()
{
}

bool AnimatorEditor::RenderEditor()
{
   
    bool stillOpen                             = true;
   

    if (ImGui::Begin(name.c_str(), &stillOpen))
    {
    }

    return stillOpen;
}


