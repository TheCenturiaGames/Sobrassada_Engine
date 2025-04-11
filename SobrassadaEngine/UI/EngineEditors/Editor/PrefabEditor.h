#pragma once

#include "EngineEditors/EngineEditorBase.h"
#include "PrefabPortView.h"

class ResourcePrefab;
class GameObject;

class PrefabEditor : public EngineEditorBase
{
  public:
    PrefabEditor(const std::string& editorName, UID uid);
    ~PrefabEditor() override;

  private:
    // Core editor logic
    bool RenderEditor() override;
    void RenderPrefabList();
    void HandlePrefabViewer();
    void RenderPrefabPortView();
    void RenderPrefabSidePanel(float width, float height);

    // Hierarchy rendering
    void treeHierarchyView();
    std::vector<GameObject*> GetRootGameObjects(ResourcePrefab* prefab);
    bool DrawHierarchyRecursiveFiltered(GameObject* go, const std::string& filter);
    void ApplyChangesToOriginalPrefab();

    // State
    ResourcePrefab* selectedPrefab           = nullptr;
    GameObject* selectedGameObject           = nullptr;
    std::unique_ptr<PrefabPortView> portView = nullptr;
    bool openPrefabViewer                    = false;
    UID currentSelectedPrefabUID = INVALID_UID;
    char filterBuffer[64]                    = "";


    float appliedFeedbackTimer               = 0.0f;
    bool showAppliedFeedback                 = false;
};