﻿#pragma once

#include "Scene/Components/Component.h"

enum ComponentMobilitySettings
{
    STATIC  = 0,
    DYNAMIC = 1,
};

class RootComponent : public Component
{
  public:
    RootComponent(UID uid, UID uidParent, const float4x4& parentGlobalTransform);

    RootComponent(const rapidjson::Value& initialState);

    ~RootComponent() override;

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

    AABB& TransformUpdated(const float4x4& parentGlobalTransform) override;

    virtual bool CreateComponent(ComponentType componentType);

    void RenderComponentEditor();
    void RenderEditorComponentTree(UID selectedComponentUID) override;
    void RenderEditorInspector() override;

    void Update() override;

    void SetSelectedComponent(UID componentUID);
    bool IsSelectedComponent(UID componentUID) const { return selectedUID == componentUID; }

    int GetMobilitySettings() const { return mobilitySettings; }

    const float4x4& GetParentGlobalTransform() override;

  private:
    Component* GetSelectedComponent();

  private:
    UID selectedUID;
    int mobilitySettings         = DYNAMIC;

    Component* selectedComponent = nullptr;
};
