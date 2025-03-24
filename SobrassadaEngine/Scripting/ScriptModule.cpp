#include "ScriptModule.h"
#include <thread>

// Information: To make the dll works you need to recompile (not compile!)
bool ScriptModule::Init()
{
    ReloadDLLIfUpdated();
    return true;
}

void ScriptModule::LoadDLL()
{
    dllHandle = LoadLibrary("SobrassadaScripts.dll");
    if (!dllHandle)
    {
        GLOG("Failed to load DLL\n");
        return;
    }
    lastWriteTime     = fs::last_write_time("SobrassadaScripts.dll");

    createScriptFunc  = (CreateScriptFunc)GetProcAddress(dllHandle, "CreateScript");
    destroyScriptFunc = (DestroyScriptFunc)GetProcAddress(dllHandle, "DestroyScript");
    if (!createScriptFunc || !destroyScriptFunc)
    {
        GLOG("Failed to load CreateScript or DestroyScript functions\n");
        return;
    }

    scriptInstance = createScriptFunc();
    if (scriptInstance && !scriptInstance->Init())
    {
        GLOG("Failed to initialize script\n");
        destroyScriptFunc(scriptInstance);
        scriptInstance = nullptr;
    }
}

update_status ScriptModule::Update(float deltaTime)
{
    ReloadDLLIfUpdated();

    if (scriptInstance)
    {
        scriptInstance->Update(deltaTime);
    }

    return UPDATE_CONTINUE;
}

void ScriptModule::UnloadDLL()
{
    if (dllHandle)
    {
        if (scriptInstance)
        {
            destroyScriptFunc(scriptInstance);
            scriptInstance = nullptr;
        }

        createScriptFunc  = nullptr;
        destroyScriptFunc = nullptr;

        FreeLibrary(dllHandle);
        dllHandle = nullptr;
    }
}

void ScriptModule::ReloadDLLIfUpdated()
{
    //This needs to be move into globals.h
    const fs::path dllPath  = "..\\SobrassadaEngine\\x64\\Debug\\SobrassadaScripts.dll";
    const fs::path copyPath = "..\\Game";

    if (fs::exists(dllPath))
    {
        fs::file_time_type currentWriteTime = fs::last_write_time(dllPath);

        if (currentWriteTime != lastWriteTime)
        {
            if (lastWriteTime != fs::file_time_type {}) GLOG("DLL has been updated, reloading...\n");

            UnloadDLL();
            // We need to do a little sleep because windows doesnt release directly the program (probably I need to change that)
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            fs::copy(dllPath, copyPath, fs::copy_options::overwrite_existing);
            LoadDLL();
            lastWriteTime = currentWriteTime;
        }
    }
}