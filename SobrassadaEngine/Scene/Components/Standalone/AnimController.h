#pragma once
#include "Globals.h"



class ResourceAnimation;

class AnimController
{
  public:
   
    AnimController();
    ~AnimController();
                                    
    void GetTransform(const std::string& nodeName, float3& pos, Quat& rot); 

    update_status Update();  
    void Play(UID resource, bool loop);
    void Stop();
    void Pause();
    void Resume();
    
   ResourceAnimation* GetCurrentAnimation() { return animation; }
    void SetAnimationResource(ResourceAnimation* anim) { animation = anim; }
    void SetPlaybackSpeed(float speed) { playbackSpeed = speed; }
    void SetTime(float time) { currentTime = time; }
    float GetTime() const { return currentTime; }
    bool IsPlaying() const { return playAnimation; }

  
  
   
  private:
    UID resource; 
    float currentTime                    = 0; 
    bool loop                       = false;
    bool playAnimation                      = false;
    float playbackSpeed                     = 0.5f;
    ResourceAnimation* animation            = nullptr;
};