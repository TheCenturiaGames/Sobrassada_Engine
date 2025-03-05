#pragma once

#include "Globals.h"
#include "Scene/AABBUpdatable.h"
#include "Transform.h"

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
    const Transform& GetGlobalTransform() const override;
    const Transform& GetParentGlobalTransform() override;

    bool CreateRootComponent();
    bool AddGameObject(UID gameObjectUUID);
    inline void AddChildren(UID childUUID) { children.push_back(childUUID); }
    bool RemoveGameObject(UID gameObjectUUID);

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;
    void SaveToLibrary();

    void OnEditor();
    void Render();
    void RenderEditor();
    void RenderHierarchyNode(UID& selectedGameObjectUUID);
    void HandleNodeClick(UID& selectedGameObjectUUID);
    void RenderContextMenu();

    bool UpdateGameObjectHierarchy(UID sourceUID, UID targetUID);
    void RenameGameObjectHierarchy();

    inline std::string GetName() const { return name; }
    inline UID GetUID() const { return uuid; }
    inline std::vector<UID> GetChildren() const { return children; }
    inline UID GetParent() const { return parentUUID; }
    inline const AABB& GetAABB() const { return globalAABB; };
    RootComponent* GetRootComponent() const { return rootComponent; }

    void SetName(std::string newName) { name = newName; }
    void SetParent(UID newParentUUID) { parentUUID = newParentUUID; }
    void SetUUID(UID newUUID) { uuid = newUUID; }

    void CreatePrefab();

  public:
    inline static UID currentRenamingUID = INVALID_UUID;

  private:
    UID uuid;
    std::string name;
    UID parentUUID;
    std::vector<UID> children;
    RootComponent* rootComponent;
    AABB globalAABB;

    bool isRenaming = false;
    char renameBuffer[128];

    UID prefabUid = CONSTANT_EMPTY_UID;
};
