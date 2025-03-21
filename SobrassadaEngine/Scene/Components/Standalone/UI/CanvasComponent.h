#pragma once

#include "Component.h"

class CanvasComponent : public Component
{
  public:
    CanvasComponent(UID uid, GameObject* parent);
    ~CanvasComponent();

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void Clone(const Component* otherComponent) override;
};