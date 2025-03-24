#pragma once

#include "Component.h"

class Transform2DComponent;

class CanvasComponent : public Component
{
  public:
    CanvasComponent(UID uid, GameObject* parent);
    ~CanvasComponent();

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;
    void Clone(const Component* otherComponent) override;

  private:
    Transform2DComponent* transform2D = nullptr;

    bool isInWorldSpaceEditor = false;
    bool isInWorldSpaceGame = true;
};