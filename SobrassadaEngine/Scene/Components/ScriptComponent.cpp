#include "ScriptComponent.h"
#include "Application.h"
#include "Script.h"
#include "ScriptModule.h"

// GLOG in DLL
// Finish ScriptComponent.cpp
// When unload the DLL, I need to destroy and recreate the script
// Change the std::string in the createScriptFunc to an enum to make it easier 
// Do script of object rotating

ScriptComponent::ScriptComponent(UID uid, GameObject* parent) : Component(uid, parent, "Script", COMPONENT_SCRIPT)
{
}

ScriptComponent::ScriptComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("Script Name"))
    {
        scriptName     = initialState["Script Name"].GetString();
        scriptInstance = App->GetScriptModule()->CreateScript(scriptName);
        scriptInstance->Init();
    }
}

ScriptComponent::~ScriptComponent()
{
    if (scriptInstance)
    {
        App->GetScriptModule()->DestroyScript(scriptInstance);
        scriptInstance = nullptr;
    }
}

void ScriptComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);
    targetState.AddMember("Script Name", rapidjson::Value(scriptName.c_str(), allocator), allocator);
}

void ScriptComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_CAMERA)
    {
        const ScriptComponent* otherScript = static_cast<const ScriptComponent*>(other);
        enabled                            = otherScript->enabled;

        // Create Script
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void ScriptComponent::Update(float deltaTime)
{
    if (!scriptInstance) scriptInstance->Update(deltaTime);
}

void ScriptComponent::Render(float deltaTime)
{
}

void ScriptComponent::RenderEditorInspector()
{
}