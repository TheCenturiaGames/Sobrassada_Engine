#include "Transform2DComponent.h"

Transform2DComponent::Transform2DComponent(UID uid, GameObject* parent)
    : height(50), width(50), x(0), y(0), Component(uid, parent, "Transform 2D", COMPONENT_TRANSFORM_2D)
{
}

Transform2DComponent::~Transform2DComponent()
{
}

void Transform2DComponent::Update(float deltaTime)
{
}

void Transform2DComponent::Render(float deltaTime)
{
}

void Transform2DComponent::Clone(const Component* otherComponent)
{
}
