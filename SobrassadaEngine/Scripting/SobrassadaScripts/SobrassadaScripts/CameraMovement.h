#pragma once

#include "Script.h"

#include "Math/float2.h"

class GameObject;

class CameraMovement : public Script
{
  public:
    CameraMovement(GameObject* parent);
    virtual ~CameraMovement() noexcept override { parent = nullptr; }

    bool Init() override;
    void Update(float deltaTime) override;

  private:
    void FollowTarget(float deltaTime);

  private:
    std::string targetName;
    const GameObject* target = nullptr;

    float3 finalPosition;
    float smoothnessVelocity   = 10;

    bool mouseOffsetEnabled    = false;
    float mouseOffsetIntensity = 0;

    bool lookAheadEnabled      = false;
    float lookAheadIntensity   = 0;
    float currentLookAhead     = 0;
    float lookAheadSmoothness  = 0;
};