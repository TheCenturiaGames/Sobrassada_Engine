#pragma once

#include "Globals.h"

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
    COMPONENT_AIAGENT,
    FIRST = COMPONENT_NONE,
    LAST  = COMPONENT_AIAGENT
};

class ComponentUtils
{
  public:
    static Component* CreateEmptyComponent(ComponentType type, UID uid, GameObject* parent);

    static Component* CreateExistingComponent(const rapidjson::Value& initialState, GameObject* parent);
};
