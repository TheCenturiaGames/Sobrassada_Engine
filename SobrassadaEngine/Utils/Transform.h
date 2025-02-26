#pragma once

#include "LightsConfig.h"


#include <Math/Quat.h>
#include <Math/float3.h>

struct Transform 
{
    Transform() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1) {}

    Transform(const float3& position, const float3& rotation, const float3& scale): position(position), rotation(rotation), scale(scale) {}

    Transform(const Transform& other) = default;

public:

    Transform operator+(const Transform &transform) const
    {
        return {position + transform.position, rotation + transform.rotation,
            float3(scale.x * transform.scale.x, scale.y * transform.scale.y, scale.z * transform.scale.z)};
    }

    Transform operator-(const Transform &transform) const
    {
        float3 newScale;
        newScale.x = transform.scale.x != 0 ? scale.x / transform.scale.x : 0;  
        newScale.y = transform.scale.y != 0 ? scale.y / transform.scale.y : 0;  
        newScale.z = transform.scale.z != 0 ? scale.z / transform.scale.z : 0;  
        
        return {position - transform.position, rotation - transform.rotation, newScale};
    }

    Transform orientedAdd(const Transform& transform) const
    {
        Transform newTransform;
        newTransform.rotation = rotation + transform.rotation;
        const Quat newRotation = Quat::FromEulerXYZ(newTransform.rotation.x, newTransform.rotation.y, newTransform.rotation.z);

        //float3 rotatedScale = newRotation.Transform(transform.scale);
        //newTransform.scale.x = rotatedScale.x != 0 ? scale.x * rotatedScale.x : 0;  
        //newTransform.scale.y = rotatedScale.y != 0 ? scale.y * rotatedScale.y : 0;  
        //newTransform.scale.z = rotatedScale.z != 0 ? scale.z * rotatedScale.z : 0;
        
        newTransform.position = position + newRotation.Transform(transform.position);
        return newTransform;
    }

    Transform orientedSub(const Transform& transform) const
    {
        Transform newTransform;
        newTransform.rotation = rotation + transform.rotation * -1;
        const Quat newRotation = Quat::FromEulerXYZ(newTransform.rotation.x, newTransform.rotation.y, newTransform.rotation.z);
        
        //float3 rotatedScale = newRotation.Transform(transform.scale);
        //newTransform.scale.x = rotatedScale.x != 0 ? scale.x / rotatedScale.x : 0;  
        //newTransform.scale.y = rotatedScale.y != 0 ? scale.y / rotatedScale.y : 0;  
        //newTransform.scale.z = rotatedScale.z != 0 ? scale.z / rotatedScale.z : 0;
        
        newTransform.position = position + newRotation.Transform(transform.position * -1);
        return newTransform;
    }

    void Set(const Transform &transform);
    
public:
    
    float3 position;
    float3 rotation;
    float3 scale;

    static const Transform identity;
};
