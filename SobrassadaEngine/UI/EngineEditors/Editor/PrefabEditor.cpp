#include "PrefabEditor.h"
#include "Application.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "LibraryModule.h"
#include "ResourcePrefab.h"
#include "ResourcesModule.h"
#include "Standalone/MeshComponent.h"
#include "imgui.h"
#include <unordered_set>

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
    if (selectedPrefab) HandlePrefabViewer();
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
        ResourcePrefab* prefab = static_cast<ResourcePrefab*>(App->GetResourcesModule()->RequestResource(prefabUID));
        bool isSelected        = (prefabUID == currentSelectedPrefabUID);

        if (ImGui::Selectable(name.c_str(), isSelected))
        {
            selectedPrefab           = prefab;
            currentSelectedPrefabUID = prefabUID;

            if (!portView) portView = std::make_unique<PrefabPortView>();
            portView->SetPrefab(prefab);
            selectedGameObject = nullptr;
        }
    }
}

// Manages the Prefab Viewer window and related logic
void PrefabEditor::HandlePrefabViewer()
{
    if (!openPrefabViewer)
    {
        openPrefabViewer = true;
    }

    if (openPrefabViewer)
    {
        ImGui::Begin("Prefab Viewer", &openPrefabViewer);

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

        if (MeshComponent* mesh = selectedGameObject->GetMeshComponent())
        {
            ImGui::SeparatorText("Mesh");
            ImGui::Text(mesh->GetResourceMesh() ? mesh->GetResourceMesh()->GetName().c_str() : "None");
            ImGui::SameLine();

            if (ImGui::Button("Change Mesh"))
            {
                ImGui::OpenPopup("SelectMeshForGO");
            }

            if (ImGui::IsPopupOpen("SelectMeshForGO"))
            {
                UID newMeshUID = App->GetEditorUIModule()->RenderResourceSelectDialog<UID>(
                    "SelectMeshForGO", App->GetLibraryModule()->GetMeshMap(), INVALID_UID
                );

                if (newMeshUID != INVALID_UID)
                {
                    mesh->AddMesh(newMeshUID);
                    portView->LoadMeshAndMaterialFromComponent(mesh);
                }
            }

            ImGui::Text(mesh->GetResourceMaterial() ? mesh->GetResourceMaterial()->GetName().c_str() : "None");
            ImGui::SameLine();

            if (ImGui::Button("Change Material"))
            {
                ImGui::OpenPopup("SelectMaterialForGO");
            }

            if (ImGui::IsPopupOpen("SelectMaterialForGO"))
            {
                UID newMatUID = App->GetEditorUIModule()->RenderResourceSelectDialog<UID>(
                    "SelectMaterialForGO", App->GetLibraryModule()->GetMaterialMap(), INVALID_UID
                );

                if (newMatUID != INVALID_UID)
                {
                    mesh->AddMaterial(newMatUID);
                    portView->LoadMeshAndMaterialFromComponent(mesh);
                }
            }
        }

        if (ImGui::Button("Apply to Prefab"))
        {
            ApplyChangesToOriginalPrefab();
            showAppliedFeedback  = true;
            appliedFeedbackTimer = 2.0f;
        }

        if (showAppliedFeedback)
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Changes applied!");
            appliedFeedbackTimer -= ImGui::GetIO().DeltaTime;
            if (appliedFeedbackTimer <= 0.0f) showAppliedFeedback = false;
        }
    }

    ImGui::EndChild();
}


void PrefabEditor::treeHierarchyView()
{
    if (!portView) return;

    ImGui::InputTextWithHint("##filter", "Filter by name...", filterBuffer, IM_ARRAYSIZE(filterBuffer));
    std::string filter         = filterBuffer;

    const auto& previewObjects = portView->GetPreviewObjects();

    std::unordered_set<UID> allUIDs;
    std::unordered_map<UID, GameObject*> uidToGO;
    std::unordered_set<UID> renderedRoots;

    for (GameObject* go : previewObjects)
    {
        UID uid    = go->GetUID();
        UID parent = go->GetParent();
        allUIDs.insert(uid);
        uidToGO[uid] = go;
    }

    for (GameObject* go : previewObjects)
    {
        UID uid       = go->GetUID();
        UID parentUID = go->GetParent();

        if (allUIDs.find(parentUID) == allUIDs.end())
        {
            if (renderedRoots.find(uid) == renderedRoots.end())
            {
                DrawHierarchyRecursiveFiltered(go, filter);
                renderedRoots.insert(uid);
            }
        }
        else
        {
            GameObject* parentGO = uidToGO[parentUID];
            GLOG(
                "%s (%llu) is child of %s (%llu)", go->GetName().c_str(), uid,
                parentGO ? parentGO->GetName().c_str() : "UNKNOWN", parentUID
            );
        }
    }

    GLOG("treeHierarchyView - END");
}


bool PrefabEditor::DrawHierarchyRecursiveFiltered(GameObject* go, const std::string& filter)
{
    const auto& previewObjects = portView->GetPreviewObjects();

    // Find GameObject by its UID
    auto findByUID             = [&previewObjects](UID uid) -> GameObject*
    {
        for (GameObject* obj : previewObjects)
        {
            if (obj->GetUID() == uid) return obj;
        }
        return nullptr;
    };

    bool matchesFilter = filter.empty() || go->GetName().find(filter) != std::string::npos;

    if (!matchesFilter) return false;

    std::string label = go->GetName() + "##" + std::to_string(go->GetUID());
    bool nodeOpen     = ImGui::TreeNodeEx(label.c_str(), selectedGameObject == go ? ImGuiTreeNodeFlags_Selected : 0);

    if (ImGui::IsItemClicked())
    {
        selectedGameObject = go;
    }

    // If the node is open, recursively draw the children
    if (nodeOpen)
    {
        for (UID childUID : go->GetChildren())
        {
            GameObject* child = findByUID(childUID);
            if (child)
            {
                // Call this function for each child of the current GameObject
                DrawHierarchyRecursiveFiltered(child, filter);
            }
        }
        ImGui::TreePop(); 
    }

    return true;
}






void PrefabEditor::ApplyChangesToOriginalPrefab()
{
    if (!selectedPrefab || !portView) return;

    const auto& originalObjects = selectedPrefab->GetGameObjectsVector();
    const auto& clonedObjects   = portView->GetPreviewObjects();

    std::unordered_map<UID, GameObject*> originalMap;
    for (GameObject* go : originalObjects)
        originalMap[go->GetUID()] = go;

    for (GameObject* clonedGO : clonedObjects)
    {
        auto it = originalMap.find(clonedGO->GetUID());
        if (it != originalMap.end())
        {
            GameObject* originalGO = it->second;

            originalGO->SetLocalTransform(clonedGO->GetLocalTransform());
            originalGO->UpdateTransformForGOBranch();
            originalGO->SetName(clonedGO->GetName());

            if (MeshComponent* clonedMesh = clonedGO->GetMeshComponent())
            {
                GLOG("Applied mesh UID: %llu", clonedMesh->GetMeshUID());
                GLOG("Applied material UID: %llu", clonedMesh->GetMaterialUID());

                MeshComponent* originalMesh = originalGO->GetMeshComponent();
                if (originalMesh)
                {
                    originalMesh->AddMesh(clonedMesh->GetMeshUID());
                    originalMesh->AddMaterial(clonedMesh->GetMaterialUID());

                }
            }
        }
    }

    selectedPrefab->SaveClonedPrefab(clonedObjects);
    selectedPrefab->ReloadFromDisk();

    App->GetResourcesModule()->ReleaseResource(selectedPrefab);
    selectedPrefab = static_cast<ResourcePrefab*>(App->GetResourcesModule()->RequestResource(selectedPrefab->GetUID()));


    selectedGameObject = nullptr;
    portView           = std::make_unique<PrefabPortView>();
    portView->SetPrefab(selectedPrefab);


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

        if (isRoot) roots.push_back(go);
    }

    return roots;
}