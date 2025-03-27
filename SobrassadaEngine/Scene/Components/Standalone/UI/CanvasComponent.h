#pragma once

#include "Component.h"

class Transform2DComponent;

class CanvasComponent : public Component
{
  public:
    CanvasComponent(UID uid, GameObject* parent);
    CanvasComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~CanvasComponent();

    void Init() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;
    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;

    bool IsInWorldSpaceEditor() const { return isInWorldSpaceEditor; }
    bool IsInWorldSpaceGame() const { return isInWorldSpaceGame; }

  private:
    float width               = SCREEN_WIDTH;
    float height              = SCREEN_HEIGHT;
    bool isInWorldSpaceEditor = false;
    bool isInWorldSpaceGame   = true;
};