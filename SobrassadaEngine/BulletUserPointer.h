#pragma once

#include "ComponentUtils.h"

class Component;

struct BulletUserPointer
{
    BulletUserPointer(Component* component, ComponentType type)
    {
        collider     = component;
        colliderType = type;
    }

    Component* collider;
    ComponentType colliderType;
};