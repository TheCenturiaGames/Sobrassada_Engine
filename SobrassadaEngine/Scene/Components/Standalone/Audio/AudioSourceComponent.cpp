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
    const char* textPtr = initialState["Text"].GetString();
    strcpy_s(defaultEvent, sizeof(defaultEvent), textPtr);

    volume         = initialState["Volume"].GetFloat();
    pitch          = initialState["Pitch"].GetFloat();
    spatialization = initialState["Spatialization"].GetFloat();
}

AudioSourceComponent::~AudioSourceComponent()
{
    App->GetAudioModule()->RemoveAudioSource(this);
}

void AudioSourceComponent::Init()
{
    App->GetAudioModule()->AddAudioSource(this);
    SetInitValues();
}

void AudioSourceComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("DefaultEvent", defaultEvent, allocator);
    targetState.AddMember("Volume", volume, allocator);
    targetState.AddMember("Pitch", pitch, allocator);
    targetState.AddMember("Spatialization", spatialization, allocator);
}

void AudioSourceComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_AUDIO_SOURCE)
    {
        const AudioSourceComponent* otherAudioSource = static_cast<const AudioSourceComponent*>(other);
        enabled                                      = otherAudioSource->enabled;

        const char* textPtr                          = otherAudioSource->defaultEvent;
        strcpy_s(defaultEvent, sizeof(defaultEvent), textPtr);

        volume         = otherAudioSource->volume;
        pitch          = otherAudioSource->pitch;
        spatialization = otherAudioSource->spatialization;

        SetInitValues();
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
        EmitEvent(defaultEvent);
    }
}

void AudioSourceComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::InputText("Default event", &defaultEvent[0], sizeof(defaultEvent));
        if (ImGui::DragFloat("Volume", &volume, 0.01f, 0, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp)) SetVolume(volume);
        if (ImGui::DragFloat("Pitch", &pitch, 0.01f, 0, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp)) SetPitch(pitch);
        if (ImGui::DragFloat("3D Spatialization", &spatialization, 0.01f, 0, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp))
            SetSpatialization(spatialization);
    }
}

void AudioSourceComponent::EmitEvent(const std::string& event) const
{
    AK::SoundEngine::PostEvent(event.c_str(), (AkGameObjectID)parent->GetUID());
}

void AudioSourceComponent::SetVolume(const float newVolume)
{
    volume = newVolume;
    AK::SoundEngine::SetRTPCValue("Volume", volume, parent->GetUID());
}

void AudioSourceComponent::SetPitch(const float newPitch)
{
    pitch = newPitch;
    AK::SoundEngine::SetRTPCValue("Pitch", pitch, parent->GetUID());
}

void AudioSourceComponent::SetSpatialization(const float newSpatialization)
{
    spatialization = newSpatialization;
    AK::SoundEngine::SetRTPCValue("Spatialization", spatialization, parent->GetUID());
}

void AudioSourceComponent::SetInitValues() const
{
    AK::SoundEngine::SetRTPCValue("Volume", volume, parent->GetUID());
    AK::SoundEngine::SetRTPCValue("Pitch", pitch, parent->GetUID());
    AK::SoundEngine::SetRTPCValue("Spatialization", spatialization, parent->GetUID());
}