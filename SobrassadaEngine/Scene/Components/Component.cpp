#include "Component.h"

#include "Application.h"
#include "ComponentUtils.h"
#include "SceneModule.h"

#include "imgui.h"
#include <Math/float4x4.h>
#include <string>

Component::Component(UID uid, UID parentUID, const char* initName, ComponentType type)
    : uid(uid), parentUID(parentUID), type(type), enabled(true)
{
    memcpy(name, initName, strlen(initName));
}

Component::Component(const rapidjson::Value& initialState)
    : uid(initialState["UID"].GetUint64()), parentUID(initialState["ParentUID"].GetUint64()),
      type(static_cast<ComponentType>(initialState["Type"].GetInt()))
{
    enabled              = initialState["Enabled"].GetBool();

    const char* initName = initialState["Name"].GetString();
    memcpy(name, initName, strlen(initName));
}

void Component::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    targetState.AddMember("UID", uid, allocator);
    targetState.AddMember("ParentUID", parentUID, allocator);
    targetState.AddMember("Type", type, allocator);

    targetState.AddMember("Enabled", enabled, allocator);
    targetState.AddMember("Name", rapidjson::Value(std::string(name).c_str(), allocator), allocator);
}

void Component::RenderEditorInspector()
{
    ImGui::InputText("Name", name, sizeof(name));
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &enabled);
}

const float4x4& Component::GetGlobalTransform()
{
    GameObject* parent = GetParent();
    if (parent != nullptr)
    {
        return parent->GetGlobalTransform();
    }
    return float4x4::identity;
}

GameObject* Component::GetParent()
{
    if (parent == nullptr)
    {
        parent = App->GetSceneModule()->GetGameObjectByUUID(parentUID);
        if (parent == nullptr)
        {
            // GLOG("Could not load parent with UID: %s - Object does not exist", uidParent)
        }
    }
    return parent;
}
