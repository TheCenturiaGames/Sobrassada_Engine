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
    bool RenderEditor() override;
    ResourcePrefab* selectedPrefab = nullptr;
    void treeHierarchyView();
    void DrawHierarchyRecursive(GameObject* go);

    GameObject* selectedGameObject = nullptr;
    std::unique_ptr<PrefabPortView> portView;

    bool openPrefabViewer = false;
};