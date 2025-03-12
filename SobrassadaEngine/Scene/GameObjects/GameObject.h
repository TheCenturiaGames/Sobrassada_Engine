#pragma once

#include "ComponentUtils.h"
#include "Globals.h"

#include <Geometry/AABB.h>
#include <Geometry/OBB.h>
#include <Libs/rapidjson/document.h>
#include <string>
#include <unordered_map>
#include <vector>

class MeshComponent;

enum ComponentMobilitySettings
{
    STATIC  = 0,
    DYNAMIC = 1,
};

class GameObject
{
  public:
    GameObject(std::string name);
    GameObject(UID parentUID, std::string name);

    GameObject(const rapidjson::Value& initialState);

    ~GameObject();

    const float4x4& GetParentGlobalTransform() const;

    bool AddGameObject(UID gameObjectUID);
    bool RemoveGameObject(UID gameObjectUID);

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;

    void RenderHierarchyNode(UID& selectedGameObjectUID);
    void HandleNodeClick(UID& selectedGameObjectUID);
    void RenderContextMenu();
    void RenderEditorInspector();

    void UpdateGameObjectHierarchy(UID sourceUID);
    void RenameGameObjectHierarchy();

    const std::string& GetName() const { return name; }
    void SetName(const std::string& newName) { name = newName; }

    const std::vector<UID>& GetChildren() { return children; }
    void AddChildren(UID childUID) { children.push_back(childUID); }

    UID GetParent() const { return parentUID; }
    void SetParent(UID newParentUID) { parentUID = newParentUID; }

    UID GetUID() const { return uid; }
    void SetUID(UID newUID) { uid = newUID; }

    const AABB& GetLocalAABB() const { return localAABB; }

    const AABB& GetGlobalAABB() const { return globalAABB; }
    const OBB& GetGlobalOBB() const { return globalOBB; }

    void OnAABBUpdated();

    void Render() const;
    void RenderEditor();

    const float4x4& GetGlobalTransform() const { return globalTransform; }
    const float4x4& GetLocalTransform() const { return localTransform; }

    bool CreateComponent(ComponentType componentType);
    bool RemoveComponent(ComponentType componentType);

    // Updates the transform for this game object and all descending children
    void UpdateTransformForGOBranch() const;

    const std::unordered_map<ComponentType, Component*>& GetComponents() const { return components; }

    const MeshComponent* GetMeshComponent() const;
    void AddModel(UID meshUid, UID materialUid) const;

    void SetLocalTransform(const float4x4& newTransform) { localTransform = newTransform; }
    void DrawGizmos() const;

  private:
    void OnTransformUpdated();
    void UpdateLocalTransform(const float4x4& parentGlobalTransform);
    void DrawNodes() const;
    void OnDrawConnectionsToggle();

  public:
    inline static UID currentRenamingUID = INVALID_UID;

  private:
    UID parentUID;
    UID uid;
    std::vector<UID> children;

    std::string name;

    std::unordered_map<ComponentType, Component*> components;

    AABB localAABB;
    AABB globalAABB;
    OBB globalOBB;

    bool isRenaming = false;
    char renameBuffer[128];

    bool drawNodes                       = false;

    float4x4 localTransform              = float4x4::identity;
    float4x4 globalTransform             = float4x4::identity;

    ComponentType selectedComponentIndex = COMPONENT_NONE;
    int mobilitySettings                 = DYNAMIC;
};
