#pragma once
#include "Globals.h"
#include "Module.h"
#include <atomic>
#include <filesystem>
#include <thread>
#include <windows.h>

class Application;
class Script;
class GameObject;

namespace fs = std::filesystem;

class ScriptModule : public Module
{
  public:
    ScriptModule() {}
    ~ScriptModule() override { UnloadDLL(); }

    bool Init() override;
    update_status Update(float deltaTime) override;
    bool ShutDown() override;

    Script* CreateScript(const std::string name, GameObject* parent) const { return createScriptFunc(name, parent); }
    void DestroyScript(Script* script) const { destroyScriptFunc(script); }

    std::vector<LogEntry>* GetDLLConsoleLogs() { 
        if(setDLLConsoleLogsFunc != nullptr ) return setDLLConsoleLogsFunc();
        return nullptr;
    }

  private:
    void LoadDLL();
    void UnloadDLL();
    void ReloadDLLIfUpdated();
    void DeleteAllScripts();
    void RecreateAllScripts();
    bool IsFileLocked(const std::filesystem::path& filePath);

  private:
    HMODULE dllHandle = nullptr;

    typedef Script* (*CreateScriptFunc)(const std::string&, GameObject*);
    typedef void (*DestroyScriptFunc)(Script*);
    typedef void (*DestroyExterns)();
    typedef void (*SetApplicationFunc)(Application*);
    typedef std::vector<LogEntry>* (*DLLConsoleLogs)();

    CreateScriptFunc createScriptFunc    = nullptr;
    DestroyScriptFunc destroyScriptFunc  = nullptr;
    DestroyExterns destroyExternsFunc    = nullptr;
    SetApplicationFunc setAppFunc        = nullptr;
    DLLConsoleLogs setDLLConsoleLogsFunc = nullptr;

    fs::file_time_type lastWriteTime;
    std::atomic<bool> running = true;
    std::thread dllMonitorThread;
    const fs::path copyPath = GAME_PATH;
#ifdef _DEBUG
    const fs::path dllPath = DEBUG_DLL_PATH;
#else
    const fs::path dllPath = RELEASE_DLL_PATH;
#endif
};