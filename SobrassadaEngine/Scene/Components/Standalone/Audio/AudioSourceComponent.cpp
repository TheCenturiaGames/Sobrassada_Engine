#include "AudioSourceComponent.h"

#include "Application.h"
#include "AudioModule.h"
#include "GameObject.h"
#include "InputModule.h" // TODO: DELETE THIS

#include "ImGui.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>

AudioSourceComponent::AudioSourceComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Audio Source", COMPONENT_AUDIO_SOURCE)
{
}

AudioSourceComponent::AudioSourceComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
}

AudioSourceComponent::~AudioSourceComponent()
{
    App->GetAudioModule()->RemoveAudioSource(this);
}

void AudioSourceComponent::Init()
{
    App->GetAudioModule()->AddAudioSource(this);
}

void AudioSourceComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);
}

void AudioSourceComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_AUDIO_SOURCE)
    {
        const AudioSourceComponent* otherAudioSource = static_cast<const AudioSourceComponent*>(other);
        enabled                                      = otherAudioSource->enabled;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void AudioSourceComponent::Update(float deltaTime)
{
    if (App->GetInputModule()->GetKeyboard()[SDL_SCANCODE_0] == KEY_DOWN)
    {
        GLOG("Play audio");
        EmitEvent("Test");
    }
}

void AudioSourceComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        if (ImGui::SliderFloat("Volume", &volume, 0, 1)) SetVolume(volume);
        if (ImGui::SliderFloat("Pitch", &pitch, 0, 1)) SetPitch(pitch);
        if (ImGui::SliderFloat("Pitch", &spatialization, 0, 1)) SetSpatialization(spatialization);
    }
}

void AudioSourceComponent::EmitEvent(const std::string& event) const
{
    AK::SoundEngine::PostEvent("Test", (AkGameObjectID)parent->GetUID());
}

void AudioSourceComponent::SetVolume(const float newVolume)
{
    volume = newVolume;
    AK::SoundEngine::SetRTPCValue("Volume", volume, parent->GetUID());
}

void AudioSourceComponent::SetPitch(const float newPitch)
{
    pitch = newPitch;
    AK::SoundEngine::SetRTPCValue("Pitch", volume, parent->GetUID());
}

void AudioSourceComponent::SetSpatialization(const float newSpatialization)
{
    spatialization = newSpatialization;
    AK::SoundEngine::SetRTPCValue("Spatialization", spatialization, parent->GetUID());
}