#pragma once
#include "Globals.h"



class ResourceAnimation;

class AnimController
{
  public:
    AnimController();
    ~AnimController() = default;
                                    
    void GetTransform(const std::string& nodeName, float3& pos, Quat& rot); 

    update_status Update();  
    void Play(UID resource, bool loop);
    void Stop();
    
   
    void SetPlaybackSpeed(float speed) { playbackSpeed = speed; }
    void SetTime(float time) { currentTime = time; }
    float GetTime() const { return currentTime; }
  private:
   
  private:
    UID resource; 
    float currentTime                    = 0; 
    bool loop                       = false;
    bool playAnimation                      = false;
    float playbackSpeed                     = 1.0f;
    ResourceAnimation* animation            = nullptr;
};