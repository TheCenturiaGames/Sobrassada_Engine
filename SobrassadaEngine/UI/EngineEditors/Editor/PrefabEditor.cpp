#include "PrefabEditor.h"
#include "Application.h"
#include "LibraryModule.h"
#include "ResourcesModule.h"
#include "imgui.h"
#include "ResourcePrefab.h"

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
    const auto& prefabMap = App->GetLibraryModule()->GetPrefabMap();

    if (prefabMap.empty())
    {
        ImGui::Text("No prefabs loaded.");
        ImGui::End();
        return true;
    }

    ImGui::Text("Loaded Prefabs:");
    ImGui::Separator();
    ImGui::Columns(1, "prefabs", false);
    ImGui::Text("Name");

    for (const auto& [name, prefabUID] : prefabMap)
    {
        Resource* resource = App->GetResourcesModule()->RequestResource(prefabUID);
        if (!resource) continue;

        ResourcePrefab* prefab = dynamic_cast<ResourcePrefab*>(resource);
        if (!prefab) continue;

        ImGui::NextColumn();
        ImGui::Text("%d", name.c_str());
    }

    ImGui::End();
    return true;
}