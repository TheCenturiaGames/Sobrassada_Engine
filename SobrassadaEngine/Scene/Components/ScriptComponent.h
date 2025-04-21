#pragma once

#include "Component.h"
#include "Globals.h"

enum ScriptType
{
    SCRIPT_ROTATE_GAME_OBJECT = 0,
    SCRIPT_BUTTON,
    SCRIPT_GOD_MODE,
    SCRIPT_CU_CHULAINN,
    SCRIPT_SOLDIER
};

namespace math
{
    class float3;
}

class Script;
class GameObject;

constexpr const char* scripts[] = {"RotateGameObject", "ButtonScript", "GodMode", "CuChulainn", "Soldier"};

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

  private:
    std::string scriptName = "Not selected";
    Script* scriptInstance = nullptr;
    ScriptType scriptType  = SCRIPT_ROTATE_GAME_OBJECT;
};