#pragma once

#include "Module.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

constexpr float DEFAULT_GRAVITY = -9.81f;

class PhysicsModule : public Module
{
  public:
    PhysicsModule()  = default;
    ~PhysicsModule() = default;

    bool Init() override;
    update_status PreUpdate(float deltaTime) override;
    update_status Render(float deltaTime) override;
    update_status PostUpdate(float deltaTime) override;
    bool ShutDown() override;

    float GetGravity() const { return gravity; }

    void SetGravity(float newGravity)
    {
        gravity       = newGravity;
        updateGravity = true;
    };

  private:
    void DeleteWorld();

  private:
    float gravity                                           = DEFAULT_GRAVITY;
    bool updateGravity                                      = false;

    btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
    btCollisionDispatcher* dispatcher                       = nullptr;
    btBroadphaseInterface* broadPhase                       = nullptr;
    btSequentialImpulseConstraintSolver* solver             = nullptr;
    btDiscreteDynamicsWorld* dynamicsWorld                  = nullptr;
};