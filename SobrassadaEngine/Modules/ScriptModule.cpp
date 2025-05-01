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

#include <fstream>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

void ScriptModule::SaveScriptsToFile(const std::string& filename, const rapidjson::Document& doc)
{
    std::ofstream outFile(filename, std::ios::out | std::ios::trunc);

    if (!outFile.is_open())
    {
        GLOG("Error opening file to save scripts.\n");
        return;
    }

    // Usar PrettyWriter para guardar el JSON de forma legible
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);  // Convertir el documento JSON a texto

    // Guardar el texto JSON en el archivo
    outFile << buffer.GetString();
    outFile.close();

    GLOG("Scripts saved successfully to '%s'.\n", filename.c_str());
}

void ScriptModule::DeleteAllScripts()
{
    if (App->GetSceneModule()->GetScene())
    {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        rapidjson::Value scriptsArray(rapidjson::kArrayType);

        // Iterar sobre todos los objetos del juego
        for (auto& gameObject : App->GetSceneModule()->GetScene()->GetAllGameObjects())
        {
            ScriptComponent* scriptComponent = gameObject.second->GetComponent<ScriptComponent*>();

            if (!scriptComponent) continue;

            rapidjson::Value targetState(rapidjson::kObjectType);
            scriptComponent->Save(targetState, allocator);
            scriptsArray.PushBack(targetState, allocator);
            // Eliminar todos los scripts del ScriptComponent
            scriptComponent->DeleteAllScripts();
        }

        // Agregar el array de scripts al documento JSON
        doc.AddMember("Scripts", scriptsArray, allocator);

        SaveScriptsToFile("scripts_state.json", doc);
        GLOG("All scripts deleted\n");
        freeScriptFunc();
    }
}

bool ScriptModule::LoadScriptsFromFile(const std::string& filename, rapidjson::Document& doc)
{
    std::ifstream inFile(filename, std::ios::in);
    if (!inFile.is_open())
    {
        GLOG("Error opening file to load scripts.\n");
        return false;
    }

    // Leer todo el contenido del archivo
    std::string fileContent((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    // Parsear el contenido a un documento JSON
    doc.Parse(fileContent.c_str());

    if (doc.HasParseError())
    {
        GLOG("Error parsing JSON file.\n");
        return false;
    }

    return true;
}

void ScriptModule::RecreateAllScripts()
{
    if (App->GetSceneModule()->GetScene())
    {
       rapidjson::Document doc;
        if (!LoadScriptsFromFile("scripts_state.json", doc))
        {
            GLOG("Failed to load scripts state from file.\n");
            return;
        }

        // Asegurarse de que el JSON tiene la estructura correcta
        if (!doc.IsObject() || !doc.HasMember("Scripts") || !doc["Scripts"].IsArray())
        {
            GLOG("Invalid script state format in JSON file.\n");
            return;
        }

        const rapidjson::Value& scriptsArray = doc["Scripts"];

        // Iterar sobre todos los objetos del juego
        auto scriptIt = scriptsArray.Begin();
        for (auto& gameObject : App->GetSceneModule()->GetScene()->GetAllGameObjects())
        {
            ScriptComponent* scriptComponent = gameObject.second->GetComponent<ScriptComponent*>();
            if (!scriptComponent) continue;

            // Verificar si hay un estado guardado para este gameObject
            if (scriptIt != scriptsArray.End())
            {
                const rapidjson::Value& scriptState = *scriptIt;

                // Llamar a Load para restaurar el estado del script
                scriptComponent->Load(scriptState, gameObject.second);

                ++scriptIt;
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