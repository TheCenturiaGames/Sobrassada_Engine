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

    float2 desiredPosition;
    float smoothness = 10;
    float lookAhead = 0;
};