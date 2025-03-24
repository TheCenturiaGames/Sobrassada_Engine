#pragma once
#include "Globals.h"
#include "Module.h"
#include "Script.h"
#include <filesystem>
#include <windows.h>

typedef Script* (*CreateScriptFunc)();
typedef void (*DestroyScriptFunc)(Script*);

namespace fs = std::filesystem;

class ScriptModule : public Module
{
  public:
    ScriptModule() {}
    ~ScriptModule() override { UnloadDLL(); }

    bool Init() override;
    update_status Update(float deltaTime) override;

  private:
    void LoadDLL();
    void UnloadDLL();
    void ReloadDLLIfUpdated();

  private:
    HMODULE dllHandle                   = nullptr;
    CreateScriptFunc createScriptFunc   = nullptr;
    DestroyScriptFunc destroyScriptFunc = nullptr;

    Script* scriptInstance              = nullptr;

    fs::file_time_type lastWriteTime;
};