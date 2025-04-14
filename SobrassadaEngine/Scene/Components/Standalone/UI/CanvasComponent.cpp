#include "CanvasComponent.h"

#include "Application.h"
#include "CameraModule.h"
#include "DebugDrawModule.h"
#include "GameObject.h"
#include "GameUIModule.h"
#include "SceneModule.h"
#include "Transform2DComponent.h"
#include "WindowModule.h"

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
    // If the canvas' GameObject is not globally enabled (it or any of its parents are disabled), skip rendering
    if (!parent->IsGloballyEnabled()) return;

    // Draw the canvas debug lines (borders)
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

    // Render all UI widgets in the hierarchy using a queue
    std::queue<UID> children;

    for (const UID child : parent->GetChildren())
    {
        children.push(child);
    }

    while (!children.empty())
    {
        const GameObject* currentObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(children.front());

        // Only render 2D UI elements that are globally enabled and have a Transform2D component.
        // This ensures that deactivated UI elements (or their parents) are not rendered.
        if (currentObject->IsGloballyEnabled() && currentObject->GetComponentByType(COMPONENT_TRANSFORM_2D) != nullptr)
        {
            currentObject->Render(deltaTime);
        }

        children.pop();

        for (const UID child : currentObject->GetChildren())
        {
            children.push(child);
        }
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