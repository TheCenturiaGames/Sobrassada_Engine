#pragma once

#include "Transform.h"

class AABBUpdatable
{
  public:
    virtual ~AABBUpdatable()                            = default;

    virtual void PassAABBUpdateToParent()               = 0;
    virtual void ComponentGlobalTransformUpdated()      = 0;

    virtual const Transform& GetGlobalTransform() const = 0;
    virtual const Transform& GetParentGlobalTransform() = 0;
};
