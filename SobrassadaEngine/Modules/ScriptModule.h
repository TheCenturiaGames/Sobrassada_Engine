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

    HMODULE getHandle() const { return dllHandle; }

  private:
    void LoadDLL();
    void UnloadDLL();
    void ReloadDLLIfUpdated();
    bool IsFileLocked(const std::filesystem::path& filePath);

  private:
    HMODULE dllHandle = nullptr;

    typedef Script* (*CreateScriptFunc)(const std::string&);
    typedef void (*DestroyScriptFunc)(Script*);
    typedef void (*SetApplicationFunc)(Application*);

    CreateScriptFunc createScriptFunc   = nullptr;
    DestroyScriptFunc destroyScriptFunc = nullptr;
    SetApplicationFunc setAppFunc       = nullptr;

    Script* scriptInstance              = nullptr;

    fs::file_time_type lastWriteTime;
    std::atomic<bool> running = true;
    std::thread dllMonitorThread;
};