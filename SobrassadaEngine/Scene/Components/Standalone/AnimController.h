#pragma once
#include "AnimationComponent.h"
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
    void Resume(){ playAnimation = true; }

    void GetTransform(const std::string& nodeName, float3& pos, Quat& rot);
    ResourceAnimation* GetCurrentAnimation() const { return currentAnimation; }
    float GetTime() const { return currentTime; }

    void SetTargetAnimationResource(UID uid, unsigned transitionTime);
    void SetPlaybackSpeed(float speed) { playbackSpeed = speed; }
    void SetTime(float time) { currentTime = time; }

    bool IsPlaying() const { return playAnimation; }

  private:
    void SetAnimationResource(ResourceAnimation* anim) { currentAnimation = anim; }
    
    Quat Interpolate(Quat& first, Quat& second, float lambda);

    void GetChannelPosition(const Channel* animChannel, float3& pos, float time) const;
    void GetChannelRotation(Channel* animChannel, Quat& rot, float time);

  private: 

    UID resource;
    float currentTime            = 0;
    bool loop                    = false;
    bool playAnimation           = false;
    float playbackSpeed          = 1.0f;
    float transitionTime      = 0;
    float fadeTime               = 0;
    float currentTargetTime      = 0;
    
    ResourceAnimation* currentAnimation = nullptr;
    ResourceAnimation* targetAnimation = nullptr;
};