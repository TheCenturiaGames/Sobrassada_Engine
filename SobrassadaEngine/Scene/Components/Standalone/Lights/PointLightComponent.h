#pragma once

#include "../LightComponent.h"

#include "rapidjson/document.h"

class PointLightComponent : public LightComponent
{
  public:
    PointLightComponent(UID uid, GameObject* parent);
    PointLightComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~PointLightComponent() override;

    void Init() override;

    void Render(float deltaTime = 0) override;
    void RenderEditorInspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    float GetRange() const { return range; }

  private:
    float range;
    int gizmosMode;
};
