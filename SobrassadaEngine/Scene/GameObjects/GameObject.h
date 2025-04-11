#pragma once

#include "ComponentUtils.h"
#include "Globals.h"

#include "Geometry/AABB.h"
#include "Geometry/OBB.h"
#include "rapidjson/document.h"
#include <string>
#include <unordered_map>
#include <vector>

class MeshComponent;

enum MobilitySettings
{
    DYNAMIC = 0,
    STATIC  = 1,
};

class SOBRASADA_API_ENGINE GameObject
{
  public:
    GameObject(const std::string& name);
    GameObject(UID parentUID, const std::string& name);
    GameObject(UID parentUID, const std::string& name, UID uid);
    GameObject(UID parentUID, GameObject* refObject);

    GameObject(const rapidjson::Value& initialState);

    ~GameObject();

    void Init();

    const float4x4& GetParentGlobalTransform() const;

    bool IsStatic() const { return mobilitySettings == MobilitySettings::STATIC; };
    bool IsTopParent() const { return isTopParent; };

    bool AddGameObject(UID gameObjectUID);
    bool RemoveGameObject(UID gameObjectUID);

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;

    void RenderHierarchyNode(UID& selectedGameObjectUID);
    void HandleNodeClick(UID& selectedGameObjectUID);
    void RenderContextMenu();
    void RenderEditorInspector();

    void UpdateGameObjectHierarchy(UID sourceUID);
    void RenameGameObjectHierarchy();
    bool TargetIsChildren(UID uidTarget);

    const std::string& GetName() const { return name; }
    void SetName(const std::string& newName) { name = newName; }

    const std::vector<UID>& GetChildren() const { return children; }
    void AddChildren(UID childUID) { children.push_back(childUID); }

    UID GetParent() const { return parentUID; }
    void SetParent(UID newParentUID) { parentUID = newParentUID; }

    UID GetUID() const { return uid; }
    void SetUID(UID newUID) { uid = newUID; }

    const AABB& GetLocalAABB() const { return localAABB; }

    const AABB& GetGlobalAABB() const { return globalAABB; }
    const OBB& GetGlobalOBB() const { return globalOBB; }

    void OnAABBUpdated();

    void Render(float deltatime) const;
    void RenderEditor();

    const float4x4& GetGlobalTransform() const { return globalTransform; }
    const float4x4& GetLocalTransform() const { return localTransform; }

    bool CreateComponent(ComponentType componentType);
    bool RemoveComponent(ComponentType componentType);

    // Updates the transform for this game object and all descending children
    void UpdateTransformForGOBranch();
    void UpdateMobilityHierarchy(MobilitySettings type);

    const std::unordered_map<ComponentType, Component*>& GetComponents() const { return components; }
    Component* GetComponentByType(ComponentType type) const;

    MeshComponent* GetMeshComponent() const;
    const float3& GetPosition() const { return position; }
    const float3& GetRotation() const { return rotation; }
    const float3& GetScale() const { return scale; }

    void SetLocalTransform(const float4x4& newTransform);
    void DrawGizmos() const;

    void CreatePrefab();
    UID GetPrefabUID() const { return prefabUID; }
    void SetPrefabUID(const UID uid) { prefabUID = uid; }

    void OnTransformUpdated();
    void UpdateComponents();
    AABB GetHierarchyAABB();

    void SetPosition(float3& newPosition) { position = newPosition; };
    void SetWillUpdate(bool willUpdate) { this->willUpdate = willUpdate; };
    bool WillUpdate() const { return willUpdate; };

  private:
    void UpdateLocalTransform(const float4x4& parentGlobalTransform);
    void DrawNodes() const;
    void OnDrawConnectionsToggle();

    void SetMobility(MobilitySettings newMobility) { mobilitySettings = newMobility; };

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

    UID prefabUID                        = INVALID_UID;
    bool drawNodes                       = false;

    float3 position                      = float3::zero;
    float3 rotation                      = float3::zero;
    float3 scale                         = float3::one;

    float4x4 localTransform              = float4x4::identity;
    float4x4 globalTransform             = float4x4::identity;

    ComponentType selectedComponentIndex = COMPONENT_NONE;
    int mobilitySettings                 = STATIC;
    bool isTopParent                     = false;
    bool willUpdate                      = false;
};