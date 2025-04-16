#pragma once

#include "Component.h"

#include "Math/float3.h"

class dtNavMeshQuery;

using dtPolyRef = unsigned int;

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

    void AdjustHeightToNavMesh(float3& currentPos);
    void Move(const float3& direction, float deltaTime);
    void Rotate(float rotationDirection, float deltaTime);
    void HandleInput(float deltaTime);

    const float3& GetTargetDirection() const { return targetDirection; }
    const float3& GetLastPosition() const { return lastPosition; }
    const float& GetSpeed() const { return speed; }

    void SetTargetDirection(float3 newTargetDirection) { targetDirection = newTargetDirection; }
    void SetSpeed(float newSpeed) { speed = newSpeed; }

  private:
    float3 targetDirection       = float3::zero;
    float3 lastPosition          = float3::zero;

    float speed                  = 0.0f;
    float maxLinearSpeed         = 0.0f;
    float maxAngularSpeed        = 0.0f;

    bool isRadians               = false;

    dtNavMeshQuery* navMeshQuery = nullptr;
    dtPolyRef currentPolyRef     = 0;

    float gravity                = -9.81f;
    float verticalSpeed          = 0.0f;
    float maxFallSpeed           = -20.0f;
};
