#include "UIWidgetComponent.h"

UIWidgetComponent::UIWidgetComponent(UID uid, GameObject* parent, const char* uiName, ComponentType widgetType)
    : Component(uid, parent, uiName, widgetType)
{
    if (parent->GetComponentByType(COMPONENT_TRANSFORM_2D) == nullptr) parent->CreateComponent(COMPONENT_TRANSFORM_2D);
}

UIWidgetComponent::~UIWidgetComponent()
{
}