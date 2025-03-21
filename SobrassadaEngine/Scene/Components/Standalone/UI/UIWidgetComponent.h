#pragma once

#include "Component.h"
#include "Transform2DComponent.h"

#include "Math/float4.h"

struct UIStyle
{
    float4 fontColor;
    float4 backgroundColor;
    int fontSize;
};

class UIWidgetComponent : Component
{
  public:
    UIWidgetComponent(UID uid, GameObject* parent, const char* uiName, ComponentType widgetType);
    ~UIWidgetComponent();

    bool isVisible;
    UIStyle style;
};
