#pragma once

#include "ComponentUtils.h"
#include "Globals.h"
#include "Scene/GameObjects/GameObject.h"

#include <Geometry/AABB.h>
#include <Libs/rapidjson/document.h>

class Component
{
  public:
    Component(UID uid, UID parentUID, const char* initName, ComponentType type);
    Component(const rapidjson::Value& initialState);
    virtual ~Component() = default;

    virtual void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;
    virtual void Clone(const Component* other)  = 0;

    virtual void Update() = 0;
    virtual void Render() = 0;
    virtual void RenderEditorInspector();

    UID GetUID() const { return uid; }

    const AABB& GetLocalAABB() const { return localComponentAABB; }

    ComponentType GetType() const { return type; }
    const char* GetName() const { return name; }

    const float4x4& GetGlobalTransform();

  protected:
    GameObject* GetParent();

  protected:
    const UID uid;
    const UID parentUID;

    char name[64];
    bool enabled;

    AABB localComponentAABB;

    const ComponentType type = COMPONENT_NONE;

  private:
    GameObject* parent = nullptr;
};
