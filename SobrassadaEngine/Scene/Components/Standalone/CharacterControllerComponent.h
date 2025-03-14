#pragma once
#include "Component.h"

class CharacterControllerComponent : public Component
{

public:

	CharacterControllerComponent(UID uid, UID uidParent);
    CharacterControllerComponent(const rapidjson::Value& initialState);

	~CharacterControllerComponent();

	void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;

	void Update() override;
    void Render() override;
	void RenderEditorInspector() override;

	float3 GetTargetDirection() const { return targetDirection; }
	float  GetSpeed() const{ return speed; }

    void SetTargetDirection(float3 newTargetDir) { targetDirection = newTargetDir; }
    void SetSpeed(float newSpeed) { speed = newSpeed; }

private:
    void Move(const float3& direction, float deltaTime);
    void Rotate(float rotationDir, float deltaTime);
    void HandleInput(float deltaTime);

private:

	float3 targetDirection;

	float speed;
    float maxLinearSpeed;
    float maxAngularSpeed;

	bool useRad;

};
