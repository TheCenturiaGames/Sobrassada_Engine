#pragma once

#include "ComponentUtils.h"
#include "Globals.h"
#include "Scene/AABBUpdatable.h"

#include <Geometry/AABB.h>
#include <Libs/rapidjson/document.h>
#include <vector>
#include <Math/float4x4.h>

class Component : public AABBUpdatable
{
  public:
    Component(UID uid, UID uidParent, UID uidRoot, const char *initName, int type, const float4x4 &parentGlobalTransform);

    Component(const rapidjson::Value &initialState);

    ~Component() override;

    virtual void Save(rapidjson::Value &targetState, rapidjson::Document::AllocatorType &allocator) const;

    virtual void Update() = 0;
    virtual void Render();

    virtual bool AddChildComponent(UID componentUID);
    virtual bool RemoveChildComponent(UID componentUID);
    virtual bool DeleteChildComponent(UID componentUID);

    virtual void RenderEditorInspector();
    virtual void RenderEditorComponentTree(UID selectedComponentUID);

    virtual void OnTransformUpdate(const float4x4 &parentGlobalTransform);
    virtual AABB &TransformUpdated(const float4x4 &parentGlobalTransform);
    void PassAABBUpdateToParent() override;

    void ComponentGlobalTransformUpdated() override {}

    const float4x4& GetParentGlobalTransform() override;

    void HandleDragNDrop();

    UID GetUID() const { return uid; }

    UID GetUIDParent() const { return uidParent; }

    const std::vector<UID> &GetChildren() const { return children; }

    void SetUIDParent(UID newUIDParent);

    const float4x4 &GetGlobalTransform() const override { return globalTransform; }
    const float4x4 &GetLocalTransform() const { return localTransform; }

    const AABB &GetGlobalAABB() const { return globalComponentAABB; }

    void CalculateLocalAABB();

    int GetType() const { return type; }

    void RenderGuizmo();

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

    float4x4 localTransform = float4x4::identity;
    float4x4 globalTransform = float4x4::identity;

    AABB localComponentAABB;
    AABB globalComponentAABB;

    const int type = COMPONENT_NONE;

private:
    
    RootComponent* rootComponent = nullptr;
    AABBUpdatable* parent = nullptr;
    std::vector<Component*> childComponents;
};
