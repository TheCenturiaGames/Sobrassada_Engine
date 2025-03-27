#pragma once

#include "Component.h"
#include "Math/float2.h"

class Transform2DComponent : public Component
{
  public:
    Transform2DComponent(UID uid, GameObject* parent);
    Transform2DComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~Transform2DComponent();

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;

    const float2& GetPosition() const { return position; }
    const float2& GetSize() const { return size; }
    void SetPosition(const float x, const float y) { position = float2(x, y); }
    void SetSize(const float width, const float height) { size = float2(width, height); }

  private: 
    float2 position;
    float2 size;
};