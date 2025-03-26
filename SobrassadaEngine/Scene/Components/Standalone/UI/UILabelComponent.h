#pragma once

#include "Component.h"

#include "Math/float3.h"

namespace TextManager
{
    struct FontData;
}
class Transform2DComponent;
class CanvasComponent;

class UILabelComponent : public Component
{
  public:
    UILabelComponent(UID uid, GameObject* parent);
    UILabelComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~UILabelComponent();

    void Init() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* otherComponent) override;
    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;

  private:
    void InitBuffers();

  private:
    Transform2DComponent* transform2D;
    TextManager::FontData* fontData;

    char text[512];
    int fontSize = 48;
    float3 fontColor;

    unsigned int vbo = 0;
    unsigned int vao = 0;

    CanvasComponent* parentCanvas;
};