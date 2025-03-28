#pragma once

#include "BulletMotionState.h"
#include "Component.h"

#include "Math/float3.h"

class btRigidBody;
class GameObject;

class CubeColliderComponent : public Component
{
  public:
    CubeColliderComponent(UID uid, GameObject* parent);
    ~CubeColliderComponent() override;

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void RenderEditorInspector() override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;

  private:
    btRigidBody* rigidBody        = nullptr;

    bool freezeRotation           = false;
    float mass                    = 0.f;
    float3 centrerOffset          = float3::zero;
    float3 centrerRotation        = float3::zero;
    float3 size                   = float3::one;

    BulletMotionState motionState = BulletMotionState(nullptr, float3::zero, float3::zero, false);
};
