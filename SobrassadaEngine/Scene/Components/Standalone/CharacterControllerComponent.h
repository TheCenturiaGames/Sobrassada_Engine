#pragma once

#include "Component.h"

class CharacterControllerComponent : public Component
{

  public:
    CharacterControllerComponent(UID uid, GameObject* parent);
    CharacterControllerComponent(const rapidjson::Value& initialState, GameObject* parent);

    ~CharacterControllerComponent() override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void Move(const float3& direction, float deltaTime) const;
    void Rotate(float rotationDirection, float deltaTime);
    void HandleInput(float deltaTime);

    const float3& GetTargetDirection() const { return targetDirection; }
    const float& GetSpeed() const { return speed; }

    void SetTargetDirection(float3 newTargetDirection) { targetDirection = newTargetDirection; }
    void SetSpeed(float newSpeed) { speed = newSpeed; }

  private:
    float3 targetDirection;

    float speed;
    float maxLinearSpeed;
    float maxAngularSpeed;

    bool isRadians;
};
