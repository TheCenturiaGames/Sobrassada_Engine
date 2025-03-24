#pragma once

#include "Component.h"

#include "Math/float3.h"

namespace TextManager
{
    struct FontData;
}

class UILabelComponent : public Component
{
  public:
    UILabelComponent(UID uid, GameObject* parent);
    ~UILabelComponent();

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;
    void Clone(const Component* otherComponent) override;

  private:
    void InitBuffers();

  private:
    TextManager::FontData* fontData;

    std::string text;
    int fontSize = 48;
    float3 fontColor;

    unsigned int vbo = 0;
    unsigned int vao = 0;
};