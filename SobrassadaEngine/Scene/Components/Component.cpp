#include "Component.h"

#include "Application.h"
#include "ComponentUtils.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "Root/RootComponent.h"
#include "SceneModule.h"

#include "debug_draw.hpp"
#include "imgui.h"
#include <Geometry/OBB.h>
#include <Math/Quat.h>
#include <Math/float4x4.h>
#include <string>

Component::Component(
    const UID uid, const UID uidParent, const UID uidRoot, const char* initName, int type,
    const float4x4& parentGlobalTransform
)
    : uid(uid), uidParent(uidParent), uidRoot(uidRoot), type(type), enabled(true),
      globalTransform(parentGlobalTransform)
{
    localComponentAABB.SetNegativeInfinity();
    globalComponentAABB.SetNegativeInfinity();
    memcpy(name, initName, strlen(initName));
}

Component::Component(const rapidjson::Value& initialState)
    : uid(initialState["UID"].GetUint64()),
      type(initialState["Type"].GetInt())
{
    enabled   = initialState["Enabled"].GetBool();
    
    const char* initName = initialState["Name"].GetString();
    memcpy(name, initName, strlen(initName));
}

Component::~Component()
{
    for (Component* childComponent : GetChildComponents())
    {
        globalComponentAABB.Enclose(childComponent->GetGlobalAABB());
    }
    for (Component* childComponent : GetChildComponents())
    {
        App->GetSceneModule()->RemoveComponent(childComponent->GetUID());
        delete App->GetSceneModule()->GetComponentByUID(childComponent->GetUID());
    }
}

void Component::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    targetState.AddMember("UID", uid, allocator);
    targetState.AddMember("Type", type, allocator);
    
    targetState.AddMember("Enabled", enabled, allocator);
    targetState.AddMember("Name", rapidjson::Value(std::string(name).c_str(), allocator), allocator);
}

void Component::Render()
{
    if (enabled)
    {
        for (Component* child : GetChildComponents())
        {
            child->Render();
        }
    }
}

bool Component::AddChildComponent(const UID componentUID)
{
    children.push_back(componentUID);
    return true;
}

bool Component::RemoveChildComponent(const UID componentUID)
{
    if (const auto it = std::find(children.begin(), children.end(), componentUID); it != children.end())
    {
        children.erase(it);
        return true;
    }
    return false;
}

bool Component::DeleteChildComponent(const UID componentUID)
{
    if (const auto it = std::find(children.begin(), children.end(), componentUID); it != children.end())
    {
        children.erase(it);
        delete App->GetSceneModule()->GetComponentByUID(componentUID
        ); // More efficient than finding the child pointer in childComponents
        App->GetSceneModule()->RemoveComponent(componentUID);

        return true;
    }

    return false;
}

void Component::RenderEditorInspector()
{
    ImGui::InputText("Name", name, sizeof(name));
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &enabled);
}

void Component::OnTransformUpdate(const float4x4& parentGlobalTransform)
{
    TransformUpdated(parentGlobalTransform);

    AABBUpdatable* parentObject = GetParent();
    if (parentObject != nullptr)
    {
        parentObject->PassAABBUpdateToParent();
    }
}

AABB& Component::TransformUpdated(const float4x4& parentGlobalTransform)
{
    globalTransform = parentGlobalTransform * localTransform;

    CalculateLocalAABB();

    for (Component* childComponent : GetChildComponents())
    {
        globalComponentAABB.Enclose(childComponent->TransformUpdated(globalTransform));
    }

    return globalComponentAABB;
}

void Component::PassAABBUpdateToParent()
{
    CalculateLocalAABB();

    for (Component* childComponent : GetChildComponents())
    {
        globalComponentAABB.Enclose(childComponent->GetGlobalAABB());
    }

    AABBUpdatable* parentObject = GetParent();
    if (parentObject != nullptr)
    {
        parentObject->PassAABBUpdateToParent();
    }
}

void Component::CalculateLocalAABB()
{
    OBB globalComponentOBB = OBB(localComponentAABB);
    globalComponentOBB.Transform(globalTransform); // TODO Testing once the aabb debug renderer is available

    globalComponentAABB = AABB(globalComponentOBB);
}
