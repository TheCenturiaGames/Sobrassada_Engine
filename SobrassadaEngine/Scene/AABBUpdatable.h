#pragma once

class AABBUpdatable
{
  public:
    virtual ~AABBUpdatable()                            = default;

    virtual void PassAABBUpdateToParent()               = 0;
    virtual void ComponentGlobalTransformUpdated()      = 0;

    virtual const float4x4& GetGlobalTransform() const  = 0;
    virtual const float4x4& GetParentGlobalTransform()  = 0;
};
