#pragma once

#include "Component.h"

class Transform2DComponent;
class CanvasComponent;

class ImageComponent : public Component
{
  public:
    ImageComponent(UID uid, GameObject* parent);
    ImageComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~ImageComponent() override;

    void Init() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;
    void Update(float deltaTime) override {};
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;

    void RenderUI() const;

  private:
    Transform2DComponent* transform2D;
    CanvasComponent* parentCanvas;
};
