#pragma once

#include "Component.h"

#include <vector>

namespace Math
{
    class float2;
}

class Transform2DComponent;
class GameObject;
class ButtonComponent;

class CanvasComponent : public Component
{
  public:
    static const ComponentType STATIC_TYPE = ComponentType::COMPONENT_CANVAS;

    CanvasComponent(UID uid, GameObject* parent);
    CanvasComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~CanvasComponent() override;

    void Init() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;
    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderDebug(float deltaTime) override;
    void RenderEditorInspector() override;

    void RenderUI();
    void OnWindowResize(const float width, const float height);

    void UpdateChildren();
    void UpdateMousePosition(const float2& mousePos);
    void OnMouseButtonPressed() const;
    void OnMouseButtonReleased() const;

    bool IsInWorldSpaceEditor() const { return isInWorldSpaceEditor; }
    bool IsInWorldSpaceGame() const { return isInWorldSpaceGame; }
    float GetWidth() const { return width; }
    float GetHeight() const { return height; }

  private:
    float width               = SCREEN_WIDTH;
    float height              = SCREEN_HEIGHT;
    bool isInWorldSpaceEditor = false;
    bool isInWorldSpaceGame   = true;

    std::vector<const GameObject*> sortedChildren;
    ButtonComponent* hoveredButton = nullptr;
};