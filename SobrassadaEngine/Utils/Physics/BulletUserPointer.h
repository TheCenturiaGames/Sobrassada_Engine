#pragma once

#include "ComponentUtils.h"
#include "Delegate.h"

#include "Math/float3.h"

class Component;
class GameObject;

struct BulletUserPointer
{
    BulletUserPointer(Component* component, CollisionDelegate* newCallback)
    {
        collider            = component;
        onCollisionCallback = newCallback;
    }

    Component* collider;
    CollisionDelegate* onCollisionCallback;
};