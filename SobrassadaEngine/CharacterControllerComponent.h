#pragma once
#include "Component.h"

enum class MovementType
{
	FORWARD = 0,
	BACKWARD
};

class CharacterControllerComponent : public Component
{

public:

	CharacterControllerComponent(UID uid, UID uidParent);
	
	/*CharacterControllerComponent(
		UID uid, UID uidParent,UID uidRoot, 
		const char* initName, int type,
		const float4x4& parentGlobalTransform
	);*/

	~CharacterControllerComponent();

	void Update() override;

	void Render() override;
	void RenderEditorInspector() override;

	float3 GetTargetDirection() { return targetDirection; }
	float  GetSpeed() { return speed; }

	void SetMovementType(MovementType newMovement) { currentMovement = newMovement; }
    void SetTargetDirection(float3 newTargetDir) { targetDirection = newTargetDir; }
    void SetSpeed(float newSpeed) { speed = newSpeed; }

private:
    void Move(float deltaTime);
	void Rotate(float deltaTime);

private:

	bool isPlayerInput = false;
	float3 targetDirection;

	float speed;

	MovementType currentMovement;

	float maxLinearSpeed;
    float maxAngularSpeed;



};
