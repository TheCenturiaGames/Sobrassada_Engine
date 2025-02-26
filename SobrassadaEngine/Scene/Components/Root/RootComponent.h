#pragma once

#include "Scene/Components/Component.h"

enum ComponentMobilitySettings
{
    STATIC  = 0,
    DYNAMIC = 1,
};

class RootComponent : public Component
{
  public:
    RootComponent(UID uid, UID uidParent, const Transform& parentGlobalTransform);
    RootComponent(const rapidjson::Value& initialState);
    ~RootComponent() override;

    void Update() override;
    void RenderEditorInspector() override;
    void RenderEditorComponentTree(UID selectedComponentUID) override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    AABB& TransformUpdated(const Transform& parentGlobalTransform) override;

    virtual bool CreateComponent(ComponentType componentType);
    void RenderComponentEditor();
    void RenderGuizmo();

    bool IsSelectedComponent(UID componentUID) const { return selectedUID == componentUID; }
    int GetMobilitySettings() const { return mobilitySettings; }

    void SetSelectedComponent(UID componentUID);

  private:
    UID selectedUID;
    int mobilitySettings = DYNAMIC;
};
