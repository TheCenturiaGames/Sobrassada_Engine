#include "ButtonComponent.h"

#include "Application.h"
#include "CanvasComponent.h"
#include "GameObject.h"
#include "ImageComponent.h"
#include "Transform2DComponent.h"

#include "imgui.h"

ButtonComponent::ButtonComponent(UID uid, GameObject* parent)
    : defaultColor(float3(1.0f, 1.0f, 1.0f)), hoverColor(float3(0.0f, 1.0f, 1.0f)), Component(uid, parent, "Button", COMPONENT_BUTTON)
{
}

ButtonComponent::ButtonComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
}

ButtonComponent::~ButtonComponent()
{
}

void ButtonComponent::Init()
{
    Component* transform = parent->GetComponentByType(COMPONENT_TRANSFORM_2D);
    if (transform == nullptr)
    {
        parent->CreateComponent(COMPONENT_TRANSFORM_2D);
        transform2D = static_cast<Transform2DComponent*>(parent->GetComponentByType(COMPONENT_TRANSFORM_2D));
    }
    else
    {
        transform2D = static_cast<Transform2DComponent*>(transform);
    }

    parentCanvas = transform2D->GetParentCanvas();

    if (parentCanvas == nullptr)
    {
        // Try to get it again, just in case the transform was created later
        transform2D->GetCanvas();
        parentCanvas = transform2D->GetParentCanvas();

        if (parentCanvas == nullptr) GLOG("[WARNING] Button has no parent canvas, it won't be rendered");
    }

    // Get the image
    Component* parentImage = parent->GetComponentByType(COMPONENT_IMAGE);
    if (parentImage == nullptr)
    {
        parent->CreateComponent(COMPONENT_IMAGE);
        image = static_cast<ImageComponent*>(parent->GetComponentByType(COMPONENT_IMAGE));
    }
    else
    {
        transform2D = static_cast<Transform2DComponent*>(parentImage);
    }
}

void ButtonComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);
}

void ButtonComponent::Clone(const Component* other)
{
    // It will have to look for the canvas here probably, seems better to do that in a separate function
    if (other->GetType() == COMPONENT_BUTTON)
    {
        const ButtonComponent* otherButton = static_cast<const ButtonComponent*>(other);
        enabled                            = otherButton->enabled;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void ButtonComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    ImGui::SeparatorText("Button");

    ImGui::ColorEdit3("Hover color", hoverColor.ptr());
}

void ButtonComponent::UpdateMousePosition(const float2& mousePos)
{
}

void ButtonComponent::OnClick()
{
    if (!isHovered) return;
}

bool ButtonComponent::IsWithinBounds(const float2& pos)
{
    if (transform2D == nullptr) return false;

    const float2 screenPos = float2(
        transform2D->GetGlobalPosition().x + (parentCanvas->GetWidth() / 2),
        transform2D->GetGlobalPosition().y + (parentCanvas->GetHeight() / 2)
    );

    if (pos.x < screenPos.x + transform2D->size.x && pos.x > screenPos.x - transform2D->size.x &&
        pos.y < screenPos.y + transform2D->size.y && pos.y > screenPos.y - transform2D->size.x)
    {
        GLOG("Mouse is in");

        if (!isHovered)
        {
            // On mouse enter
            image->SetColor(hoverColor);
            isHovered = true;
        }
    }
    else
    {
        if (isHovered)
        {
            // On mouse exit
            image->SetColor(defaultColor);
        }
    }
}