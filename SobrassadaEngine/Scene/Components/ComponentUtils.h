#pragma once

#include "Globals.h"

#include <Libs/rapidjson/document.h>
#include <Math/float4x4.h>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>

class Component;

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
    FIRST = COMPONENT_NONE,
    LAST  = COMPONENT_CAMERA
};

class ComponentUtils
{
  public:
    static Component* CreateEmptyComponent(ComponentType type, UID uid, UID uidParent);

    static Component* CreateExistingComponent(const rapidjson::Value& initialState);
};
