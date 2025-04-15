#pragma once

#include "Application.h"
#include "Component.h"

class AudioListenerComponent : public Component
{
  public:
    AudioListenerComponent(UID uid, GameObject* parent);
    AudioListenerComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~AudioListenerComponent() override;

    void Init() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void Update(float deltaTime) override {};
    void Render(float deltaTime) override {};
    void RenderEditorInspector() override;

};
