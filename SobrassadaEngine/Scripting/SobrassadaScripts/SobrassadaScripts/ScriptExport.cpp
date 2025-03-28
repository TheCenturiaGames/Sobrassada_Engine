#include "pch.h"
#include "MyScript.h"
#include <string>

extern "C" SOBRASSADA_API Script* CreateScript(const std::string& scriptType)
{
    if (scriptType == "MyScript") return new MyScript();
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