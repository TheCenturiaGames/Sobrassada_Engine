#pragma once

#include "Application.h"
#include "Component.h"

class AudioSourceComponent : public Component
{
  public:
    AudioSourceComponent(UID uid, GameObject* parent);
    AudioSourceComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~AudioSourceComponent() override;

    void Init() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override {};
    void RenderEditorInspector() override;

};
