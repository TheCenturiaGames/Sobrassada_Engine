#include "CanvasComponent.h"

#include "Application.h"
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
}

void CanvasComponent::Render(float deltaTime)
{
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
    const int uiProgram = App->GetShaderModule()->GetUIWidgetProgram();
    if (uiProgram == -1)
    {
        GLOG("Error with UI Program");
        return;
    }
    glUseProgram(uiProgram);

    // Get the view and projection matrix (world space or screen space)
    const float4x4& view = isInWorldSpaceEditor ? App->GetCameraModule()->GetViewMatrix() : float4x4::identity;
    const float4x4& proj = isInWorldSpaceEditor ? App->GetCameraModule()->GetProjectionMatrix()
                                                : float4x4::D3DOrthoProjLH(
                                                      -1, 1, (float)App->GetWindowModule()->GetWidth(),
                                                      (float)App->GetWindowModule()->GetHeight()
                                                  );

    for (UID child : sortedChildren)
    {
        const GameObject* currentObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(child);

        // Only render UI components
        Component* uiWidget             = currentObject->GetComponentByType(COMPONENT_TRANSFORM_2D);
        if (uiWidget) static_cast<const Transform2DComponent*>(uiWidget)->RenderWidgets();

        uiWidget = currentObject->GetComponentByType(COMPONENT_LABEL);
        if (uiWidget) static_cast<const UILabelComponent*>(uiWidget)->RenderUI(view, proj);

        uiWidget = currentObject->GetComponentByType(COMPONENT_IMAGE);
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

void CanvasComponent::OnWindowResize(const unsigned int width, const unsigned int height)
{
    this->width        = (float)width;
    this->height       = (float)height;

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

    parent->UpdateTransformForGOBranch();
}

void CanvasComponent::UpdateChildren()
{
    // Update the children list every frame in case they are reorderen in hierarchy. In the future to be more optimal 
    // maybe this can be called only when a gameObject is dragged around the hierarchy
    sortedChildren.clear();

    std::queue<UID> children;

    for (const UID child : parent->GetChildren())
    {
        children.push(child);
    }

    while (!children.empty())
    {
        const GameObject* currentObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(children.front());

        sortedChildren.push_back(currentObject->GetUID());
        children.pop();

        for (const UID child : currentObject->GetChildren())
        {
            children.push(child);
        }
    }
}

void CanvasComponent::UpdateMousePosition(const float2& mousePos)
{

}

void CanvasComponent::OnMouseButtonPressed()
{

}