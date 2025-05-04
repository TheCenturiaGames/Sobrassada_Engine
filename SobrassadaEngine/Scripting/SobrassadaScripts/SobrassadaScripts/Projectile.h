#pragma once

#include "Script.h"

#include "Math/float3.h"

class CubeColliderComponent;

class Projectile : public Script
{
  public:
    Projectile(GameObject* parent);
    virtual ~Projectile() noexcept override { parent = nullptr; }

    bool Init() override;
    void Update(float deltaTime) override;

    void Shoot(const float3& origin, const float3& direction);

  private:
    void Move(float deltaTime);

  private:
    CubeColliderComponent* collider = nullptr;

    float3 direction;
    float speed = 10;

    float range = 10;
    float3 startPos;
};