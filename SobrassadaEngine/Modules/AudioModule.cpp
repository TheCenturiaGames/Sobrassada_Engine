#include "AudioModule.h"

#include "Application.h"
#include "Globals.h"
#include "ProjectModule.h"

#include <AK/IBytes.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>       // Memory Manager interface
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h> // Default memory manager
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>    // Streaming Manager
#include <AK/SpatialAudio/Common/AkSpatialAudio.h> // Spatial Audio

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>
#endif

AudioModule::AudioModule()
{
    g_envMAP[0] = 0; // hardcode environment 0 to id=0
    for (int i = 1; i < 255; i++)
    {
        g_envMAP[i] = -1;
    }
}

AudioModule::~AudioModule()
{
}

bool AudioModule::Init()
{
    AkMemSettings memSettings;
    AK::MemoryMgr::GetDefaultSettings(memSettings);

    AkStreamMgrSettings stmSettings;
    AK::StreamMgr::GetDefaultSettings(stmSettings);

    AkDeviceSettings deviceSettings;
    AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

    AkInitSettings l_InitSettings;
    AkPlatformInitSettings l_platInitSetings;
    AK::SoundEngine::GetDefaultInitSettings(l_InitSettings);
    AK::SoundEngine::GetDefaultPlatformInitSettings(l_platInitSetings);

    // Setting pool sizes for this game. Here, allow for user content;
    // every game should determine its own optimal values.
    // l_InitSettings.uDefaultPoolSize           = 2 * 1024 * 1024;
    // l_platInitSetings.uLEngineDefaultPoolSize = 4 * 1024 * 1024;

    AkMusicSettings musicInit;
    AK::MusicEngine::GetDefaultInitSettings(musicInit);

    AkSpatialAudioInitSettings settings;

    // Create and initialise an instance of our memory manager.
    if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
    {
        GLOG("Could not create the memory manager.");
        return false;
    }
    // Create and initialise an instance of the default stream manager.
    if (!AK::StreamMgr::Create(stmSettings))
    {
        GLOG("Could not create the Stream Manager");
        return false;
    }
    // Create an IO device.
    if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
    {
        GLOG("Cannot create streaming I/O device");
        return false;
    }
    // Initialize sound engine.
    if (AK::SoundEngine::Init(&l_InitSettings, &l_platInitSetings) != AK_Success)
    {
        GLOG("Cannot initialize sound engine");
        return false;
    }
    // Initialize music engine.
    if (AK::MusicEngine::Init(&musicInit) != AK_Success)
    {
        GLOG("Cannot initialize music engine");
        return false;
    }
    if (AK::SpatialAudio::Init(settings) != AK_Success)
    {
        GLOG("Could not initialize the Spatial Audio.");
        return false;
    }

    // load initialization and main soundbanks
    const std::string soundbanksPath = App->GetProjectModule()->GetLoadedProjectPath() + "Soundbanks\\Windows\\";

    const int size                   = MultiByteToWideChar(CP_UTF8, 0, soundbanksPath.c_str(), -1, nullptr, 0);
    std::wstring wSoundbanksPath(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, soundbanksPath.c_str(), -1, &wSoundbanksPath[0], size);

#ifdef AK_WIN
    g_lowLevelIO.SetBasePath(wSoundbanksPath.c_str());
#else
    g_lowLevelIO.SetBasePath(AKTEXT("soundbanks/Mac/"));
#endif
    AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));
#ifndef AK_OPTIMIZED
    // Initialize communications (not in release build!)
    AkCommSettings commSettings;
    AK::Comm::GetDefaultInitSettings(commSettings);
    if (AK::Comm::Init(commSettings) != AK_Success)
    {
        GLOG("Could not initialize communication.");
        return false;
    }
#endif

    AkBankID bankID;
    AKRESULT retValue;
    retValue = AK::SoundEngine::LoadBank(BANKNAME_INIT, bankID);
    if (retValue != AK_Success)
    {
        GLOG("Cannot initialize Init.bnk soundbank");
        // return false;
    }
    retValue = AK::SoundEngine::LoadBank(BANKNAME_MAIN, bankID);
    if (retValue != AK_Success)
    {
        GLOG("Cannot initialize main.bnk soundbank");
        // return false;
    }

    //  initialize volume parameters to sensible default values
    musicvol(255);
    soundvol(255);
    voicevol(255);

    return true;
}

update_status AudioModule::Update(float deltaTime)
{
    // more things here needed
    AK::SoundEngine::RenderAudio();
    return UPDATE_CONTINUE;
}

bool AudioModule::ShutDown()
{
#ifndef AK_OPTIMIZED
    AK::Comm::Term();
#endif
    AK::MusicEngine::Term();
    AK::SoundEngine::Term();
    g_lowLevelIO.Term();
    if (AK::IAkStreamMgr::Get()) AK::IAkStreamMgr::Get()->Destroy();
    AK::MemoryMgr::Term();
    return true;
}
