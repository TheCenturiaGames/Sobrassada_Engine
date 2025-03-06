#pragma once

#include "Globals.h"
#include "Scene/AABBUpdatable.h"

#include <Geometry/AABB.h>
#include <Libs/rapidjson/document.h>
#include <string>
#include <vector>

class RootComponent;

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
    const float4x4& GetGlobalTransform() const override;
    const float4x4& GetParentGlobalTransform() override;

    bool AddGameObject(UID gameObjectUUID);
    bool RemoveGameObject(UID gameObjectUUID);

    bool CreateRootComponent();

    void OnEditor();

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;

    void SaveToLibrary();

    void RenderHierarchyNode(UID& selectedGameObjectUUID);
    void HandleNodeClick(UID& selectedGameObjectUUID);
    void RenderContextMenu();

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

    RootComponent* GetRootComponent() const { return rootComponent; }

    inline const AABB& GetAABB() const { return globalAABB; };

    void Render();
    void RenderEditor();

  public:
    inline static UID currentRenamingUID = INVALID_UUID;

  private:
    UID parentUUID;
    UID uuid;
    std::vector<UID> children;

    std::string name;

    RootComponent* rootComponent;

    AABB globalAABB;

    bool isRenaming = false;
    char renameBuffer[128];
};
