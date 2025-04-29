#pragma once

#include "Component.h"
#include "Globals.h"

enum ScriptType
{
    SCRIPT_ROTATE_GAME_OBJECT = 0,
    SCRIPT_BUTTON,
    SCRIPT_GOD_MODE,
    SCRIPT_CU_CHULAINN,
    SCRIPT_SOLDIER,
    SCRIPT_EXIT_GAME,
    SCRIPT_FULLSCREEN_TOGGLE,
    SCRIPT_VSYNC_TOGGLE,
    SCRIPT_PAUSE_MENU,
    SCRIPT_OPTIONS_MENU_SWITCHER,
    SCRIPT_MAIN_MENU_SELECTOR,
    SCRIPT_PRESS_ANY_KEY,
    SCRIPT_CAMERA_MOVEMENT,

    SCRIPT_TYPE_COUNT // Add at the end
};

namespace math
{
    class float3;
}

class Script;
class GameObject;

constexpr const char* scripts[] = {
    "RotateGameObject",          // SCRIPT_ROTATE_GAME_OBJECT
    "ButtonScript",              // SCRIPT_BUTTON
    "GodMode",                   // SCRIPT_GOD_MODE
    "CuChulainnScript",          // SCRIPT_CU_CHULAINN
    "SoldierScript",             // SCRIPT_SOLDIER
    "ExitGameScript",            // SCRIPT_EXIT_GAME
    "FullscreenToggleScript",    // SCRIPT_FULLSCREEN_TOGGLE
    "VSyncToggleScript",         // SCRIPT_VSYNC_TOGGLE
    "PauseMenuScript",           // SCRIPT_PAUSE_MENU
    "OptionsMenuSwitcherScript", // SCRIPT_OPTIONS_MENU_SWITCHER
    "MainMenuSelectorScript",    // SCRIPT_MAIN_MENU_SELECTOR
    "PressAnyKeyScript",         // SCRIPT_PRESS_ANY_KEY
    "CameraMovement"             // SCRIPT_CAMERA_MOVEMENT
};

static_assert(
    SCRIPT_TYPE_COUNT == sizeof(scripts) / sizeof(scripts[0]), "ScriptType enum and scripts[] size mismatch!"
);

class ScriptComponent : public Component
{
  public:
    ScriptComponent(UID uid, GameObject* parent);
    ScriptComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~ScriptComponent() override;

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderDebug(float deltaTime) override;
    void RenderEditorInspector() override;

    void InitScriptInstances();
    void OnCollision(GameObject* otherObject, const float3& collisionNormal);
    void CreateScript(const std::string& scriptType);
    void DeleteScript();

    const std::string& GetScriptName() const { return scriptName; }
    Script* GetScriptInstance() const { return scriptInstance; }
    const ScriptType GetScriptType() const { return scriptType; }

  private:
    int SearchIdxForString(const std::string& name) const;
    std::string scriptName = "Not selected";
    Script* scriptInstance = nullptr;
    bool startScript       = false;
    ScriptType scriptType  = SCRIPT_ROTATE_GAME_OBJECT;
};