#include "ScriptComponent.h"
#include "Application.h"
#include "ImGui.h"
#include "SceneModule.h"
#include "Script.h"
#include "ScriptModule.h"
#include "EditorUIModule.h"

// Do script of object rotating

ScriptComponent::ScriptComponent(UID uid, GameObject* parent) : Component(uid, parent, "Script", COMPONENT_SCRIPT)
{
}

ScriptComponent::ScriptComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("Script Name"))
    {
        CreateScript(initialState["Script Name"].GetString());

        if (scriptInstance != nullptr && initialState.HasMember("ScriptData") && initialState["ScriptData"].IsObject())
        {
            scriptInstance->LoadFromJson(initialState["ScriptData"]);
        }
    }
}


ScriptComponent::~ScriptComponent()
{
    if (scriptInstance) DeleteScript();
}

void ScriptComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);
    targetState.AddMember("Script Name", rapidjson::Value(scriptName.c_str(), allocator), allocator);

    if (scriptInstance != nullptr)
    {
        rapidjson::Value scriptData(rapidjson::kObjectType);
        scriptInstance->SaveToJson(scriptData, allocator);
        targetState.AddMember("ScriptData", scriptData, allocator);
    }
}


void ScriptComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_SCRIPT)
    {
        const ScriptComponent* otherScript = static_cast<const ScriptComponent*>(other);
        enabled                            = otherScript->enabled;

        CreateScript(otherScript->scriptName);
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void ScriptComponent::Update(float deltaTime)
{
    if (App->GetSceneModule()->GetInPlayMode())
    {
        if (scriptInstance != nullptr)
        {
            if (!startScript)
            {
                scriptInstance->Init();
                startScript = true;
            }
            scriptInstance->Update(deltaTime);
        }
    }
    else startScript = false;
}

void ScriptComponent::Render(float deltaTime)
{
}

void ScriptComponent::RenderDebug(float deltaTime)
{

}

void ScriptComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();
    if (enabled)
    {
        ImGui::SeparatorText("Script Component");
        ImGui::Text(scriptName.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Select script"))
        {
            ImGui::OpenPopup("Select Script");
        }
        if (ImGui::BeginPopup("Select Script"))
        {
            for (const auto& scriptType : scripts)
            {
                if (ImGui::Selectable(scriptType.c_str()))
                {
                    if (scriptInstance != nullptr) DeleteScript();
                    CreateScript(scriptType);
                }
            }
            ImGui::EndPopup();
        }
        if (scriptInstance != nullptr)
        {
            ImGui::Separator();
            scriptInstance->Inspector();
        }
    }
}

void ScriptComponent::CreateScript(const std::string& scriptType)
{
    scriptName     = scriptType;
    scriptInstance = App->GetScriptModule()->CreateScript(scriptType, parent);
    if (scriptInstance == nullptr) scriptName = "Not selected";
}

void ScriptComponent::DeleteScript()
{
    App->GetScriptModule()->DestroyScript(scriptInstance);
    scriptInstance = nullptr;
}