#pragma once

#include "Component.h"
#include "UIScaler.h"

#include <vector>

namespace Math
{
    class float2;
}

class Transform2DComponent;
class GameObject;
class ButtonComponent;
class UIScaler;


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
    bool IsInWorldSpace() const { return isInWorldSpace; }


    float GetWidth() const { return width; }
    float GetHeight() const { return height; }
    float GetUIScale() const;

    void PrintTransform2DDebugInfo() const;
    void PrintTransform2DRecursive(Transform2DComponent* t2d, int depth) const;


  private:
    float width               = SCREEN_WIDTH;
    float height              = SCREEN_HEIGHT;
    bool isInWorldSpace;


    float referenceWidth      = 1920.0f;
    float referenceHeight     = 1080.0f;

    UIScaleMode scaleMode     = UIScaleMode::MatchWidthOrHeight;
    float matchFactor         = 0.5f; // Only when scaleMode == MatchWidthOrHeight


    std::vector<const GameObject*> sortedChildren;
    ButtonComponent* hoveredButton = nullptr;
    UIScaler* uiScaler             = nullptr;
    bool inWorldSpace                 = false;
    Transform2DComponent* transform2D = nullptr;

};