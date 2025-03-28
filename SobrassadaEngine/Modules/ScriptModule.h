#pragma once
#include "Globals.h"
#include "Module.h"
#include <filesystem>
#include <windows.h>
#include <thread>
#include <atomic>

class Application;
class Script;

namespace fs              = std::filesystem;

class ScriptModule : public Module
{
  public:
    ScriptModule() {}
    ~ScriptModule() override { UnloadDLL(); }

    bool Init() override;
    update_status Update(float deltaTime) override;
    bool ShutDown() override;

    Script* CreateScript(const std::string name) const { return createScriptFunc(name); }
    void DestroyScript(Script* script) const { destroyScriptFunc(script); }


  private:
    void LoadDLL();
    void UnloadDLL();
    void ReloadDLLIfUpdated();
    bool IsFileLocked(const std::filesystem::path& filePath);

  private:
    HMODULE dllHandle = nullptr;

    typedef Script* (*CreateScriptFunc)(const std::string&);
    typedef void (*DestroyScriptFunc)(Script*);
    typedef void (*DestroyExterns)();
    typedef void (*SetApplicationFunc)(Application*);
    typedef void (*SetLogsFunc)(std::vector<char*>*);

    CreateScriptFunc createScriptFunc   = nullptr;
    DestroyScriptFunc destroyScriptFunc = nullptr;
    DestroyExterns destroyExternsFunc   = nullptr;
    SetApplicationFunc setAppFunc       = nullptr;
    SetLogsFunc setLogsFunc             = nullptr;

    Script* scriptInstance              = nullptr;

    fs::file_time_type lastWriteTime;
    std::atomic<bool> running = true;
    std::thread dllMonitorThread;
    const fs::path copyPath = GAME_PATH;
    #ifdef _DEBUG
    const fs::path dllPath  = DEBUG_DLL_PATH;
    #else
    const fs::path dllPath  = RELEASE_DLL_PATH;
    #endif
};