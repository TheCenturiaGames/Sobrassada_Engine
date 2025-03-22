#include "CanvasComponent.h"
#include "Application.h"
#include "DebugDrawModule.h"
#include "Transform2DComponent.h"
#include "WindowModule.h"
#include "GameUIModule.h"

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
}

void CanvasComponent::Clone(const Component* otherComponent)
{
}