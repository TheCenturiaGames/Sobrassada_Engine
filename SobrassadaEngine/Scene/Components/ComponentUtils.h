#pragma once

#include "Globals.h"
#include "Delegate.h"

#include "Math/float4x4.h"
#include "rapidjson/document.h"
#include <cstdint>

class Component;
class GameObject;

enum ComponentType
{
    // Empty type
    COMPONENT_NONE = 0,
    // Standalone types
    COMPONENT_MESH,
    COMPONENT_POINT_LIGHT,
    COMPONENT_SPOT_LIGHT,
    COMPONENT_DIRECTIONAL_LIGHT,
    COMPONENT_CHARACTER_CONTROLLER,
    COMPONENT_CAMERA,
    COMPONENT_SCRIPT,
    COMPONENT_CUBE_COLLIDER,
    COMPONENT_SPHERE_COLLIDER,
    COMPONENT_CAPSULE_COLLIDER,
    FIRST = COMPONENT_NONE,
    LAST  = COMPONENT_CAPSULE_COLLIDER
};

enum class ColliderType : uint8_t
{
    STATIC = 0,
    DYNAMIC,
    KINEMATIC,
    TRIGGER
};

constexpr const char* ColliderTypeStrings[] = {"Static", "Dynamic", "Kinematic", "Trigger"};

// LAYERS IN THE PHYSICS WORLD ARE REPRESENTED AS AN INT, SO 32 LAYERS ARE THE MAXIMUM ALLOWED WITH CURRENT
// IMPLEMENTATION
enum class ColliderLayer : uint8_t
{
    WORLD_OBJECTS = 0,
    TRIGGERS,
    ENEMY,
    PLAYER,
};

constexpr const char* ColliderLayerStrings[] = {"World Objects", "Triggers", "Enemies", "Player"};

typedef Delegate<void, GameObject*, float3> CollisionDelegate;

class ComponentUtils
{
  public:
    static Component* CreateEmptyComponent(ComponentType type, UID uid, GameObject* parent);

    static Component* CreateExistingComponent(const rapidjson::Value& initialState, GameObject* parent);
};
