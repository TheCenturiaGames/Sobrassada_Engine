#pragma once

#include "Module.h"
#include "Standalone/Audio/AudioSourceComponent.h"
#include "Standalone/Audio/AudioListenerComponent.h"

#include <AkFilePackageLowLevelIODeferred.h>
#include <vector>

class AudioModule : public Module
{
  public:
    AudioModule();
    ~AudioModule() override;

    bool Init() override;
    update_status Update(float deltaTime) override;
    bool ShutDown() override;

    void musicvol(int volume) { music = volume; };
    void soundvol(int volume) { sound = volume; };
    void voicevol(int volume) { voice = volume; };

    void AddAudioSource(AudioSourceComponent* newSource);
    void RemoveAudioSource(AudioSourceComponent* newSource);

    bool AddAudioListener(AudioListenerComponent* newListener);
    void RemoveAudioListener(AudioListenerComponent* newListener);

  private:
    CAkFilePackageLowLevelIODeferred g_lowLevelIO;
    int music = 0;
    int sound = 0;
    int voice = 0;
    int g_envMAP[255];

    std::vector<AudioSourceComponent*> sources;
    AudioListenerComponent* listener;
};
