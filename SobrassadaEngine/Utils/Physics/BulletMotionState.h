#pragma once

#include "btMotionState.h"

class Component;

namespace math
{
    class float3;
}

class BulletMotionState : public btMotionState
{
  public:
    BulletMotionState(Component* newCollider, const math::float3& newCenterOffset, bool newFreezeRotation);
    ~BulletMotionState() = default;

  private:
    Component* collider = nullptr;
    btTransform centerOffset = btTransform::getIdentity();
    bool freezeRotation      = false;
};
