#pragma once

#include "../LightComponent.h"

#include <Libs/rapidjson/document.h>

class DirectionalLightComponent : public LightComponent
{
  public:
    DirectionalLightComponent(UID uid, UID uidParent, UID uidRoot, const Transform &parentGlobalTransform);
    DirectionalLightComponent(const rapidjson::Value& initialState);
    ~DirectionalLightComponent();

    void Render() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

    float3 GetDirection() const;

};