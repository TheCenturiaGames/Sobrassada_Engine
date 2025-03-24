#pragma once
#include "Globals.h"
#include "Module.h"
#include <filesystem>
#include <windows.h>

typedef void (*FibonacciInitFunc)(int, int);
typedef bool (*FibonacciNextFunc)();
typedef int (*FibonacciIndexFunc)();
typedef int (*FibonacciCurrentFunc)();

namespace fs = std::filesystem;

class ScriptModule : public Module
{
  public:
    ScriptModule()
        : dllHandle(nullptr), fibonacci_initFunc(nullptr), fibonacci_nextFunc(nullptr), fibonacci_indexFunc(nullptr),
          fibonacci_currentFunc(nullptr)
    {
    }
    ~ScriptModule() override { UnloadDLL(); }

    bool Init() override;
    update_status Update(float deltaTime) override;
    void LoadDLL();
    void UnloadDLL();
    void ReloadDLLIfUpdated();

    void CallTestFunction();

  private:
    HINSTANCE dllHandle;
    FibonacciInitFunc fibonacci_initFunc;
    FibonacciNextFunc fibonacci_nextFunc;
    FibonacciIndexFunc fibonacci_indexFunc;
    FibonacciCurrentFunc fibonacci_currentFunc;

    fs::file_time_type lastWriteTime;
};