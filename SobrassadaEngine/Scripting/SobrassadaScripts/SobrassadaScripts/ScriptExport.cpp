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