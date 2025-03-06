#pragma once

#include "ComponentUtils.h"
#include "Globals.h"
#include "Scene/AABBUpdatable.h"

#include <Geometry/AABB.h>
#include <Libs/rapidjson/document.h>
#include <string>
#include <unordered_map>
#include <vector>

class RootComponent;
class Component;

enum ComponentMobilitySettings
{
    STATIC  = 0,
    DYNAMIC = 1,
};

class GameObject : public AABBUpdatable
{
  public:
    GameObject(std::string name);
    GameObject(UID parentUUID, std::string name);
    GameObject(UID parentUUID, std::string name, UID rootComponent);

    GameObject(const rapidjson::Value& initialState);

    ~GameObject() override;

    void PassAABBUpdateToParent() override;
    void ComponentGlobalTransformUpdated() override;
    const float4x4& GetParentGlobalTransform() override;

    bool AddGameObject(UID gameObjectUUID);
    bool RemoveGameObject(UID gameObjectUUID);

    void LoadComponentsInGameObject(Component* component);

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;

    void RenderHierarchyNode(UID& selectedGameObjectUUID);
    void HandleNodeClick(UID& selectedGameObjectUUID);
    void RenderContextMenu();
    void RenderEditorInspector();

    bool UpdateGameObjectHierarchy(UID sourceUID, UID targetUID);
    void RenameGameObjectHierarchy();

    inline std::string GetName() const { return name; }
    void SetName(std::string newName) { name = newName; }

    inline std::vector<UID> GetChildren() { return children; }
    inline void AddChildren(UID childUUID) { children.push_back(childUUID); }

    inline UID GetParent() const { return parentUUID; }
    void SetParent(UID newParentUUID) { parentUUID = newParentUUID; }

    inline UID GetUID() const { return uuid; }
    void SetUUID(UID newUUID) { uuid = newUUID; }

    inline const AABB& GetAABB() const { return globalAABB; };

    void Render();
    void RenderEditor();

    const float4x4& GetGlobalTransform() const override { return globalTransform; }
    const float4x4& GetLocalTransform() const { return localTransform; }
    
    bool CreateComponent(ComponentType componentType);
    bool RemoveComponent(ComponentType componentType);

  public:
    inline static UID currentRenamingUID = INVALID_UUID;

  private:
    UID parentUUID;
    UID uuid;
    std::vector<UID> children;

    std::string name;

    std::unordered_map<ComponentType, Component*> components;  

    AABB globalAABB;

    bool isRenaming = false;
    char renameBuffer[128];

    float4x4 localTransform  = float4x4::identity;
    float4x4 globalTransform = float4x4::identity;

    ComponentType selectedComponentIndex;
    int mobilitySettings         = DYNAMIC;
};
