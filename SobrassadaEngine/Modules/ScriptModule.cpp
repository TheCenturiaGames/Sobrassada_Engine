#include "ScriptModule.h"
#include "Application.h"
#include "Script.h"

bool ScriptModule::Init()
{
    running          = true;
    dllMonitorThread = std::thread(&ScriptModule::ReloadDLLIfUpdated, this);
    return true;
}

bool ScriptModule::ShutDown()
{
    running = false;
    if (dllMonitorThread.joinable()) dllMonitorThread.join();
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
    lastWriteTime      = fs::last_write_time("SobrassadaScripts.dll");

    createScriptFunc   = (CreateScriptFunc)GetProcAddress(dllHandle, "CreateScript");
    destroyScriptFunc  = (DestroyScriptFunc)GetProcAddress(dllHandle, "DestroyScript");
    destroyExternsFunc = (DestroyExterns)GetProcAddress(dllHandle, "DestroyExterns");
    setAppFunc         = (SetApplicationFunc)GetProcAddress(dllHandle, "setApplication");
    setLogsFunc        = (SetLogsFunc)GetProcAddress(dllHandle, "setLogs");

    if (!createScriptFunc || !destroyScriptFunc || !setAppFunc || !setLogsFunc || !destroyExternsFunc)
    {
        GLOG("Failed to load CreateScript or DestroyScript functions\n");
        return;
    }

    setAppFunc(App);
    setLogsFunc(Logs);
}

update_status ScriptModule::Update(float deltaTime)
{
    return UPDATE_CONTINUE;
}

void ScriptModule::UnloadDLL()
{
    if (dllHandle)
    {
        destroyExternsFunc();

        createScriptFunc   = nullptr;
        destroyScriptFunc  = nullptr;
        destroyExternsFunc = nullptr;
        setAppFunc         = nullptr;
        setLogsFunc        = nullptr;

        FreeLibrary(dllHandle);
        dllHandle = nullptr;
    }
}

bool ScriptModule::IsFileLocked(const std::filesystem::path& filePath)
{
    HANDLE hFile = CreateFileW(
        filePath.wstring().c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) return true;

    CloseHandle(hFile);
    return false;
}

void ScriptModule::ReloadDLLIfUpdated()
{
    while (running)
    {
        if (fs::exists(dllPath))
        {
            const fs::file_time_type currentWriteTime = fs::last_write_time(dllPath);

            if (currentWriteTime != lastWriteTime)
            {
                lastWriteTime = currentWriteTime;
                UnloadDLL();

                while (IsFileLocked(dllPath) && running)
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));

                if (!running) return;
                if (lastWriteTime != fs::file_time_type {}) GLOG("DLL has been updated, reloading...\n");
                fs::copy(dllPath, copyPath, fs::copy_options::overwrite_existing);

                LoadDLL();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}