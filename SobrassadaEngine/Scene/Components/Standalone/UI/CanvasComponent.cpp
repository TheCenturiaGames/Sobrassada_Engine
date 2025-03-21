#include "CanvasComponent.h"

CanvasComponent::CanvasComponent(UID uid, GameObject* parent) : Component(uid, parent, "Canvas", COMPONENT_CANVAS)
{
}

CanvasComponent::~CanvasComponent()
{
}

void CanvasComponent::Render(float deltaTime)
{

}