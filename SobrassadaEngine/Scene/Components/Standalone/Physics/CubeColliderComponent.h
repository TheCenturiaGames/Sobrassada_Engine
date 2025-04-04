#pragma once

#include "BulletMotionState.h"
#include "BulletUserPointer.h"
#include "Component.h"
#include "Delegate.h"

#include "Math/float3.h"
#include <bitset>

class btRigidBody;
class GameObject;

class CubeColliderComponent : public Component
{
  public:
    CubeColliderComponent(UID uid, GameObject* parent);
    CubeColliderComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~CubeColliderComponent() override;

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void RenderEditorInspector() override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;

    void ParentUpdated() override;

    void OnCollision(GameObject* otherObject, float3 collisionNormal);

  public:
    bool generateCallback         = true;
    bool freezeRotation           = false;
    float mass                    = 0.f;
    float3 centerOffset           = float3::zero;
    float3 centerRotation         = float3::zero;
    float3 size                   = float3::one;
    ColliderType colliderType     = ColliderType::STATIC;

    btRigidBody* rigidBody        = nullptr;
    BulletMotionState motionState = BulletMotionState(nullptr, float3::zero, float3::zero, false);
    CollisionDelegate onCollissionCallback;
    BulletUserPointer userPointer = BulletUserPointer(this, &onCollissionCallback, generateCallback);
    ColliderLayer layer           = ColliderLayer::WORLD_OBJECTS;
};
