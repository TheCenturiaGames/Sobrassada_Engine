#pragma once
#include "Globals.h"

class ResourceAnimation;
class Channel;

class AnimController
{
  public:
    AnimController();
    ~AnimController();

    update_status Update(float deltaTime);
    void Play(UID resource, bool loop);
    void Stop();
    void Pause() { playAnimation = false; }
    void Resume() { playAnimation = true; }

    void GetTransform(const std::string& nodeName, float3& pos, Quat& rot);

    ResourceAnimation* GetCurrentAnimation() const { return currentAnimation; }
    float GetTime() const { return currentTime; }

    void SetTargetAnimationResource(UID uid, unsigned transitionTime, bool shouldLoop);
    void SetPlaybackSpeed(float speed) { playbackSpeed = speed; }
    void SetTime(float time) { currentTime = time; }

    bool IsPlaying() const { return playAnimation; }

  private:
    void GetChannelPosition(const Channel* animChannel, float3& pos, float time) const;
    void GetChannelRotation(Channel* animChannel, Quat& rot, float time);
   
    void SetAnimationResource(ResourceAnimation* anim) { currentAnimation = anim; }

    Quat Interpolate(Quat& first, Quat& second, float lambda);
  
  private:
    UID resource                        = INVALID_UID;
    float currentTime                   = 0.0f;
    bool loop                           = false;
    bool playAnimation                  = false;
    float playbackSpeed                 = 1.0f;
    float transitionTime                = 0.0f;
    float fadeTime                      = 0.0f;
    float currentTargetTime             = 0.0f;

    ResourceAnimation* currentAnimation = nullptr;
    ResourceAnimation* targetAnimation  = nullptr;
};