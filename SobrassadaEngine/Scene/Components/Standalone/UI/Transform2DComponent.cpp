#include "Transform2DComponent.h"
#include "imgui.h"

Transform2DComponent::Transform2DComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Transform 2D", COMPONENT_TRANSFORM_2D)
{
}

Transform2DComponent::~Transform2DComponent()
{
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
