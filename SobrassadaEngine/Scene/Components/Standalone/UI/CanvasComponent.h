#pragma once

#include "Component.h"

class Transform2DComponent;

class CanvasComponent : public Component
{
  public:
    CanvasComponent(UID uid, GameObject* parent);
    CanvasComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~CanvasComponent();

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* otherComponent) override;
    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;

    bool IsInWorldSpaceEditor() const { return isInWorldSpaceEditor; }
    bool IsInWorldSpaceGame() const { return isInWorldSpaceGame; }

  private:
    Transform2DComponent* transform2D = nullptr;

    bool isInWorldSpaceEditor = false;
    bool isInWorldSpaceGame = true;
};