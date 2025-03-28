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

        if (ImGui::Selectable(name.c_str(), selectedPrefab == prefab))
        {
            selectedPrefab = prefab;
        }
    }

    if (selectedPrefab)
    {
        bool openPrefabViewer = true; // member variable?
        if (ImGui::Begin("Prefab Viewer", &openPrefabViewer))
        {
            ImVec2 windowSize = ImGui::GetContentRegionAvail();

            //Divide: left (portview) i right (herarchy + properties)
            float leftWidth   = windowSize.x * 0.6f;
            float rightWidth  = windowSize.x - leftWidth;

            ImGui::BeginChild("Left_Preview", ImVec2(leftWidth, windowSize.y), true);
            {
                ImGui::Text("Portview");

                ImVec2 size = ImVec2(leftWidth - 20, leftWidth - 20); // Square

                // previewtexture
                // ImGui::Image(previewTexture, size);

                
                ImGui::Dummy(size);
                ImGui::GetWindowDrawList()->AddRect(
                    ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 255, 255) // white line
                );
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
                ImGui::Text("No preview disponible.");

            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Right_Side", ImVec2(rightWidth, windowSize.y), true);
            {
                float hierarchyHeight = windowSize.y * 0.4f;
                float propsHeight     = windowSize.y - hierarchyHeight;

                ImGui::BeginChild("Hierarchy", ImVec2(rightWidth, hierarchyHeight), true);
                {
                    ImGui::Text("Jerarquia");
                    // Tree gameobjects
                }
                ImGui::EndChild();

                ImGui::BeginChild("Properties", ImVec2(rightWidth, propsHeight), true);
                {
                    ImGui::Text("Propietats");
                    // Position, Rotation, ...
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }
        ImGui::End();

    }



    ImGui::End();
    return true;
}