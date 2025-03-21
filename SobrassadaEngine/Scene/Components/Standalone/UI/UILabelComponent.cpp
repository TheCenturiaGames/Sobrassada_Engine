#include "UILabelComponent.h"

UILabelComponent::UILabelComponent(UID uid, GameObject* parent)
    : text("New text"), UIWidgetComponent(uid, parent, "Label", COMPONENT_LABEL)
{
}

UILabelComponent::~UILabelComponent()
{
}