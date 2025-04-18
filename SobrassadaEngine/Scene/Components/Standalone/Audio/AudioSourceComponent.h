#pragma once

#include "Application.h"
#include "Component.h"
#include "WwiseIDs.h"

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

    // More efficient to use the IDs, but both exist in case it is needed to use the string variant in some scenario
    void EmitEvent(const WwiseID event) const;
    void EmitEvent(const std::string& event) const;
    void SetRTPCValue(const WwiseID parameterID, const float value);
    void SetRTPCValue(const std::string& parameterName, const float value);
    void SetSwitch(const WwiseID switchGroupID, const WwiseID activeSwitchID);
    void SetSwitch(const std::string& switchGroupName, const std::string& activeSwitchName);

    void SetDefaultEvent(const WwiseID event);
    void SetVolume(const float newVolume);
    void SetPitch(const float newPitch);
    void SetSpatialization(const float newSpatialization);

  private:
    void SetInitValues() const;

    WwiseID defaultEvent;
    float volume         = 1;
    float pitch          = 0.5f;
    float spatialization = 0;
};
