#pragma once
#include "EngineEditorBase.h"
class GameObject;
class ResourceAnimation;
class AnimationComponent;
class AnimatorEditor : public EngineEditorBase
{
  public:
    AnimatorEditor(const std::string& editorName, const UID uid);
    ~AnimatorEditor();
    UID GetUID() const { return uid; }
    bool RenderEditor() override;
    void SetSelectedObject(GameObject* obj) { selectedObject = obj; }
    GameObject* GetSelectedObject() const { return selectedObject; }

    private:
    GameObject* selectedObject;
    ResourceAnimation* selectedAnimation;
     bool playing                        = false;
     float currentTime                   = 0.0f;
     float animationDuration             = 5.0f;
    AnimationComponent* currentAnimComp = nullptr;
};