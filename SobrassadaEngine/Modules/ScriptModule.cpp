#include "ScriptModule.h"
#include "Application.h"
#include "Component.h"
#include "SceneModule.h"
#include "Script.h"
#include "ScriptComponent.h"

bool ScriptModule::Init()
{
    LoadDLL();
    running          = true;
    dllMonitorThread = std::thread(&ScriptModule::ReloadDLLIfUpdated, this);
    return true;
}

bool ScriptModule::ShutDown()
{
    UnloadDLL();
    running = false;
    if (dllMonitorThread.joinable()) dllMonitorThread.join();
    return true;
}

void ScriptModule::LoadDLL()
{
    dllHandle = LoadLibrary(TEXT("SobrassadaScripts.dll"));
    if (!dllHandle)
    {
        GLOG("Failed to load DLL\n");
        return;
    }
    lastWriteTime     = fs::last_write_time("SobrassadaScripts.dll");

    startScriptFunc   = (StartSobrassadaScripts)GetProcAddress(dllHandle, "InitSobrassadaScripts");
    createScriptFunc  = (CreateScriptFunc)GetProcAddress(dllHandle, "CreateScript");
    destroyScriptFunc = (DestroyScriptFunc)GetProcAddress(dllHandle, "DestroyScript");

    if (!createScriptFunc || !destroyScriptFunc)
    {
        GLOG("Failed to load CreateScript or DestroyScript functions\n");
        return;
    }

    startScriptFunc(App);
}

update_status ScriptModule::Update(float deltaTime)
{
    return UPDATE_CONTINUE;
}

void ScriptModule::UnloadDLL()
{
    if (dllHandle)
    {
        createScriptFunc  = nullptr;
        destroyScriptFunc = nullptr;

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

void ScriptModule::DeleteAllScripts()
{
    if (App->GetSceneModule()->GetScene())
    {
        for (auto* component : App->GetSceneModule()->GetScene()->GetAllComponents())
        {
            if (component->GetType() == ComponentType::COMPONENT_SCRIPT)
            {
                ScriptComponent* scriptComponent = static_cast<ScriptComponent*>(component);
                scriptComponent->DeleteScript();
            }
        }
    }
}

void ScriptModule::RecreateAllScripts()
{
    if (App->GetSceneModule()->GetScene())
    {
        for (auto* component : App->GetSceneModule()->GetScene()->GetAllComponents())
        {
            if (component->GetType() == ComponentType::COMPONENT_SCRIPT)
            {
                ScriptComponent* scriptComponent = static_cast<ScriptComponent*>(component);
                scriptComponent->CreateScript(scriptComponent->GetScriptName());
            }
        }
    }
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
                DeleteAllScripts();

                UnloadDLL();

                while (IsFileLocked(dllPath) && running)
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));

                if (!running) return;
                if (lastWriteTime != fs::file_time_type {}) GLOG("DLL has been updated, reloading...\n");
                fs::copy(dllPath, copyPath, fs::copy_options::overwrite_existing);

                LoadDLL();

                RecreateAllScripts();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}