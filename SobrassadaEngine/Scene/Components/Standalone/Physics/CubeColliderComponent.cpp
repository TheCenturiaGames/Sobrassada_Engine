#include "CubeColliderComponent.h"

CubeColliderComponent::CubeColliderComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Cube Collider", COMPONENT_CUBE_COLLIDER)
{
    motionState = BulletMotionState(this, float3::zero, float3::zero, false);
}


CubeColliderComponent::~CubeColliderComponent()
{
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
