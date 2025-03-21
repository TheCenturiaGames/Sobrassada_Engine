#pragma once

#include "UIWidgetComponent.h"

class UILabelComponent : public UIWidgetComponent
{
  public:
    UILabelComponent(UID uid, GameObject* parent);
    ~UILabelComponent();

  private:
    std::string text;
};