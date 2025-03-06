#pragma once

#include "Globals.h"

#include <Libs/rapidjson/document.h>
#include <cstdint>
#include <map>
#include <string>
#include <Math/float4x4.h>

class Component;
class RootComponent;
class MeshComponent;

enum ComponentType
{
    // Empty type
    COMPONENT_NONE              = 0,
    // Root types
    COMPONENT_ROOT              = 1,
    // Standalone types
    COMPONENT_MESH              = 2,
    COMPONENT_POINT_LIGHT       = 4,
    COMPONENT_SPOT_LIGHT        = 5,
    COMPONENT_DIRECTIONAL_LIGHT = 6,
    FIRST = COMPONENT_NONE,
    LAST = COMPONENT_DIRECTIONAL_LIGHT
};

static const std::map<std::string, ComponentType> standaloneComponents = {
    {"Mesh",              COMPONENT_MESH             },
    {"Point Light",       COMPONENT_POINT_LIGHT      },
    {"Spot Light",        COMPONENT_SPOT_LIGHT       },
    {"Directional Light", COMPONENT_DIRECTIONAL_LIGHT}
};

class ComponentUtils
{
  public:
    static Component* CreateEmptyComponent(ComponentType type, UID uid);

    static Component* CreateExistingComponent(const rapidjson::Value& initialState);
};
