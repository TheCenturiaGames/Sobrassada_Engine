#include "ButtonComponent.h"

#include "Application.h"
#include "CanvasComponent.h"
#include "GameObject.h"
#include "ImageComponent.h"
#include "Transform2DComponent.h"

#include "imgui.h"

ButtonComponent::ButtonComponent(UID uid, GameObject* parent)
    : defaultColor(float3(1.0f, 1.0f, 1.0f)), hoverColor(float3(0.0f, 1.0f, 1.0f)),
      disabledColor(float3(0.5f, 0.5f, 0.5f)), isInteractable(true), Component(uid, parent, "Button", COMPONENT_BUTTON)
{
}

ButtonComponent::ButtonComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("IsInteractable"))
    {
        isInteractable = initialState["IsInteractable"].GetBool();
    }

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

    if (initialState.HasMember("DisabledColor") && initialState["DisabledColor"].IsArray())
    {
        const rapidjson::Value& initColor = initialState["DisabledColor"];
        disabledColor.x                   = initColor[0].GetFloat();
        disabledColor.y                   = initColor[1].GetFloat();
        disabledColor.z                   = initColor[2].GetFloat();
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

    targetState.AddMember("IsInteractable", isInteractable, allocator);

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

    rapidjson::Value valColorDisabled(rapidjson::kArrayType);
    valColorDisabled.PushBack(disabledColor.x, allocator);
    valColorDisabled.PushBack(disabledColor.y, allocator);
    valColorDisabled.PushBack(disabledColor.z, allocator);
    targetState.AddMember("DisabledColor", valColorDisabled, allocator);
}

void ButtonComponent::Clone(const Component* other)
{
    // It will have to look for the canvas here probably, seems better to do that in a separate function
    if (other->GetType() == COMPONENT_BUTTON)
    {
        const ButtonComponent* otherButton = static_cast<const ButtonComponent*>(other);
        enabled                            = otherButton->enabled;
        isInteractable                     = otherButton->isInteractable;

        defaultColor                       = otherButton->defaultColor;
        hoverColor                         = otherButton->hoverColor;
        disabledColor                      = otherButton->disabledColor;
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

    if (ImGui::Checkbox("Interactable", &isInteractable)) OnInteractionChange();
    if (ImGui::ColorEdit3("Default color", defaultColor.ptr())) image->SetColor(defaultColor);
    ImGui::ColorEdit3("Hover color", hoverColor.ptr());
    ImGui::ColorEdit3("Disabled color", disabledColor.ptr());
}

bool ButtonComponent::UpdateMousePosition(const float2& mousePos, bool dismiss)
{
    if (!isInteractable) return false;

    if (!dismiss && IsWithinBounds(mousePos))
    {
        if (!isHovered)
        {
            // On mouse enter
            if (image) image->SetColor(hoverColor);
            isHovered = true;
        }
        return true;
    }
    else
    {
        if (isHovered)
        {
            // On mouse exit
            if (image) image->SetColor(defaultColor);
            isHovered = false;
        }
        return false;
    }
}

void ButtonComponent::OnClick()
{
    GLOG("Clicked button!");
    onClickDispatcher.Call();
}

bool ButtonComponent::IsWithinBounds(const float2& pos) const
{
    if (transform2D == nullptr) return false;

    const float2 screenPos = float2(
        transform2D->GetGlobalPosition().x + (parentCanvas->GetWidth() / 2),
        transform2D->GetGlobalPosition().y + (parentCanvas->GetHeight() / 2)
    );

    if (pos.x < screenPos.x + (transform2D->size.x / 2) && pos.x > screenPos.x - (transform2D->size.x / 2) &&
        pos.y < screenPos.y + (transform2D->size.y / 2) && pos.y > screenPos.y - (transform2D->size.y / 2))
        return true;

    else return false;
}

void ButtonComponent::AddOnClickCallback(Delegate<void>& newDelegate)
{
    delegateID = onClickDispatcher.SubscribeCallback(std::move(newDelegate));
}

void ButtonComponent::RemoveOnClickCallback()
{
    onClickDispatcher.RemoveCallback(delegateID);
}

void ButtonComponent::OnInteractionChange() const
{
    if (image == nullptr) return;

    if (isInteractable) image->SetColor(defaultColor);
    else image->SetColor(disabledColor);
}