#include "AudioSourceComponent.h"

#include "AudioModule.h"
#include "Application.h"

#include "ImGui.h"

AudioSourceComponent::AudioSourceComponent(UID uid, GameObject* parent) : Component(uid, parent, "Audio Source", COMPONENT_AUDIO_SOURCE)
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

void AudioSourceComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        
    }
}
