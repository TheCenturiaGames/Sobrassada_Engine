#include "PrefabEditor.h"
#include "Application.h"
#include "LibraryModule.h"
#include "ResourcesModule.h"
#include "imgui.h"
#include "ResourcePrefab.h"
#include "GameObject.h"


PrefabEditor::PrefabEditor(const std::string& editorName, UID uid) : EngineEditorBase(editorName, uid)
{
    portView = std::make_unique<PrefabPortView>(); // crea el portview
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
            portView->SetPrefab(prefab);
        }
    }


    if (selectedPrefab)
    {
        if (!openPrefabViewer)
        {
            openPrefabViewer = true;
            portView->SetPrefab(selectedPrefab); // només quan s'obre la finestra
        }

        if (openPrefabViewer && ImGui::Begin("Prefab Viewer", &openPrefabViewer))
        {
            ImVec2 windowSize = ImGui::GetContentRegionAvail();
            float leftWidth   = windowSize.x * 0.6f;
            float rightWidth  = windowSize.x - leftWidth;

            ImGui::BeginChild("Left_Preview", ImVec2(leftWidth, windowSize.y), true);
            {
                ImGui::Text("Portview");
                portView->Update(0.016f);
                portView->RenderContent();
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

            ImGui::End(); // Prefab Viewer
        }

        // Quan la finestra es tanca, netejem tot
        if (!openPrefabViewer)
        {
            selectedPrefab     = nullptr;
            selectedGameObject = nullptr;
            portView->SetPrefab(nullptr);
        }
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
