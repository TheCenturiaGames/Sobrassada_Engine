#pragma once

#include "Component.h"

#include "Math/float3.h"

class btRigidBody;
class GameObject;

class CubeColliderComponent : public Component
{
  public:
    CubeColliderComponent(UID uid, GameObject* parent)
        : Component(uid, parent, "Cube Collider", COMPONENT_CUBE_COLLIDER) {};
    ~CubeColliderComponent() override;

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void RenderEditorInspector() override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;

  private:
    btRigidBody* rigidBody = nullptr;

    bool freezeRotation    = false;
    float mass             = 0.f;
    float3 size            = float3::one;
    float3 centrerOffset   = float3::zero;
};
