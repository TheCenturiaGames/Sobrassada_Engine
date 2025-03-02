#pragma once
#include "ResourceManagement/Resources/ResourceAnimation.h"


class AnimController
{
  public:
    AnimController();
    ~AnimController() = default;

    void SetAnimation(ResourceAnimation* animation);                                      
    void GetTransform(const std::string& nodeName, float3& pos, Quat& rot); 

    update_status Update();  
    void Play(UID resource, bool loop);
    void Stop();
   
  private:
    float3 PosInterpolate(const float3& first, const float3& second, float lambda)
    {
        return first * (1.0f - lambda) + second * lambda;
    }

    Quat Interpolate(const Quat& first, const Quat& second, float lambda)
    {
        Quat result;
        float dot = first.Dot(second);
        if (dot >= 0.0f) 
        {
            result.x = first.x * (1.0f - lambda) + second.x * lambda;
            result.y = first.y * (1.0f - lambda) + second.y * lambda;
            result.z = first.z * (1.0f - lambda) + second.z * lambda;
            result.w = first.w * (1.0f - lambda) + second.w * lambda;
        }
        else
        {
            result.x = first.x * (1.0f - lambda) - second.x * lambda;
            result.y = first.y * (1.0f - lambda) - second.y * lambda;
            result.z = first.z * (1.0f - lambda) - second.z * lambda;
            result.w = first.w * (1.0f - lambda) - second.w * lambda;
        }
        result.Normalize();
        return result;
    }

  private:
    UID resource; 
    float currentTime                    = 0.0; 
    bool loop                       = false;
    bool playAnimation                      = false;
};