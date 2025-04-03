#pragma once

#include "ComponentUtils.h"
#include "Module.h"

#include <bitset>
#include <vector>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class BulletDebugDraw;
class btRigidBody;

class CubeColliderComponent;

constexpr float DEFAULT_GRAVITY = -9.81f;

typedef std::bitset<sizeof(ColliderLayerStrings) / sizeof(char*)> LayerBitset;

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

    //TODO READ FROM CONFIG FILE
    void LoadLayerData();
    void EmptyWorld();
    void RebuildWorld();

    void CreateCubeRigidBody(CubeColliderComponent* colliderComponent);
    void UpdateCubeRigidBody(CubeColliderComponent* colliderComponent);
    void DeleteCubeRigidBody(CubeColliderComponent* colliderComponent);

    float GetGravity() const { return gravity; }
    std::vector<LayerBitset>& GetLayerConfig() { return colliderLayerConfig; }


    void SetGravity(float newGravity)
    {
        gravity       = newGravity;
        updateGravity = true;
    };


  private:
    // TODO UPDATE WITH CHANNELS
    void AddRigidBody(btRigidBody* rigidBody, ColliderType colliderType, ColliderLayer layerType);

  private:
    float gravity                                           = DEFAULT_GRAVITY;
    bool updateGravity                                      = true;

    btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
    btCollisionDispatcher* dispatcher                       = nullptr;
    btBroadphaseInterface* broadPhase                       = nullptr;
    btSequentialImpulseConstraintSolver* solver             = nullptr;
    btDiscreteDynamicsWorld* dynamicsWorld                  = nullptr;

    std::vector<btRigidBody*> bodiesToRemove;

    BulletDebugDraw* debugDraw = nullptr;
    std::vector<LayerBitset> colliderLayerConfig;
};