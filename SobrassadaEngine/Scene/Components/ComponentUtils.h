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
    FIRST = COMPONENT_NONE,
    LAST  = COMPONENT_DIRECTIONAL_LIGHT
};

static const std::unordered_map<std::string, ComponentType> standaloneComponents = {
    {"Mesh",              COMPONENT_MESH             },
    {"Point Light",       COMPONENT_POINT_LIGHT      },
    {"Spot Light",        COMPONENT_SPOT_LIGHT       },
    {"Directional Light", COMPONENT_DIRECTIONAL_LIGHT}
};

class ComponentUtils
{
  public:
    static Component* CreateEmptyComponent(ComponentType type, UID uid, UID uidParent);

    static Component* CreateExistingComponent(const rapidjson::Value& initialState);
};
