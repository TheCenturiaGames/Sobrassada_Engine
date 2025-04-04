#pragma once

#include "ComponentUtils.h"
#include "Delegate.h"

#include "Math/float3.h"

class Component;
class GameObject;

struct BulletUserPointer
{
    BulletUserPointer(Component* component, CollisionDelegate* newCallback, bool generateCallback)
    {
        collider            = component;
        onCollisionCallback = newCallback;
        this->generateCallback = generateCallback;
    }

    Component* collider;
    CollisionDelegate* onCollisionCallback;
    bool generateCallback;
};