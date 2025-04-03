#pragma once

#include "BulletMotionState.h"
#include "BulletUserPointer.h"
#include "Component.h"
#include "Delegate.h"

#include "Math/float3.h"
#include <bitset>

class btRigidBody;
class GameObject;

class CapsuleColliderComponent : public Component
{
  public:
    CapsuleColliderComponent(UID uid, GameObject* parent);
    CapsuleColliderComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~CapsuleColliderComponent() override;

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void RenderEditorInspector() override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;

    void ParentUpdated() override;

    ColliderType GetColliderType() const { return colliderType; }
    ColliderLayer GetLayer() const { return layer; }
    const CollisionDelegate& GetCallback() const { return onCollissionCallback; }

    void OnCollision(GameObject* otherObject, float3 collisionNormal);

  public:
    bool freezeRotation           = false;
    float mass                    = 0.f;
    float3 centerOffset           = float3::zero;
    float3 centerRotation         = float3::zero;
    float radius                  = 1.f;
    float length                  = 1.f;
    ColliderType colliderType     = ColliderType::STATIC;

    btRigidBody* rigidBody        = nullptr;
    BulletMotionState motionState = BulletMotionState(nullptr, float3::zero, float3::zero, false);
    CollisionDelegate onCollissionCallback;
    BulletUserPointer userPointer = BulletUserPointer(this, &onCollissionCallback);
    ColliderLayer layer           = ColliderLayer::WORLD_OBJECTS;
};
