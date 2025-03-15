#pragma once

#include "../LightComponent.h"

#include <Libs/rapidjson/document.h>

class SpotLightComponent : public LightComponent
{
  public:
    SpotLightComponent(UID uid, UID uidParent);
    SpotLightComponent(const rapidjson::Value& initialState);
    ~SpotLightComponent() override;

    void Render(float deltaTime = 0) override;
    void RenderEditorInspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    const float3 GetDirection();
    float GetRange() const { return range; }
    float GetInnerAngle() const { return innerAngle; }
    float GetOuterAngle() const { return outerAngle; }

  private:
    float range;
    float innerAngle;
    float outerAngle;
};