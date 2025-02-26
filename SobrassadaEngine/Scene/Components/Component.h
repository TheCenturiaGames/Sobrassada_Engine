﻿#pragma once

#include "ComponentUtils.h"
#include "Globals.h"
#include "Scene/AABBUpdatable.h"
#include "Transform.h"

#include <Geometry/AABB.h>
#include <Libs/rapidjson/document.h>
#include <vector>

class Component : public AABBUpdatable
{
  public:
    Component(
        UID uid, UID uidParent, UID uidRoot, const char* initName, int type, const Transform& parentGlobalTransform
    );
    Component(const rapidjson::Value& initialState);
    ~Component() override;

    void PassAABBUpdateToParent() override;
    void ComponentGlobalTransformUpdated() override {}
    const Transform& GetGlobalTransform() const override { return globalTransform; }
    const Transform& GetParentGlobalTransform() override;

    virtual void Update() = 0;
    virtual void Render();
    virtual void RenderEditorInspector();
    virtual void RenderEditorComponentTree(UID selectedComponentUID);
    virtual void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;

    virtual bool AddChildComponent(UID componentUID);
    virtual bool RemoveChildComponent(UID componentUID);
    virtual bool DeleteChildComponent(UID componentUID);

    virtual void OnTransformUpdate(const Transform& parentGlobalTransform);
    virtual AABB& TransformUpdated(const Transform& parentGlobalTransform);

    void HandleDragNDrop();
    void CalculateLocalAABB();

    UID GetUID() const { return uid; }
    UID GetUIDParent() const { return uidParent; }
    const std::vector<UID>& GetChildren() const { return children; }
    const Transform& GetLocalTransform() const { return localTransform; }
    const AABB& GetGlobalAABB() const { return globalComponentAABB; }
    int GetType() const { return type; }

    void SetUIDParent(UID newUIDParent);

    void SetLocalTransform(const Transform& newTransform);

  protected:
    RootComponent* GetRootComponent();
    AABBUpdatable* GetParent();
    std::vector<Component*>& GetChildComponents();

  protected:
    const UID uid;
    const UID uidRoot;
    UID uidParent;
    std::vector<UID> children;

    char name[64];
    bool enabled;

    Transform localTransform;
    Transform globalTransform;

    AABB localComponentAABB;
    AABB globalComponentAABB;

    const int type = COMPONENT_NONE;

  private:
    RootComponent* rootComponent = nullptr;
    AABBUpdatable* parent        = nullptr;
    std::vector<Component*> childComponents;
};
