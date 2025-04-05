#pragma once

#include "Component.h"
#include "Math/float2.h"
#include "Math/float4.h"

class CanvasComponent;

class Transform2DComponent : public Component
{
  public:
    Transform2DComponent(UID uid, GameObject* parent);
    Transform2DComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~Transform2DComponent() override;

    void Init() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;

    void UpdateParent3DTransform();
    void OnTransform3DUpdated(const float4x4& transform3D);

    float2 GetRenderingPosition() const;
    float2 GetGlobalPosition() const;
    void AddChildTransform(Transform2DComponent* newChild) { childTransforms.push_back(newChild); }
    CanvasComponent* GetParentCanvas() const { return parentCanvas; }

  private:
    void GetCanvas();
    bool IsRootTransform2D() const;

    float GetAnchorXPos(const float anchor) const;
    float GetAnchorYPos(const float anchor) const;

    void OnAnchorsUpdated();
    void OnSizeChanged();
    void UpdateHorizontalMargins();
    void UpdateVerticalMargins();

    void OnLeftMarginChanged();
    void OnRightMarginChanged();
    void OnTopMarginChanged();
    void OnBottomMarginChanged();

  public:
    float2 position;
    float2 size;
    float2 pivot;
    float2 anchorsX;
    float2 anchorsY;

  private:
    CanvasComponent* parentCanvas;
    Transform2DComponent* parentTransform;
    std::vector<Transform2DComponent*> childTransforms;

    bool transform2DUpdated;
    float4 previousMargins;
    float4 margins;
};