#include "ScriptModule.h"
#include "Application.h"
#include "Component.h"
#include "GameObject.h"
#include "SceneModule.h"
#include "Script.h"
#include "ScriptComponent.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "imgui_internal.h"

bool ScriptModule::Init()
{
    LoadDLL();
    running          = true;
    dllMonitorThread = std::thread(&ScriptModule::ReloadDLLIfUpdated, this);
    return true;
}

bool ScriptModule::close()
{
    DeleteAllScripts();
    UnloadDLL();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    running = false;
    if (dllMonitorThread.joinable()) dllMonitorThread.join();

    return true;
}

bool ScriptModule::ShutDown()
{
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
    freeScriptFunc    = (FreeSobrassadaScripts)GetProcAddress(dllHandle, "FreeSobrassadaScripts");

    if (!startScriptFunc || !createScriptFunc || !destroyScriptFunc)
    {
        GLOG("Failed to load required functions from DLL\n Trying Again.");
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
        startScriptFunc   = nullptr;
        freeScriptFunc    = nullptr;

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
        for (auto& gameObject : App->GetSceneModule()->GetScene()->GetAllGameObjects())
        {
            ScriptComponent* scriptComponent = gameObject.second->GetComponent<ScriptComponent*>();
            if (scriptComponent) scriptComponent->DeleteAllScripts();
        }

        freeScriptFunc();
    }
}

void ScriptModule::RecreateAllScripts()
{
    if (App->GetSceneModule()->GetScene())
    {

        for (auto& gameObject : App->GetSceneModule()->GetScene()->GetAllGameObjects())
        {
            ScriptComponent* scriptComponent = gameObject.second->GetComponent<ScriptComponent*>();
            if (scriptComponent) scriptComponent->CreateScript(scriptComponent->GetScriptName());
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