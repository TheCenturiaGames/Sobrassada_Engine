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
    void RenderDebug(float deltaTime) override;

    void ParentUpdated() override;

    void SOBRASADA_API_ENGINE OnCollision(GameObject* otherObject, float3 collisionNormal);

    void SOBRASADA_API_ENGINE DeleteRigidBody();

  private:
    void CalculateCollider();

  public:
    bool generateCallback         = true;
    bool freezeRotation           = false;
    bool fitToSize                = false;
    float mass                    = 1.f;
    float3 centerOffset           = float3::zero;
    float3 centerRotation         = float3::zero;
    float radius                  = 1.f;
    float length                  = 1.f;
    ColliderType colliderType     = ColliderType::DYNAMIC;

    btRigidBody* rigidBody        = nullptr;
    BulletMotionState motionState = BulletMotionState(nullptr, float3::zero, float3::zero, false);
    CollisionDelegate onCollissionCallback;
    BulletUserPointer userPointer = BulletUserPointer(this, &onCollissionCallback, generateCallback);
    ColliderLayer layer           = ColliderLayer::WORLD_OBJECTS;
};
