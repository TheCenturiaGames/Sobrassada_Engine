#include "CubeColliderComponent.h"

#include "Application.h"
#include "PhysicsModule.h"
#include "GameObject.h"

CubeColliderComponent::CubeColliderComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Cube Collider", COMPONENT_CUBE_COLLIDER)
{
    App->GetPhysicsModule()->CreateCubeRigidBody(this);
    centrerOffset = parent->GetPosition();
    centrerRotation = parent->GetRotation();
}


CubeColliderComponent::~CubeColliderComponent()
{
    App->GetPhysicsModule()->DeleteCubeRigidBody(this);
}

void CubeColliderComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
}

void CubeColliderComponent::Clone(const Component* other)
{
}

void CubeColliderComponent::RenderEditorInspector()
{
}

void CubeColliderComponent::Update(float deltaTime)
{
}

void CubeColliderComponent::Render(float deltaTime)
{
}
