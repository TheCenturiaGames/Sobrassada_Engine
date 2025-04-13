#include "ButtonComponent.h"

#include "Application.h"
#include "CanvasComponent.h"
#include "GameObject.h"
#include "ImageComponent.h"
#include "Transform2DComponent.h"

#include "imgui.h"

ButtonComponent::ButtonComponent(UID uid, GameObject* parent)
    : defaultColor(float3(1.0f, 1.0f, 1.0f)), hoverColor(float3(0.0f, 1.0f, 1.0f)),
      Component(uid, parent, "Button", COMPONENT_BUTTON)
{
}

ButtonComponent::ButtonComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("DefaultColor") && initialState["DefaultColor"].IsArray())
    {
        const rapidjson::Value& initColor = initialState["DefaultColor"];
        defaultColor.x                    = initColor[0].GetFloat();
        defaultColor.y                    = initColor[1].GetFloat();
        defaultColor.z                    = initColor[2].GetFloat();
    }

    if (initialState.HasMember("HoverColor") && initialState["HoverColor"].IsArray())
    {
        const rapidjson::Value& initColor = initialState["HoverColor"];
        hoverColor.x                      = initColor[0].GetFloat();
        hoverColor.y                      = initColor[1].GetFloat();
        hoverColor.z                      = initColor[2].GetFloat();
    }
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
    Component* linkedImage = parent->GetComponentByType(COMPONENT_IMAGE);
    if (linkedImage == nullptr)
    {
        parent->CreateComponent(COMPONENT_IMAGE);
        image = static_cast<ImageComponent*>(parent->GetComponentByType(COMPONENT_IMAGE));
    }
    else
    {
        image = static_cast<ImageComponent*>(linkedImage);
    }
}

void ButtonComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    rapidjson::Value valColorDefault(rapidjson::kArrayType);
    valColorDefault.PushBack(defaultColor.x, allocator);
    valColorDefault.PushBack(defaultColor.y, allocator);
    valColorDefault.PushBack(defaultColor.z, allocator);
    targetState.AddMember("DefaultColor", valColorDefault, allocator);

    rapidjson::Value valColorHover(rapidjson::kArrayType);
    valColorHover.PushBack(hoverColor.x, allocator);
    valColorHover.PushBack(hoverColor.y, allocator);
    valColorHover.PushBack(hoverColor.z, allocator);
    targetState.AddMember("HoverColor", valColorHover, allocator);
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

    ImGui::ColorEdit3("Default color", defaultColor.ptr());
    ImGui::ColorEdit3("Hover color", hoverColor.ptr());
}

bool ButtonComponent::UpdateMousePosition(const float2& mousePos, bool dismiss)
{
    if (!dismiss && IsWithinBounds(mousePos))
    {
        GLOG("Mouse is in");

        if (!isHovered)
        {
            // On mouse enter
            image->SetColor(hoverColor);
            isHovered = true;
        }
        return true;
    }
    else
    {
        if (isHovered)
        {
            // On mouse exit
            image->SetColor(defaultColor);
            isHovered = false;
        }
        return false;
    }
}

void ButtonComponent::OnClick()
{
    GLOG("Clicked button!");
}

bool ButtonComponent::IsWithinBounds(const float2& pos)
{
    if (transform2D == nullptr) return false;

    GLOG("Mouse pos: %f, %f", pos.x, pos.y);
    const float2 screenPos = float2(
        transform2D->GetGlobalPosition().x + (parentCanvas->GetWidth() / 2),
        transform2D->GetGlobalPosition().y + (parentCanvas->GetHeight() / 2)
    );
    GLOG(
        "Max x: %f. Min x: %f. Max y: %f. Min y: %f", screenPos.x + (transform2D->size.x / 2),
        screenPos.x - (transform2D->size.x / 2), screenPos.y - (transform2D->size.y / 2),
        screenPos.y + (transform2D->size.y / 2)
    )

    if (pos.x < screenPos.x + (transform2D->size.x / 2) && pos.x > screenPos.x - (transform2D->size.x / 2) &&
        pos.y < screenPos.y + (transform2D->size.y / 2) && pos.y > screenPos.y - (transform2D->size.y / 2))
        return true;

    else return false;
}