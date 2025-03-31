#include "PrefabEditor.h"
#include "Application.h"
#include "LibraryModule.h"
#include "ResourcesModule.h"
#include "imgui.h"
#include "ResourcePrefab.h"
#include "GameObject.h"


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
                ImGui::Text("No preview available.");

            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Right_Side", ImVec2(rightWidth, windowSize.y), true);
            {
                float hierarchyHeight = windowSize.y * 0.4f;
                float propsHeight     = windowSize.y - hierarchyHeight;

                ImGui::BeginChild("Hierarchy", ImVec2(rightWidth, hierarchyHeight), true);
                {
                    ImGui::Text("Hierarchy");
                    treeHierarchyView();
                    // Tree gameobjects
                }
                ImGui::EndChild();

               ImGui::BeginChild("Properties", ImVec2(rightWidth, propsHeight), true);
                {
                    if (selectedGameObject)
                    {
                        ImGui::Text("Selected: %s", selectedGameObject->GetName().c_str());
                        ImGui::Separator();
                        selectedGameObject->RenderTransformInspector();
                    }
                    else
                    {
                        ImGui::Text("No GameObject selected.");
                    }
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

void PrefabEditor::treeHierarchyView()
{
    if (!selectedPrefab) return;

    for (GameObject* go : selectedPrefab->GetGameObjectsVector())
    {
        //ImGui::Text("GO: %s - ParentUID: %llu", go->GetName().c_str(), go->GetParent());

        bool isRoot = true;
        for (GameObject* possibleParent : selectedPrefab->GetGameObjectsVector())
        {
            if (go->GetParent() == possibleParent->GetUID())
            {
                isRoot = false;
                break;
            }
        }

        if (isRoot)
        {
            DrawHierarchyRecursive(go);
        }

    }
}

void PrefabEditor::DrawHierarchyRecursive(GameObject* go)
{
    std::string label = go->GetName() + "##" + std::to_string(go->GetUID());

    bool nodeOpen     = ImGui::TreeNodeEx(label.c_str(), selectedGameObject == go ? ImGuiTreeNodeFlags_Selected : 0);

    if (ImGui::IsItemClicked())
    {
        selectedGameObject = go;
    }

    if (nodeOpen)
    {
        for (UID childUID : go->GetChildren())
        {
            GameObject* child = selectedPrefab->FindGameObject(childUID);
            if (child)
            {
                DrawHierarchyRecursive(child);
            }
        }
        ImGui::TreePop();
    }
}
