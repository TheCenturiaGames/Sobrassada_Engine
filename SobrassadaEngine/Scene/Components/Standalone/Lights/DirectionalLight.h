#pragma once

#include "../LightComponent.h"

#include <Libs/rapidjson/document.h>

class DirectionalLight : public LightComponent
{
  public:
    DirectionalLight(UID uid, UID uidParent, UID uidRoot, const Transform& parentGlobalTransform);
    DirectionalLight(const rapidjson::Value& initialState);
    ~DirectionalLight();

    void Render() override;
    void RenderEditorInspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

    float3 GetDirection() const;

};