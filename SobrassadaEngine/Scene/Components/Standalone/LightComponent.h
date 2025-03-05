#pragma once

#include "Application.h"
#include "Component.h"

#include "Math/float3.h"

class LightComponent : public Component
{
  public:
    LightComponent(
        UID uid, UID uidParent, UID uidRoot, const char* uiName, const ComponentType lightType,
        const float4x4& parentGlobalTransform
    );
    LightComponent(const rapidjson::Value& initialState);

    virtual void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;

    virtual void RenderEditorInspector() override;
    void Update() override;
    virtual void Render() override;

    float GetIntensity() const { return intensity; }
    float3 GetColor() const { return color; }

    void SetIntensity(const float newIntensity) { intensity = newIntensity; }
    void SetColor(const float3& newColor) { color = newColor; }

  protected:
    float intensity;
    float3 color;
    bool drawGizmos;
};
