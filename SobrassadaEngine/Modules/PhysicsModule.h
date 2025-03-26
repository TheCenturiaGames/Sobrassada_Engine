#pragma once

#include "Module.h"

// TODO REMOVE, JUST FOR TESTING
#include <vector>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class BulletDebugDraw;

constexpr float DEFAULT_GRAVITY = -9.81f;

// TODO REMOVE, JUST FOR TESTING
class btCollisionShape;

class PhysicsModule : public Module
{
  public:
    PhysicsModule();
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


    void CreateWorld();
    void DeleteWorld();
  private:

  private:
    float gravity                                           = DEFAULT_GRAVITY;
    bool updateGravity                                      = true;

    btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
    btCollisionDispatcher* dispatcher                       = nullptr;
    btBroadphaseInterface* broadPhase                       = nullptr;
    btSequentialImpulseConstraintSolver* solver             = nullptr;
    btDiscreteDynamicsWorld* dynamicsWorld                  = nullptr;

    BulletDebugDraw* debugDraw                              = nullptr;

    // TODO REMOVE, JUST FOR TESTING
    std::vector<btCollisionShape*> collisionShapes;
};