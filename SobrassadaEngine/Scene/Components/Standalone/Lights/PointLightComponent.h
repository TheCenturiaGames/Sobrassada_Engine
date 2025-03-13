#pragma once

#include "../LightComponent.h"

#include <Libs/rapidjson/document.h>

class PointLightComponent : public LightComponent
{
  public:
    PointLightComponent(UID uid, UID uidParent);
    PointLightComponent(const rapidjson::Value& initialState);
    ~PointLightComponent() override;

    void Render() override;
    void RenderEditorInspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    float GetRange() const { return range; }

  private:
    float range;
    int gizmosMode;
};
