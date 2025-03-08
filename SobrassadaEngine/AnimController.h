#pragma once
#include "ResourceManagement/Resources/ResourceAnimation.h"
#include <chrono>


class AnimController
{
  public:
    AnimController();
    ~AnimController() = default;

    void SetAnimation(ResourceAnimation* animation);                                      
    void GetTransform(const std::string& nodeName, float3& pos, Quat& rot); 

    update_status Update(float deltaTime);  
    void Play(UID resource, bool loop);
    void Stop();
   
  private:
    float3 PosInterpolate(const float3& first, const float3& second, float lambda);
    Quat QuatInterpolate(const Quat& first, const Quat& second, float lambda);

  private:
    UID resource; 
    float currentTime                    = 0; 
    bool loop                       = false;
    bool playAnimation                      = false;
    clock_t startTime                    = 0;
};