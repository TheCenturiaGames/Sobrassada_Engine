#include "AudioListenerComponent.h"

#include "Application.h"
#include "AudioModule.h"

#include "ImGui.h"

AudioListenerComponent::AudioListenerComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Audio Listener", COMPONENT_AUDIO_LISTENER)
{
}

AudioListenerComponent::AudioListenerComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
}

AudioListenerComponent::~AudioListenerComponent()
{
}

void AudioListenerComponent::Init()
{
}

void AudioListenerComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);
}

void AudioListenerComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_AUDIO_LISTENER)
    {
        const AudioListenerComponent* otherAudioListener = static_cast<const AudioListenerComponent*>(other);
        enabled                                          = otherAudioListener->enabled;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void AudioListenerComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
    }
}
