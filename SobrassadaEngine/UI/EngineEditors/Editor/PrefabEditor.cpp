#include "PrefabEditor.h"
#include "Application.h"
#include "GameObject.h"
#include "LibraryModule.h"
#include "ResourcePrefab.h"
#include "ResourcesModule.h"
#include "imgui.h"

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

    RenderPrefabList();

    if (selectedPrefab)
    {
        HandlePrefabViewer();
    }

    ImGui::End();
    return true;
}

// Draws the list of loaded prefabs and handles selection logic
void PrefabEditor::RenderPrefabList()
{
    const auto& prefabMap = App->GetLibraryModule()->GetPrefabMap();
    if (prefabMap.empty())
    {
        ImGui::Text("No prefabs loaded.");
        return;
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

            if (!portView) portView = std::make_unique<PrefabPortView>();

            portView->SetPrefab(prefab);
        }
    }
}

// Manages the Prefab Viewer window and related logic
void PrefabEditor::HandlePrefabViewer()
{
    if (!openPrefabViewer)
    {
        openPrefabViewer = true;
        portView->SetPrefab(selectedPrefab);
    }

    if (openPrefabViewer && ImGui::Begin("Prefab Viewer", &openPrefabViewer))
    {
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        float leftWidth   = windowSize.x * 0.6f;
        float rightWidth  = windowSize.x - leftWidth;

        ImGui::BeginChild("Left_Preview", ImVec2(leftWidth, windowSize.y), true);
        RenderPrefabPortView();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Right_Side", ImVec2(rightWidth, windowSize.y), true);
        RenderPrefabSidePanel(rightWidth, windowSize.y);
        ImGui::EndChild();

        ImGui::End();
    }

    if (!openPrefabViewer)
    {
        selectedPrefab     = nullptr;
        selectedGameObject = nullptr;
        portView.reset();
    }
}

// Renders the visual preview of the selected prefab
void PrefabEditor::RenderPrefabPortView()
{
    ImGui::Text("Portview");

    if (portView && portView->HasValidMesh())
    {
        portView->RenderContent();
    }
    else
    {
        ImGui::Text("No mesh to display.");
    }
}

// Renders the hierarchy tree and property panel for selected GameObject
void PrefabEditor::RenderPrefabSidePanel(float width, float height)
{
    float hierarchyHeight = height * 0.4f;
    float propsHeight     = height - hierarchyHeight;

    ImGui::BeginChild("Hierarchy", ImVec2(width, hierarchyHeight), true);
    ImGui::Text("Hierarchy");
    treeHierarchyView();
    ImGui::EndChild();

    ImGui::BeginChild("Properties", ImVec2(width, propsHeight), true);
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
    ImGui::EndChild();
}

void PrefabEditor::treeHierarchyView()
{
    if (!selectedPrefab) return;

    static char filterBuffer[64] = "";
    ImGui::InputTextWithHint("##filter", "Filter by name...", filterBuffer, IM_ARRAYSIZE(filterBuffer));
    std::string filter = filterBuffer;

    for (GameObject* root : GetRootGameObjects(selectedPrefab))
    {
        DrawHierarchyRecursiveFiltered(root, filter);
    }
}

std::vector<GameObject*> PrefabEditor::GetRootGameObjects(ResourcePrefab* prefab)
{
    std::vector<GameObject*> roots;
    const auto& gameObjects = prefab->GetGameObjectsVector();

    for (GameObject* go : gameObjects)
    {
        bool isRoot = true;
        for (GameObject* possibleParent : gameObjects)
        {
            if (go->GetParent() == possibleParent->GetUID())
            {
                isRoot = false;
                break;
            }
        }

        if (isRoot)
        {
            roots.push_back(go);
        }
    }

    return roots;
}

// Tree view it can filter by the name of prefab child
void PrefabEditor::DrawHierarchyRecursiveFiltered(GameObject* go, const std::string& filter)
{
    if (!filter.empty() && go->GetName().find(filter) == std::string::npos)
    {
        // If the name doesn't match the filter, skip drawing this node and its children
        return;
    }

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
                DrawHierarchyRecursiveFiltered(child, filter);
            }
        }
        ImGui::TreePop();
    }
}
