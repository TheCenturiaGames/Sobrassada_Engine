#pragma once
#include "Globals.h"

class ResourceAnimation;


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
    ResourceAnimation* GetCurrentAnimation() { return animation; }
    float GetTime() const { return currentTime; }

    void SetAnimationResource(ResourceAnimation* anim) { animation = anim; }
    void SetPlaybackSpeed(float speed) { playbackSpeed = speed; }
    void SetTime(float time) { currentTime = time; }

    bool IsPlaying() const { return playAnimation; }

  private:
    Quat Interpolate(Quat& first, Quat& second, float lambda);

  private: 

    UID resource;
    float currentTime            = 0;
    bool loop                    = false;
    bool playAnimation           = false;
    float playbackSpeed          = 1.0f;
    ResourceAnimation* animation = nullptr;
};