#pragma once

#include "../LightComponent.h"

#include <Libs/rapidjson/document.h>

class PointLight : public LightComponent
{
  public:
    PointLight(UID uid, UID uidParent, UID uidRoot, const Transform& parentGlobalTransform);
    PointLight(const rapidjson::Value& initialState);
    ~PointLight();

    void Render() override;
    void RenderEditorInspector() override;

    virtual void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;

    float GetRange() const { return range; }

  private:
    float range;
    int gizmosMode;
};
