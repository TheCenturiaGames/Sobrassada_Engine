#include "pch.h"
#include "MyScript.h"
#include "RotateGameObject.h"
#include <string>

extern "C" SOBRASSADA_API Script* CreateScript(const std::string& scriptType, GameObject* parent)
{
    if (scriptType == "MyScript") return new MyScript(parent);
    if (scriptType == "RotateGameObject") return new RotateGameObject(parent);
    return nullptr;
}

extern "C" SOBRASSADA_API void DestroyScript(Script* script)
{
    delete script;
}

extern "C" SOBRASSADA_API void DestroyExterns()
{
    App = nullptr;
    // Free memory from log*
    for (auto log : *logsDLL)
    {
        free(log.message);
    }
    logsDLL->clear();
    delete logsDLL;

}

extern "C" SOBRASSADA_API void setApplication(Application* application)
{
    App = application;
}

extern "C" SOBRASSADA_API std::vector<LogEntry>* DLLConsoleLogs()
{
    return logsDLL;
}