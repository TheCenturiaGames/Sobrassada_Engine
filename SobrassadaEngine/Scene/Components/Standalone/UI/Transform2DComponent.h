#pragma once

#include "Component.h"
#include "Math/float2.h"

class CanvasComponent;

class Transform2DComponent : public Component
{
  public:
    Transform2DComponent(UID uid, GameObject* parent);
    Transform2DComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~Transform2DComponent();

    void Init() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;

    void UpdateParentTransform();
    void OnTransform3DUpdated(const float4x4& transform3D);

    float2 GetRenderingPosition() const;

    CanvasComponent* GetParentCanvas() const { return parentCanvas; }

  private:
    void GetCanvas();
    bool IsRootTransform2D() const;

  public:
    float2 position;
    float2 size;
    float2 pivot;
    float2 anchorsX;
    float2 anchorsY;

  private:
    CanvasComponent* parentCanvas;
};