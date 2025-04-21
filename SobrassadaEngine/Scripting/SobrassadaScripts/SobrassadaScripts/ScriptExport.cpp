#include "pch.h"

#include "ButtonScript.h"
#include "CuChulainn.h"
#include "Globals.h"
#include "GodMode.h"
#include "RotateGameObject.h"
#include "Soldier.h"

#include <string>

#ifndef SOBRASSADASCRIPTS_EXPORTS
#define SOBRASSADA_API __declspec(dllexport)
#else
#define SOBRASSADA_API __declspec(dllimport)
#endif

Application* AppEngine = nullptr;
extern "C" SOBRASSADA_API void InitSobrassadaScripts(Application* App)
{
    GLOG("Sobrassada Scripts Initialized");
    AppEngine = App;
}

extern "C" SOBRASSADA_API Script* CreateScript(const std::string& scriptType, GameObject* parent)
{
    if (scriptType == "RotateGameObject") return new RotateGameObject(parent);
    if (scriptType == "ButtonScript") return new ButtonScript(parent);
    if (scriptType == "GodMode") return new GodMode(parent);
    if (scriptType == "CuChulainn") return new CuChulainn(parent);
    if (scriptType == "Soldier") return new Soldier(parent);
    return nullptr;
}

extern "C" SOBRASSADA_API void DestroyScript(Script* script)
{
    delete script;
}

extern "C" SOBRASSADA_API void FreeSobrassadaScripts()
{
    GLOG("Sobrassada Scripts deleted");
    AppEngine = nullptr;
}