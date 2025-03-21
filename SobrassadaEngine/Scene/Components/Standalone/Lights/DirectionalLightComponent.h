#pragma once

#include "../LightComponent.h"

#include <Libs/rapidjson/document.h>

class DirectionalLightComponent : public LightComponent
{
  public:
    DirectionalLightComponent(UID uid, GameObject* parent);
    DirectionalLightComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~DirectionalLightComponent() override;

    void Init() override;

    void Render(float deltaTime = 0) override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    const float3 GetDirection() const;
};