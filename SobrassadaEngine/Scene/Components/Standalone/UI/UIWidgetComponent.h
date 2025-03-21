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

class UIWidgetComponent : public Component
{
  public:
    UIWidgetComponent(UID uid, GameObject* parent, const char* uiName, ComponentType widgetType);
    ~UIWidgetComponent();

    virtual void Update(float deltaTime) override {};
    virtual void Render(float deltaTime) override {};
    virtual void Clone(const Component* otherComponent) = 0;

  private:
    bool isVisible;
    UIStyle style;
};
