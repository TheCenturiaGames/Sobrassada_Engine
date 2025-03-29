#pragma once

#include "ComponentUtils.h"

class Component;

struct Collider
{
    Collider(Component* component, ComponentType type)
    {
        collider     = component;
        colliderType = type;
    }

    void* collider;
    ComponentType colliderType;
};