#include "pch.h"
#include "RotateGameObject.h"
#include "Globals.h"
#include "ButtonScript.h"
#include <string>

#ifndef SOBRASSADASCRIPTS_EXPORTS
#define SOBRASSADA_API __declspec(dllexport)
#else
#define SOBRASSADA_API __declspec(dllimport)
#endif

extern "C" SOBRASSADA_API Script* CreateScript(const std::string& scriptType, GameObject* parent)
{
    if (scriptType == "RotateGameObject") return new RotateGameObject(parent);
    if (scriptType == "ButtonScript") return new ButtonScript(parent);
    return nullptr;
}

extern "C" SOBRASSADA_API void DestroyScript(Script* script)
{
    delete script;
}