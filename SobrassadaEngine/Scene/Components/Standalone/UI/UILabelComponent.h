#pragma once

#include "UIWidgetComponent.h"

class UILabelComponent : public UIWidgetComponent
{
  public:
    UILabelComponent(UID uid, GameObject* parent);
    ~UILabelComponent();

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void Clone(const Component* otherComponent) override;

  private:
    std::string text;
};