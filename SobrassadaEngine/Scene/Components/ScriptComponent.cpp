#include "ScriptComponent.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "SceneModule.h"
#include "Script.h"
#include "ScriptModule.h"

#include "ImGui.h"
#include "Math/float3.h"

ScriptComponent::ScriptComponent(UID uid, GameObject* parent) : Component(uid, parent, "Script", COMPONENT_SCRIPT)
{
}

ScriptComponent::ScriptComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("Script Name"))
    {
        CreateScript(initialState["Script Name"].GetString());
        scriptInstance->Load(initialState);
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
    if (scriptInstance != nullptr) scriptInstance->Save(targetState, allocator);
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
    if (!IsEffectivelyEnabled()) return;

    if (App->GetSceneModule()->GetInPlayMode())
    {
        if (scriptInstance != nullptr)
        {
            scriptInstance->Update(deltaTime);
        }
    }
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
            for (int i = 0; i < sizeof(scripts) / sizeof(char*); i++)
            {
                if (ImGui::Selectable(scripts[i]))
                {
                    if (scriptInstance != nullptr) DeleteScript();
                    CreateScript(scripts[i]);
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

void ScriptComponent::InitScriptInstances()
{
    if (scriptInstance != nullptr)
    {
        scriptInstance->Init();
    }
}

void ScriptComponent::OnCollision(GameObject* otherObject, const float3& collisionNormal)
{
    if (scriptInstance != nullptr)
    {
        scriptInstance->OnCollision(otherObject, collisionNormal);
    }
}

void ScriptComponent::CreateScript(const std::string& scripString)
{
    scriptName     = scripString;
    scriptInstance = App->GetScriptModule()->CreateScript(scripString, parent);
    if (scriptInstance == nullptr) scriptName = "Not selected";

    scriptType = ScriptType(SearchIdxForString(scriptName));
}

void ScriptComponent::DeleteScript()
{
    App->GetScriptModule()->DestroyScript(scriptInstance);
    scriptInstance = nullptr;
}

int ScriptComponent::SearchIdxForString(const std::string& scriptString) const
{
    int idx = 0;
    for (int i = 0; i < sizeof(scripts) / sizeof(scripts[0]); ++i)
    {
        if (scriptString == scripts[i])
        {
            idx = i;
            break;
        }
    }
    return idx;
}
