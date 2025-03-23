#pragma once

#include "UIWidgetComponent.h"

namespace TextManager
{
    struct FontData;
}

class UILabelComponent : public UIWidgetComponent
{
  public:
    UILabelComponent(UID uid, GameObject* parent);
    ~UILabelComponent();

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void Clone(const Component* otherComponent) override;

  private:
    void InitBuffers();
    void ClearBuffers();

  private:
    TextManager::FontData* fontData;
    std::string text;

    unsigned int fontSize = 48;
    unsigned int vbo = 0;
    unsigned int vao = 0;
};