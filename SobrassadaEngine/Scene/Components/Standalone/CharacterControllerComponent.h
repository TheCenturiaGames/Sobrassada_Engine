#pragma once
#include "Component.h"

//enum class MovementType
//{
//	FORWARD = 0,
//	BACKWARD
//};

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

	float3 GetTargetDirection() { return targetDirection; }
	float  GetSpeed() { return speed; }

	//void SetMovementType(MovementType newMovement) { currentMovement = newMovement; }
    void SetTargetDirection(float3 newTargetDir) { targetDirection = newTargetDir; }
    void SetSpeed(float newSpeed) { speed = newSpeed; }

private:
    void Move(const float3& direction, float deltaTime);
    void Rotate(float rotationDir, float deltaTime);
    void HandleInput(float deltaTime);

private:

	bool isPlayerInput = false;
	float3 targetDirection;

	float speed;

	bool useRad;

	//MovementType currentMovement;

	float maxLinearSpeed;
    float maxAngularSpeed;



};
