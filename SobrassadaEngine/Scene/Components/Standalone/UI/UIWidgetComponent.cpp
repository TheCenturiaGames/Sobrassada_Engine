#include "UIWidgetComponent.h"

UIWidgetComponent::UIWidgetComponent(UID uid, GameObject* parent, const char* uiName, ComponentType widgetType)
    : Component(uid, parent, uiName, widgetType)
{
}

UIWidgetComponent::~UIWidgetComponent()
{
}