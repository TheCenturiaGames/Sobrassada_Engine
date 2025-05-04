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
    void Render(float deltaTime) override {};
    void RenderDebug(float deltaTime) override;
    void RenderDebug(float deltaTime, const float3& color);
    void RenderUI();
    void OnWindowResize(const float width, const float height);

    void UpdateChildren();
    void UpdateMousePosition(const float2& mousePos);
    void OnMouseButtonPressed() const;
    void OnMouseButtonReleased() const;

    void RenderEditorInspector() override;

    bool IsInWorldSpace() const;
    float GetScreenScale() const;
    float GetWidth() const { return width; }
    float GetHeight() const { return height; }

  private:
    enum class CanvasRenderMode
    {
        ScreenSpaceOverlay,
        WorldSpace
    };

    float width                 = SCREEN_WIDTH;
    float height                = SCREEN_HEIGHT;

    float referenceWidth        = 1920.0f;
    float referenceHeight       = 1080.0f;

    CanvasRenderMode renderMode = CanvasRenderMode::ScreenSpaceOverlay;

    std::vector<const GameObject*> sortedChildren;
    ButtonComponent* hoveredButton = nullptr;
};