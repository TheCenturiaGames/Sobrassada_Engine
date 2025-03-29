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

    void UpdateParentTransform();
    void OnTransform3DUpdated(const float4x4& transform3D);

    float2 GetGlobalPosition() const;
    float2 GetStartPos() const;

  public: 
    float2 position;
    float2 size;

    float2 pivot;
};