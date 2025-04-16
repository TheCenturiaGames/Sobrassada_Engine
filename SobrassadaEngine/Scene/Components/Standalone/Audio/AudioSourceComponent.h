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

    void EmitEvent(const std::string& event) const;
    void SetVolume(const float newVolume);
    void SetPitch(const float newPitch);
    void SetSpatialization(const float newSpatialization);

  private:
    void SetInitValues() const;

    char defaultEvent[64];
    float volume = 1;
    float pitch = 0.5f;
    float spatialization = 0;
};
