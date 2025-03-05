#pragma once

#include "../LightComponent.h"

#include <Libs/rapidjson/document.h>

class SpotLightComponent : public LightComponent
{
  public:
    SpotLightComponent(UID uid, UID uidParent, UID uidRoot, const float4x4 &parentGlobalTransform);
    SpotLightComponent(const rapidjson::Value& initialState);
    ~SpotLightComponent();

    void Render() override;
    void RenderEditorInspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

    float3 GetDirection() const;
    float GetRange() const { return range; }
    float GetInnerAngle() const { return innerAngle; }
    float GetOuterAngle() const { return outerAngle; }

  private:
    float range;
    float innerAngle;
    float outerAngle;
};