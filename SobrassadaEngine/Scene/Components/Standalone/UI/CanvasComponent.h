#pragma once

#include "Component.h"

class CanvasComponent : public Component
{
  public:
    CanvasComponent(UID uid, GameObject* parent);
    ~CanvasComponent();

    void Render(float deltaTime) override;
};