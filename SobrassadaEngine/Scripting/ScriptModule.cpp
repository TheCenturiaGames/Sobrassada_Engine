#include "ScriptModule.h"
#include "Fibonacci.h"

bool ScriptModule::Init()
{
    LoadDLL();
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
    fibonacci_initFunc    = (FibonacciInitFunc)GetProcAddress(dllHandle, "fibonacci_init");
    fibonacci_indexFunc   = (FibonacciIndexFunc)GetProcAddress(dllHandle, "fibonacci_index");
    fibonacci_currentFunc = (FibonacciCurrentFunc)GetProcAddress(dllHandle, "fibonacci_current");
    fibonacci_nextFunc    = (FibonacciNextFunc)GetProcAddress(dllHandle, "fibonacci_next");
    fibonacci_version = (FibonacciVersion)GetProcAddress(dllHandle, "GetDLLVersion");
    GLOG("%s", fibonacci_version());

    if (!fibonacci_initFunc || !fibonacci_nextFunc || !fibonacci_indexFunc || !fibonacci_currentFunc)
    {
        GLOG("Failed to load fibonacci functions\n");
    }
}

update_status ScriptModule::Update(float deltaTime)
{
    ReloadDLLIfUpdated();
    return UPDATE_CONTINUE;
}

void ScriptModule::UnloadDLL()
{
    if (dllHandle)
    {
        FreeLibrary(dllHandle);
        dllHandle         = nullptr;
        fibonacci_initFunc    = nullptr;
        fibonacci_nextFunc    = nullptr;
        fibonacci_indexFunc   = nullptr;
        fibonacci_currentFunc = nullptr;
    }
}

void ScriptModule::CallTestFunction()
{
    if (fibonacci_initFunc && fibonacci_nextFunc && fibonacci_indexFunc && fibonacci_currentFunc)
    {
        int i = 0;
        fibonacci_initFunc(1, 1);
        // Write out the sequence values until overflow.
        do {
            GLOG("%d: %d", fibonacci_indexFunc(), fibonacci_currentFunc());
            i++;
        } while (fibonacci_nextFunc() && i < 10);
    }
    else
    {
        GLOG("Fibonacci functions are not loaded correctly\n");
    }
}

void ScriptModule::ReloadDLLIfUpdated()
{
    // Ruta del archivo DLL
    const fs::path dllPath = "SobrassadaScripts.dll";
    
    // Verificar si el archivo existe
    if (fs::exists(dllPath))
    {
        // Obtener el tiempo de última modificación
        fs::file_time_type currentWriteTime = fs::last_write_time(dllPath);
        
        // Si el tiempo de modificación ha cambiado, recargar la DLL
        if (currentWriteTime != lastWriteTime)
        {
            GLOG("DLL has been updated, reloading...\n");
            UnloadDLL();  // Descargar la DLL antigua
            LoadDLL();    // Cargar la nueva DLL
            lastWriteTime = currentWriteTime;  // Actualizar el último tiempo de modificación
            CallTestFunction();
        }
    }
}