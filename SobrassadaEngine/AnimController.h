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
    UID resource; 
    uint32_t currentTime                    = 0; 
    bool loop                       = false;
};