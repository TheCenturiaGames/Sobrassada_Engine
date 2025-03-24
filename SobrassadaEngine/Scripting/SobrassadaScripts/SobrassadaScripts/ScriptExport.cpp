#include "pch.h"
#include "MyScript.h"

#ifdef SOBRASSADASCRIPTS_EXPORTS
#define SOBRASSADA_API __declspec(dllexport)
#else
#define SOBRASSADA_API __declspec(dllimport)
#endif

extern "C" SOBRASSADA_API Script* CreateScript()
{
    return new MyScript();
}

extern "C" SOBRASSADA_API void DestroyScript(Script* script)
{
    delete script;
}