#include "UILabelComponent.h"

UILabelComponent::UILabelComponent(UID uid, GameObject* parent)
    : text("New text"), UIWidgetComponent(uid, parent, "Label", COMPONENT_LABEL)
{
}

UILabelComponent::~UILabelComponent()
{
}

void UILabelComponent::Update(float deltaTime)
{
}

void UILabelComponent::Render(float deltaTime)
{
}

void UILabelComponent::Clone(const Component* otherComponent)
{
}