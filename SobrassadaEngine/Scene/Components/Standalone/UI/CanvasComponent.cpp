#include "CanvasComponent.h"

#include "Application.h"
#include "ButtonComponent.h"
#include "CameraModule.h"
#include "DebugDrawModule.h"
#include "GameObject.h"
#include "GameUIModule.h"
#include "ImageComponent.h"
#include "SceneModule.h"
#include "ShaderModule.h"
#include "Transform2DComponent.h"
#include "UILabelComponent.h"
#include "WindowModule.h"

#include "glew.h"
#include "imgui.h"
#include <queue>

CanvasComponent::CanvasComponent(UID uid, GameObject* parent) : Component(uid, parent, "Canvas", COMPONENT_CANVAS)
{
    width  = (float)App->GetWindowModule()->GetWidth();
    height = (float)App->GetWindowModule()->GetHeight();
}

CanvasComponent::CanvasComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    width                = initialState["Width"].GetFloat();
    height               = initialState["Height"].GetFloat();
    isInWorldSpaceEditor = initialState["IsInWorldSpaceEditor"].GetBool();
    isInWorldSpaceGame   = initialState["IsInWorldSpaceGame"].GetBool();
}

CanvasComponent::~CanvasComponent()
{
    App->GetGameUIModule()->RemoveCanvas(this);
}

void CanvasComponent::Init()
{
    App->GetGameUIModule()->AddCanvas(this);

    localComponentAABB = AABB(
        float3(
            parent->GetGlobalTransform().TranslatePart().x - (width / 4.0f),
            parent->GetGlobalTransform().TranslatePart().y - (height / 4.0f), 0
        ),
        float3(
            parent->GetGlobalTransform().TranslatePart().x + (width / 4.0f),
            parent->GetGlobalTransform().TranslatePart().y + (height / 4.0f), 0
        )
    );
}

void CanvasComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("Width", width, allocator);
    targetState.AddMember("Height", height, allocator);
    targetState.AddMember("IsInWorldSpaceEditor", isInWorldSpaceEditor, allocator);
    targetState.AddMember("IsInWorldSpaceGame", isInWorldSpaceGame, allocator);
}

void CanvasComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_CANVAS)
    {
        const CanvasComponent* otherCanvas = static_cast<const CanvasComponent*>(other);
        width                              = otherCanvas->width;
        height                             = otherCanvas->height;

        isInWorldSpaceEditor               = otherCanvas->isInWorldSpaceEditor;
        isInWorldSpaceGame                 = otherCanvas->isInWorldSpaceGame;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void CanvasComponent::Update(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;

    const auto& size = App->GetSceneModule()->GetScene()->GetWindowSize();
    if (width != std::get<0>(size) || height != std::get<1>(size))
    {
        OnWindowResize(std::get<0>(size), std::get<1>(size));
    }
}

void CanvasComponent::Render(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;

    App->GetDebugDrawModule()->DrawLine(
        float3(
            parent->GetGlobalTransform().TranslatePart().x - width / 2,
            parent->GetGlobalTransform().TranslatePart().y + height / 2, 0
        ),
        float3::unitX, width, float3(1, 1, 1)
    );
    App->GetDebugDrawModule()->DrawLine(
        float3(
            parent->GetGlobalTransform().TranslatePart().x - width / 2,
            parent->GetGlobalTransform().TranslatePart().y - height / 2, 0
        ),
        float3::unitX, width, float3(1, 1, 1)
    );

    App->GetDebugDrawModule()->DrawLine(
        float3(
            parent->GetGlobalTransform().TranslatePart().x - width / 2,
            parent->GetGlobalTransform().TranslatePart().y + height / 2, 0
        ),
        -float3::unitY, height, float3(1, 1, 1)
    );

    App->GetDebugDrawModule()->DrawLine(
        float3(
            parent->GetGlobalTransform().TranslatePart().x + width / 2,
            parent->GetGlobalTransform().TranslatePart().y + height / 2, 0
        ),
        -float3::unitY, height, float3(1, 1, 1)
    );
}

void CanvasComponent::RenderUI()
{
    if (!parent->IsGloballyEnabled()) return;

    const int uiProgram = App->GetShaderModule()->GetUIWidgetProgram();
    if (uiProgram == -1)
    {
        GLOG("Error with UI Program");
        return;
    }
    glUseProgram(uiProgram);

    const float4x4& view = isInWorldSpaceEditor ? App->GetCameraModule()->GetViewMatrix() : float4x4::identity;
    const float4x4& proj = isInWorldSpaceEditor ? App->GetCameraModule()->GetProjectionMatrix()
                                                : float4x4::D3DOrthoProjLH(-1, 1, width, height);

    for (const GameObject* child : sortedChildren)
    {
        // Only render UI components
        Component* uiWidget = child->GetComponentByType(COMPONENT_TRANSFORM_2D);
        if (uiWidget) static_cast<const Transform2DComponent*>(uiWidget)->RenderWidgets();

        uiWidget = child->GetComponentByType(COMPONENT_LABEL);
        if (uiWidget) static_cast<const UILabelComponent*>(uiWidget)->RenderUI(view, proj);

        uiWidget = child->GetComponentByType(COMPONENT_IMAGE);
        if (uiWidget) static_cast<const ImageComponent*>(uiWidget)->RenderUI(view, proj);
    }
}

void CanvasComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::Text("Canvas");

        ImGui::Checkbox("Show in world space", &isInWorldSpaceEditor);
    }
}

void CanvasComponent::OnWindowResize(const float width, const float height)
{
    this->width        = width;
    this->height       = height;

    localComponentAABB = AABB(
        float3(
            parent->GetGlobalTransform().TranslatePart().x - (width / 4.0f),
            parent->GetGlobalTransform().TranslatePart().y - (height / 4.0f), 0
        ),
        float3(
            parent->GetGlobalTransform().TranslatePart().x + (width / 4.0f),
            parent->GetGlobalTransform().TranslatePart().y + (height / 4.0f), 0
        )
    );

    // parent->UpdateTransformForGOBranch();

    for (const GameObject* child : sortedChildren)
    {
        // Only render UI components
        Component* transform2D = child->GetComponentByType(COMPONENT_TRANSFORM_2D);
        if (transform2D) static_cast<Transform2DComponent*>(transform2D)->AdaptToParentChanges();
    }
}

void CanvasComponent::UpdateChildren()
{
    // TODO: Right now this updates the children list every frame in case they are reordered in hierarchy.
    // To be more optimal, this could be called only when a gameObject is dragged around the hierarchy
    sortedChildren.clear();

    std::queue<UID> children;

    for (const UID child : parent->GetChildren())
    {
        children.push(child);
    }

    while (!children.empty())
    {
        const GameObject* currentObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(children.front());
        sortedChildren.push_back(currentObject);
        children.pop();

        for (const UID child : currentObject->GetChildren())
        {
            children.push(child);
        }
    }
}

void CanvasComponent::UpdateMousePosition(const float2& mousePos)
{
    // Only interact with elements if canvas is in screen mode
    if (isInWorldSpaceEditor) return;

    hoveredButton    = nullptr;
    bool buttonFound = false;

    for (int i = (int)sortedChildren.size() - 1; i >= 0; --i)
    {
        // Update all buttons
        Component* button = sortedChildren[i]->GetComponentByType(COMPONENT_BUTTON);
        if (button)
        {
            ButtonComponent* currentButton = static_cast<ButtonComponent*>(button);
            if (currentButton->UpdateMousePosition(mousePos, buttonFound))
            {
                hoveredButton = currentButton;
                buttonFound   = true;
            }
        }
    }
}

void CanvasComponent::OnMouseButtonPressed() const
{
    if (hoveredButton) hoveredButton->OnClick();
}

void CanvasComponent::OnMouseButtonReleased() const
{
    if (hoveredButton) hoveredButton->OnRelease();
}
