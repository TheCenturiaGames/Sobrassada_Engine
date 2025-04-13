#pragma once

#include "Component.h"

#include "Math/float3.h"

class Transform2DComponent;
class CanvasComponent;
class ImageComponent;

class ButtonComponent : public Component
{
  public:
    ButtonComponent(UID uid, GameObject* parent);
    ButtonComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~ButtonComponent() override;

    void Init() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;
    void Update(float deltaTime) override {};
    void Render(float deltaTime) override {};
    void RenderEditorInspector() override;

    bool UpdateMousePosition(const float2& mousePos, bool dismiss = false);
    void OnClick();

  private:
    bool IsWithinBounds(const float2& pos);

  private:
    Transform2DComponent* transform2D;
    CanvasComponent* parentCanvas;
    ImageComponent* image;

    float3 defaultColor;
    float3 hoverColor;

    bool isHovered;
};
