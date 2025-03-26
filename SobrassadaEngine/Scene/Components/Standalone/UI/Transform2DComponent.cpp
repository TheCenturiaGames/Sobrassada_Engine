#include "Transform2DComponent.h"
#include "imgui.h"

Transform2DComponent::Transform2DComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Transform 2D", COMPONENT_TRANSFORM_2D)
{
}

Transform2DComponent::Transform2DComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("Position") && initialState["Position"].IsArray())
    {
        const rapidjson::Value& initPosition = initialState["Position"];
        position.x                           = initPosition[0].GetFloat();
        position.y                           = initPosition[1].GetFloat();
    }
    
    if (initialState.HasMember("Size") && initialState["Size"].IsArray())
    {
        const rapidjson::Value& initSize = initialState["Size"];
        size.x                           = initSize[0].GetFloat();
        size.y                           = initSize[1].GetFloat();
    }
}

Transform2DComponent::~Transform2DComponent()
{
}

void Transform2DComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    rapidjson::Value valPosition(rapidjson::kArrayType);
    valPosition.PushBack(position.x, allocator);
    valPosition.PushBack(position.y, allocator);
    targetState.AddMember("Position", valPosition, allocator); 
    
    rapidjson::Value valSize(rapidjson::kArrayType);
    valSize.PushBack(size.x, allocator);
    valSize.PushBack(size.y, allocator);
    targetState.AddMember("Size", valSize, allocator);
}

void Transform2DComponent::Update(float deltaTime)
{
}

void Transform2DComponent::Render(float deltaTime)
{
}

void Transform2DComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::Text("Transform 2D");

        if (ImGui::InputFloat2("Position", &position[0]))
        {
            // Update transform
        }
        if (ImGui::InputFloat2("Size", &size[0]))
        {
            // Update size
        }
    }
}

void Transform2DComponent::Clone(const Component* otherComponent)
{
}
