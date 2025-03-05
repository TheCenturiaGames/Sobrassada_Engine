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
    GameObject(UID parentUID, std::string name);
    GameObject(UID parentUID, std::string name, UID rootComponent);
    GameObject(const rapidjson::Value& initialState);

    ~GameObject() override;

    void PassAABBUpdateToParent() override;
    void ComponentGlobalTransformUpdated() override;
    const float4x4& GetGlobalTransform() const override;
    const float4x4& GetParentGlobalTransform() override;

    bool AddGameObject(UID gameObjectUID);
    bool RemoveGameObject(UID gameObjectUID);

    bool CreateRootComponent();

    void OnEditor();

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
    void SetParent(UID newParentUUID) { parentUUID = newParentUUID; }

    inline UID GetUID() const { return uuid; }
    void SetUUID(UID newUUID) { uuid = newUUID; }

    RootComponent* GetRootComponent() const { return rootComponent; }

    inline const AABB& GetAABB() const { return globalAABB; };

    void SetName(std::string newName) { name = newName; }
    void SetParent(UID newParentUID) { parentUID = newParentUID; }
    void SetUUID(UID newUID) { uid = newUID; }

  public:
    inline static UID currentRenamingUID = INVALID_UID;

  private:
    UID uid;
    std::vector<UID> children;

    std::string name;
    UID parentUID;
    std::vector<UID> children;
    RootComponent* rootComponent;

    AABB globalAABB;

    bool isRenaming = false;
    char renameBuffer[128];
};
