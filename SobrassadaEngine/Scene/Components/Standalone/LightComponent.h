#pragma once

#include "Application.h"
#include "Component.h"

#include "Math/float3.h"

class LightComponent : public Component
{
  public:
    LightComponent(UID uid, UID uidParent, const char* uiName, ComponentType lightType);
    LightComponent(const rapidjson::Value& initialState);

    void Update() override;
    virtual void Render() override;
    virtual void RenderEditorInspector() override;
    virtual void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

    float GetIntensity() const { return intensity; }
    float3 GetColor() const { return color; }

    void SetIntensity(const float newIntensity) { intensity = newIntensity; }
    void SetColor(const float3& newColor) { color = newColor; }

  protected:
    float intensity;
    float3 color;
    bool drawGizmos;
};
