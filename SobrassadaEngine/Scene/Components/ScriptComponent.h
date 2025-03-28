#pragma once
#include "Component.h"
#include "Globals.h"

class Script;

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
    void RenderEditorInspector() override;

  private:
    std::string scriptName;
    Script* scriptInstance = nullptr;
    std::vector<std::string> scripts = {"MyScript"};
};