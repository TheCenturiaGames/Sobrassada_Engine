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
    GameObject(UID parentUID, std::string name);
    GameObject(UID parentUID, std::string name, UID rootComponent);
    GameObject(const rapidjson::Value& initialState);
    ~GameObject() override;

    void PassAABBUpdateToParent() override;
    void ComponentGlobalTransformUpdated() override;
    const Transform& GetGlobalTransform() const override;
    const Transform& GetParentGlobalTransform() override;

    bool CreateRootComponent();
    bool AddGameObject(UID gameObjectUID);
    inline void AddChildren(UID childUID) { children.push_back(childUID); }
    bool RemoveGameObject(UID gameObjectUID);

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;
    void SaveToLibrary();

    void OnEditor();
    void Render();
    void RenderEditor();
    void RenderHierarchyNode(UID& selectedGameObjectUID);
    void HandleNodeClick(UID& selectedGameObjectUID);
    void RenderContextMenu();

    bool UpdateGameObjectHierarchy(UID sourceUID, UID targetUID);
    void RenameGameObjectHierarchy();

    inline std::string GetName() const { return name; }
    inline UID GetUID() const { return uid; }
    inline std::vector<UID> GetChildren() { return children; }
    inline UID GetParent() const { return parentUID; }
    inline const AABB& GetAABB() const { return globalAABB; };
    RootComponent* GetRootComponent() const { return rootComponent; }

    void SetName(std::string newName) { name = newName; }
    void SetParent(UID newParentUID) { parentUID = newParentUID; }
    void SetUUID(UID newUID) { uid = newUID; }

  public:
    inline static UID currentRenamingUID = INVALID_UID;

  private:
    UID uid;
    std::string name;
    UID parentUID;
    std::vector<UID> children;
    RootComponent* rootComponent;
    AABB globalAABB;

    bool isRenaming = false;
    char renameBuffer[128];
};
