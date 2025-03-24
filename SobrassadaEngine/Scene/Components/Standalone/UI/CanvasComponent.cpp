#include "CanvasComponent.h"

#include "Application.h"
#include "CameraModule.h"
#include "DebugDrawModule.h"
#include "GameUIModule.h"
#include "SceneModule.h"
#include "Transform2DComponent.h"
#include "WindowModule.h"

#include "imgui.h"
#include <queue>

CanvasComponent::CanvasComponent(UID uid, GameObject* parent) : Component(uid, parent, "Canvas", COMPONENT_CANVAS)
{
    if (parent->GetComponentByType(COMPONENT_TRANSFORM_2D) == nullptr)
    {
        parent->CreateComponent(COMPONENT_TRANSFORM_2D);
        transform2D = static_cast<Transform2DComponent*>(parent->GetComponentByType(COMPONENT_TRANSFORM_2D));
        transform2D->SetSize(App->GetWindowModule()->GetWidth(), App->GetWindowModule()->GetHeight());
    }

    App->GetGameUIModule()->AddCanvas(this);
}

CanvasComponent::~CanvasComponent()
{
    App->GetGameUIModule()->ResetCanvas();
}

void CanvasComponent::Clone(const Component* otherComponent)
{
}

void CanvasComponent::Update(float deltaTime)
{
}

void CanvasComponent::Render(float deltaTime)
{
    App->GetDebugDrawModule()->DrawLine(
        float3(
            transform2D->GetPosition().x - transform2D->GetSize().x / 2,
            transform2D->GetPosition().y + transform2D->GetSize().y / 2, 0
        ),
        float3::unitX, transform2D->GetSize().x, float3(1, 1, 1)
    );
    App->GetDebugDrawModule()->DrawLine(
        float3(
            transform2D->GetPosition().x - transform2D->GetSize().x / 2,
            transform2D->GetPosition().y - transform2D->GetSize().y / 2, 0
        ),
        float3::unitX, transform2D->GetSize().x, float3(1, 1, 1)
    );

    App->GetDebugDrawModule()->DrawLine(
        float3(
            transform2D->GetPosition().x - transform2D->GetSize().x / 2,
            transform2D->GetPosition().y + transform2D->GetSize().y / 2, 0
        ),
        -float3::unitY, transform2D->GetSize().y, float3(1, 1, 1)
    );

    App->GetDebugDrawModule()->DrawLine(
        float3(
            transform2D->GetPosition().x + transform2D->GetSize().x / 2,
            transform2D->GetPosition().y + transform2D->GetSize().y / 2, 0
        ),
        -float3::unitY, transform2D->GetSize().y, float3(1, 1, 1)
    );

    // Render all ui widgets

    // Iteratively render all UI children
    // TODO: Probably better to store them in an array of Transform2D when created and render those directly. Nope, becuase if changing hierarchy won't get updated. This way it gets updated always
    // std::queue<UID> children;
    //
    // while (!children.empty())
    //{
    //    App->GetSceneModule()->GetScene()->GetGameObjectByUID(children.front())->Render(deltaTime);
    //    children.pop();
    //
    //    for (const UID child : parent->GetChildren())
    //    {
    //        children.push(child);
    //    }
    //}

    for (const UID child : parent->GetChildren())
    {
        App->GetSceneModule()->GetScene()->GetGameObjectByUID(child)->Render(deltaTime);
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