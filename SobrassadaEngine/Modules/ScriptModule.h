#pragma once
#pragma once

#include "Globals.h"
#include "Module.h"
#include "rapidjson/document.h"

#include <atomic>
#include <filesystem>
#include <thread>
#include <unordered_map>
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
    bool close();
    bool ShutDown() override;

    Script* CreateScript(const std::string& name, GameObject* parent) const { return createScriptFunc(name, parent); }
    void DestroyScript(Script* script) const { destroyScriptFunc(script); }
    const int GetScriptCount() const { return getScriptCountFunc(); }
    const char* GetScriptName(const int i) const { return getScriptNameFunc(i); }
    const int GetScriptIdx(const std::string& scriptString) const { return searchIdxNameFunc(scriptString); }

  private:
    void LoadDLL();
    void UnloadDLL();
    void ReloadDLLIfUpdated();
    void DeleteAllScripts(bool saveJson);
    void RecreateAllScripts();
    bool IsFileLocked(const std::filesystem::path& filePath);

    void SaveScriptsToFile(const std::string& filename, const rapidjson::Document& doc);
    bool LoadScriptsFromFile(const std::string& filename, rapidjson::Document& doc);

  private:
    HMODULE dllHandle = nullptr;

    typedef Script* (*CreateScriptFunc)(const std::string&, GameObject*);
    typedef void (*DestroyScriptFunc)(Script*);
    typedef void (*StartSobrassadaScripts)(Application* App);
    typedef void (*FreeSobrassadaScripts)();

    typedef const char* (*GetScriptNameDLL)(const int index);
    typedef const int (*GetScriptCountDLL)();
    typedef const int (*SearchIdxName)(const std::string& scriptString);

    StartSobrassadaScripts startScriptFunc = nullptr;
    CreateScriptFunc createScriptFunc      = nullptr;
    DestroyScriptFunc destroyScriptFunc    = nullptr;
    FreeSobrassadaScripts freeScriptFunc   = nullptr;

    GetScriptNameDLL getScriptNameFunc        = nullptr;
    GetScriptCountDLL getScriptCountFunc      = nullptr;
    SearchIdxName searchIdxNameFunc        = nullptr;

    int scriptCount                        = 0;

    fs::file_time_type lastWriteTime;
    std::atomic<bool> running = true;
    std::thread dllMonitorThread;
    const fs::path copyPath = GAME_PATH;
#ifdef _DEBUG
    const fs::path& dllPath = DEBUG_DLL_PATH;
#else
    const fs::path& dllPath = RELEASE_DLL_PATH;
#endif
};